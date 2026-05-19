import { mock, describe, it, expect, beforeEach, afterEach } from "bun:test";
import { EventEmitter } from "node:events";

let dbInserts = [];
let dbUpdates = [];

const mockDb = {
    insert: (table) => ({
        values: (data) => {
            dbInserts.push({ table, data });
            const p = Promise.resolve(undefined);
            p.onConflictDoNothing = () => Promise.resolve();
            p.returning = () => Promise.resolve([{ id: 42 }]);
            return p;
        },
    }),
    select: () => ({
        from: () => ({
            where: () => Promise.resolve([{ id: 1 }]),
        }),
    }),
    update: (table) => ({
        set: (data) => {
            dbUpdates.push({ table, data });
            return { where: () => Promise.resolve() };
        },
    }),
};

mock.module("../db/db.js", () => ({ db: mockDb }));
mock.module("../db/schema.js", () => ({
    players: "PLAYERS",
    games: "GAMES",
    gameMoves: "GAME_MOVES",
}));
mock.module("drizzle-orm", () => ({ eq: () => ({}) }));

const { LichessBot } = await import("../src/lichessBot.js");

class MockEngine extends EventEmitter {
    constructor() {
        super();
        this.start = mock(async () => {});
        this.uciNewGame = mock(async () => {});
        this.position = mock(async () => {});
        this.go = mock(async () => "e2e4");
        this.stop = mock(async () => {});
    }
}

function createMockStream(items) {
    const encoder = new TextEncoder();
    return new ReadableStream({
        start(controller) {
            for (const item of items) {
                controller.enqueue(encoder.encode(JSON.stringify(item) + "\n"));
            }
            controller.close();
        },
    });
}

async function waitFor(conditionFn, timeout = 1500) {
    const deadline = Date.now() + timeout;
    while (Date.now() < deadline) {
        try {
            if (conditionFn()) return;
        } catch (_) {}
        await new Promise(r => setTimeout(r, 10));
    }
    conditionFn();
}

function makeGameFull(gameId, botId, opts = {}) {
    const clock = opts.clock === null
        ? null
        : { initial: opts.clock?.initial ?? 180000, increment: opts.clock?.increment ?? 2000 };

    return {
        type: "gameFull",
        initialFen: opts.fen ?? "startpos",
        white: { id: opts.white ?? botId, rating: 1500 },
        black: { id: opts.black ?? "opponent", rating: 1400 },
        variant: { key: "standard" },
        rated: false,
        clock,
        state: {
            moves: opts.moves ?? "",
            wtime: opts.wtime ?? 60000,
            btime: opts.btime ?? 60000,
            winc: opts.winc ?? 1000,
            binc: opts.binc ?? 1000,
            status: opts.status ?? "started",
            winner: opts.winner ?? undefined,
        },
    };
}

let bot;
let engine;
let originalFetch;

beforeEach(() => {
    engine = new MockEngine();
    bot = new LichessBot("fake_token", engine);
    originalFetch = global.fetch;
    dbInserts = [];
    dbUpdates = [];
});

afterEach(() => {
    global.fetch = originalFetch;
    bot.stop();
});

describe("Promotion normalization", () => {
    it("sends a lowercase promotion piece to Lichess when engine returns uppercase", async () => {
        const gameId = "promo_test";
        engine.go = mock(async () => "e7e8Q");

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([makeGameFull(gameId, "bot")]) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining(`/move/e7e8q`),
                expect.any(Object)
            );
        });
    });

    it("does not modify normal 4-character moves", async () => {
        const gameId = "normal_move";
        engine.go = mock(async () => "d2d4");

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([makeGameFull(gameId, "bot")]) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining(`/move/d2d4`),
                expect.any(Object)
            );
        });
    });
});

describe("Time management — adaptive across time controls", () => {
    async function captureMoveTime(gameId, { clockInitial, clockIncrement, wtime, winc }) {
        engine.go = mock(async () => "e2e4");

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return {
                    ok: true,
                    body: createMockStream([makeGameFull(gameId, "bot", {
                        clock: clockInitial === null ? null : { initial: clockInitial, increment: clockIncrement },
                        wtime, winc,
                    })]),
                };
            }
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();
        await waitFor(() => expect(engine.go).toHaveBeenCalled());
        return engine.go.mock.calls[0][0].moveTime;
    }

    it("BULLET (60s+0): caps at 2s per move", async () => {
        const t = await captureMoveTime("bullet_test", { clockInitial: 60_000, clockIncrement: 0, wtime: 60_000, winc: 0 });
        expect(t).toBeLessThanOrEqual(2000);
        expect(t).toBeGreaterThanOrEqual(200);
    });

    it("BLITZ (3min+2s): allocates ~2.8s on the opening move", async () => {
        const t = await captureMoveTime("blitz_test", { clockInitial: 180_000, clockIncrement: 2000, wtime: 60_000, winc: 1000 });
        expect(t).toBe(2850);
    });

    it("BLITZ (3min+2s): caps at 5s even with huge remaining time", async () => {
        const t = await captureMoveTime("blitz_cap", { clockInitial: 180_000, clockIncrement: 2000, wtime: 300_000, winc: 2000 });
        expect(t).toBeLessThanOrEqual(5000);
    });

    it("RAPID (10min+0): caps at 15s per move", async () => {
        const t = await captureMoveTime("rapid_test", { clockInitial: 600_000, clockIncrement: 0, wtime: 600_000, winc: 0 });
        expect(t).toBeLessThanOrEqual(15000);
        expect(t).toBeGreaterThan(5000);
    });

    it("CLASSICAL (30min+15s): allows up to 60s per move", async () => {
        const t = await captureMoveTime("classical_test", { clockInitial: 1_800_000, clockIncrement: 15000, wtime: 1_800_000, winc: 15000 });
        expect(t).toBeLessThanOrEqual(60000);
        expect(t).toBeGreaterThan(15000);
    });

    it("CLASSICAL (60min+0): hits the absolute 60s ceiling", async () => {
        const t = await captureMoveTime("classical_big", { clockInitial: 3_600_000, clockIncrement: 0, wtime: 3_600_000, winc: 0 });
        expect(t).toBe(60000);
    });

    it("CORRESPONDENCE (no clock): uses a steady 5s budget", async () => {
        engine.go = mock(async () => "e2e4");

        const gf = {
            ...makeGameFull("corr_test", "bot", { clock: null }),
        };
        delete gf.state.wtime;
        delete gf.state.btime;
        delete gf.state.winc;
        delete gf.state.binc;

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: "corr_test" } }]) };
            if (url.includes("/bot/game/stream/corr_test")) return { ok: true, body: createMockStream([gf]) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();
        await waitFor(() => expect(engine.go).toHaveBeenCalled());

        expect(engine.go.mock.calls[0][0].moveTime).toBe(5000);
    });

    it("enforces 200ms minimum even when remaining clock is near zero", async () => {
        const t = await captureMoveTime("low_clock", { clockInitial: 180_000, clockIncrement: 0, wtime: 1, winc: 0 });
        expect(t).toBeGreaterThanOrEqual(200);
    });

    it("never spends more than 80% of remaining clock on one move (safety)", async () => {
        const t = await captureMoveTime("safety_test", { clockInitial: 180_000, clockIncrement: 0, wtime: 600, winc: 0 });
        expect(t).toBeLessThanOrEqual(Math.floor(600 * 0.8));
    });
});

describe("isMyTurn logic", () => {
    async function setupGame(gameId, gameFull) {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([gameFull]) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });
        await bot.start();
    }

    it("makes a move when bot is white and no moves have been played", async () => {
        await setupGame("t1", makeGameFull("t1", "bot", { white: "bot", black: "opp", moves: "" }));
        await waitFor(() => expect(engine.go).toHaveBeenCalled());
    });

    it("does NOT make a move when bot is black and no moves have been played", async () => {
        await setupGame("t2", makeGameFull("t2", "bot", { white: "opp", black: "bot", moves: "" }));
        await new Promise(r => setTimeout(r, 150));
        expect(engine.go).not.toHaveBeenCalled();
    });

    it("makes a move when bot is black after white's first move", async () => {
        await setupGame("t3", makeGameFull("t3", "bot", { white: "opp", black: "bot", moves: "e2e4" }));
        await waitFor(() => expect(engine.go).toHaveBeenCalled());
    });

    it("does NOT make a move when bot is white after white's first move", async () => {
        await setupGame("t4", makeGameFull("t4", "bot", { white: "bot", black: "opp", moves: "e2e4" }));
        await new Promise(r => setTimeout(r, 150));
        expect(engine.go).not.toHaveBeenCalled();
    });

    it("makes a move when bot is white after both sides have moved once", async () => {
        await setupGame("t5", makeGameFull("t5", "bot", { white: "bot", black: "opp", moves: "e2e4 e7e5" }));
        await waitFor(() => expect(engine.go).toHaveBeenCalled());
    });
});

describe("start()", () => {
    it("fetches bot profile and sets botProfile", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "my-engine-bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([]) };
            return { ok: false };
        });

        await bot.start();

        expect(bot.botProfile).toBe("my-engine-bot");
    });

    it("calls engine.start()", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([]) };
            return { ok: false };
        });

        await bot.start();

        expect(engine.start).toHaveBeenCalledTimes(1);
    });

    it("throws if /api/account returns a non-OK response", async () => {
        global.fetch = mock(async () => ({ ok: false, statusText: "Unauthorized" }));

        await expect(bot.start()).rejects.toThrow("Failed to fetch bot profile");
    });
});

describe("stop()", () => {
    it("sets eventController to null and clears activeGames", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) {
                return { ok: true, body: new ReadableStream({ start() {} }) };
            }
            return { ok: false };
        });

        await bot.start();

        expect(bot.eventController).not.toBeNull();

        bot.stop();

        expect(bot.eventController).toBeNull();
        expect(bot.activeGames.size).toBe(0);
    });

    it("aborts all active game streams", async () => {
        bot.activeGames.add("game_abc");
        const gameController = new AbortController();
        bot.gameControllers.set("game_abc", gameController);

        bot.stop();

        expect(gameController.signal.aborted).toBe(true);
        expect(bot.gameControllers.size).toBe(0);
    });

    it("is safe to call when bot was never started", () => {
        expect(() => bot.stop()).not.toThrow();
    });
});

describe("handleChallenge()", () => {
    it("accepts standard chess challenges", async () => {
        const event = {
            type: "challenge",
            challenge: { id: "ch_std", variant: { key: "standard" } },
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([event]) };
            if (url.includes("/challenge/ch_std/accept")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/challenge/ch_std/accept"),
                expect.objectContaining({ method: "POST" })
            );
        });
    });

    it("declines challenges with a non-standard variant", async () => {
        const event = {
            type: "challenge",
            challenge: { id: "ch_960", variant: { key: "chess960" } },
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([event]) };
            if (url.includes("/challenge/ch_960/decline")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/challenge/ch_960/decline"),
                expect.any(Object)
            );
        });
    });

    it("declines a standard challenge when already in a game", async () => {
        const event = {
            type: "challenge",
            challenge: { id: "ch_busy", variant: { key: "standard" } },
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([event]) };
            if (url.includes("/challenge/ch_busy/decline")) return { ok: true };
            return { ok: false };
        });

        bot.activeGames.add("ongoing_game");
        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/challenge/ch_busy/decline"),
                expect.any(Object)
            );
        });
    });

    it("does NOT accept the challenge when busy — no accept call made", async () => {
        const event = {
            type: "challenge",
            challenge: { id: "ch_no_accept", variant: { key: "standard" } },
        };

        let acceptCalled = false;
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([event]) };
            if (url.includes("/challenge/ch_no_accept/accept")) { acceptCalled = true; return { ok: true }; }
            if (url.includes("/challenge/ch_no_accept/decline")) return { ok: true };
            return { ok: false };
        });

        bot.activeGames.add("ongoing_game");
        await bot.start();

        await new Promise(r => setTimeout(r, 200));
        expect(acceptCalled).toBe(false);
    });
});

describe("Time-control flexibility", () => {
    const timeControls = [
        { name: "bullet (60+0)",       limit: 60,    increment: 0  },
        { name: "blitz (3+2)",         limit: 180,   increment: 2  },
        { name: "rapid (10+0)",        limit: 600,   increment: 0  },
        { name: "classical (30+15)",   limit: 1800,  increment: 15 },
        { name: "custom (7+3)",        limit: 420,   increment: 3  },
    ];

    for (const tc of timeControls) {
        it(`accepts a standard challenge with ${tc.name}`, async () => {
            const event = {
                type: "challenge",
                challenge: {
                    id: `ch_${tc.limit}_${tc.increment}`,
                    variant: { key: "standard" },
                    timeControl: { limit: tc.limit, increment: tc.increment },
                },
            };

            global.fetch = mock(async (url) => {
                if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
                if (url.includes("/stream/event")) return { ok: true, body: createMockStream([event]) };
                if (url.includes(`/challenge/${event.challenge.id}/accept`)) return { ok: true };
                return { ok: false };
            });

            await bot.start();

            await waitFor(() => {
                expect(global.fetch).toHaveBeenCalledWith(
                    expect.stringContaining(`/challenge/${event.challenge.id}/accept`),
                    expect.objectContaining({ method: "POST" })
                );
            });
        });
    }
});

describe("Challenge creation", () => {
    function captureChallengeBody() {
        let captured = null;
        global.fetch = mock(async (url, opts) => {
            if (url.includes("/api/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/api/challenge/")) {
                captured = opts?.body?.toString() ?? "";
                return { ok: true, json: async () => ({ id: "fake_challenge" }) };
            }
            return { ok: false };
        });
        return () => captured;
    }

    it("createOpenChallenge defaults to rated=true with the requested time control", async () => {
        const getBody = captureChallengeBody();
        bot.botProfile = "bot";

        await bot.createOpenChallenge(600, 5);

        const body = getBody();
        expect(body).toContain("clock.limit=600");
        expect(body).toContain("clock.increment=5");
        expect(body).toContain("rated=true");
    });

    it("createOpenChallenge supports rated=false for casual games", async () => {
        const getBody = captureChallengeBody();
        bot.botProfile = "bot";

        await bot.createOpenChallenge(1800, 15, false);

        const body = getBody();
        expect(body).toContain("clock.limit=1800");
        expect(body).toContain("clock.increment=15");
        expect(body).toContain("rated=false");
    });

    it("createChallenge supports arbitrary time controls", async () => {
        const getBody = captureChallengeBody();
        bot.botProfile = "bot";

        await bot.createChallenge("opponent", 420, 3);

        const body = getBody();
        expect(body).toContain("clock.limit=420");
        expect(body).toContain("clock.increment=3");
    });

    it("createAiChallenge passes time control through to Lichess", async () => {
        const getBody = captureChallengeBody();
        bot.botProfile = "bot";

        await bot.createAiChallenge(3, 600, 0);

        const body = getBody();
        expect(body).toContain("level=3");
        expect(body).toContain("clock.limit=600");
        expect(body).toContain("clock.increment=0");
    });
});

describe("streamEvents() routing", () => {
    it("opens a game stream when a gameStart event is received", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: "g1" } }]) };
            if (url.includes("/bot/game/stream/g1")) return { ok: true, body: createMockStream([]) };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/bot/game/stream/g1"),
                expect.any(Object)
            );
        });
    });

    it("does not open a game stream for unrecognized event types", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "unknown" }]) };
            return { ok: false };
        });

        await bot.start();
        await new Promise(r => setTimeout(r, 100));

        const gameStreamCalls = global.fetch.mock.calls.filter(([url]) =>
            url.includes("/bot/game/stream/")
        );
        expect(gameStreamCalls).toHaveLength(0);
    });
});

describe("playGame() — gameFull event", () => {
    it("adds the gameId to activeGames while the stream is open", async () => {
        const gameId = "active_test";
        let streamCtrl;

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return {
                    ok: true,
                    body: new ReadableStream({ start(c) { streamCtrl = c; } }),
                };
            }
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();
        await new Promise(r => setTimeout(r, 100));

        expect(bot.activeGames.has(gameId)).toBe(true);

        streamCtrl?.close();
        await new Promise(r => setTimeout(r, 50));
        expect(bot.activeGames.has(gameId)).toBe(false);
    });

    it("does not enter the same game twice", async () => {
        const gameId = "dedup";
        let streamCtrl;

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return { ok: true, body: new ReadableStream({ start(c) { streamCtrl = c; } }) };
            }
            return { ok: false };
        });

        await bot.start();
        await new Promise(r => setTimeout(r, 50));

        await bot.playGame(gameId);

        const openStreams = global.fetch.mock.calls.filter(([url]) =>
            url.includes(`/bot/game/stream/${gameId}`)
        );
        expect(openStreams).toHaveLength(1);

        streamCtrl?.close();
    });

    it("calls uciNewGame before sending the first position", async () => {
        const gameId = "uci_order";

        const callOrder = [];
        engine.uciNewGame = mock(async () => { callOrder.push("uciNewGame"); });
        engine.position = mock(async () => { callOrder.push("position"); });
        engine.go = mock(async () => "e2e4");

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([makeGameFull(gameId, "bot")]) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => expect(callOrder).toContain("position"));

        expect(callOrder.indexOf("uciNewGame")).toBeLessThan(callOrder.indexOf("position"));
    });

    it("sends the correct position and makes a move on bot's turn", async () => {
        const gameId = "move_test";

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return { ok: true, body: createMockStream([makeGameFull(gameId, "bot", { moves: "e2e4 e7e5" })]) };
            }
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => expect(engine.position).toHaveBeenCalled());

        expect(engine.position).toHaveBeenCalledWith("startpos", ["e2e4", "e7e5"]);
        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining(`/bot/game/${gameId}/move/e2e4`),
                expect.any(Object)
            );
        });
    });
});

describe("playGame() — gameState event", () => {
    it("makes a move when a gameState event signals it's the bot's turn", async () => {
        const gameId = "state_test";
        engine.go = mock(async () => "g1f3");

        const gameState = {
            type: "gameState",
            moves: "e2e4 e7e5 g1f3",
            wtime: 58000, btime: 59000, winc: 1000, binc: 1000,
            status: "started",
        };

        const gameFull2 = makeGameFull(gameId, "bot", {
            white: "opp", black: "bot", moves: "e2e4 e7e5",
        });

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return { ok: true, body: createMockStream([gameFull2, gameState]) };
            }
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(engine.go.mock.calls.length).toBeGreaterThanOrEqual(1);
        });
    });
});

describe("Game over", () => {
    async function playUntilOver(gameId, events) {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream(events) };
            if (url.includes("/move/")) return { ok: true };
            return { ok: false };
        });
        await bot.start();
    }

    it("removes the game from activeGames on game over", async () => {
        const gameId = "over_test";
        const endState = {
            type: "gameState",
            moves: "e2e4 e7e5",
            wtime: 60000, btime: 60000, winc: 0, binc: 0,
            status: "mate",
            winner: "white",
        };

        await playUntilOver(gameId, [
            makeGameFull(gameId, "bot", { white: "opp", black: "bot" }),
            endState,
        ]);

        await waitFor(() => expect(bot.activeGames.has(gameId)).toBe(false));
    });

    it("does not call engine.go after a game-over status", async () => {
        const gameId = "no_move_after_end";
        const gameFull = makeGameFull(gameId, "bot", {
            white: "bot", black: "opp",
            status: "mate",
            winner: "black",
        });
        gameFull.state.status = "mate";
        gameFull.state.winner = "black";

        await playUntilOver(gameId, [gameFull]);

        await new Promise(r => setTimeout(r, 150));
        expect(engine.go).not.toHaveBeenCalled();
    });
});

describe("Resignation", () => {
    async function expectResign(gameId, engineMove) {
        engine.go = mock(async () => engineMove);

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([makeGameFull(gameId, "bot")]) };
            return { ok: true };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining(`/bot/game/${gameId}/resign`),
                expect.objectContaining({ method: "POST" })
            );
        });
    }

    it("resigns when engine returns (none)", () => expectResign("res1", "(none)"));
    it("resigns when engine returns 0000",  () => expectResign("res2", "0000"));
    it("resigns when engine returns null",  () => expectResign("res3", null));
});

describe("DB integration", () => {
    async function runGame(gameId, events) {
        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "bot" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream([{ type: "gameStart", game: { id: gameId } }]) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream(events) };
            if (url.includes("/move/")) return { ok: true };
            if (url.includes("/resign")) return { ok: true };
            return { ok: false };
        });
        await bot.start();
    }

    it("upserts both players when a game starts", async () => {
        const gf = makeGameFull("db1", "bot", { white: "bot", black: "opponent" });
        await runGame("db1", [gf]);

        await waitFor(() => {
            const playerInserts = dbInserts.filter(i => i.table === "PLAYERS");
            expect(playerInserts.length).toBeGreaterThanOrEqual(2);
            const names = playerInserts.map(i => i.data.name);
            expect(names).toContain("bot");
            expect(names).toContain("opponent");
        });
    });

    it("creates a game record with Lichess metadata", async () => {
        const gf = makeGameFull("db2", "bot");
        await runGame("db2", [gf]);

        await waitFor(() => {
            const gameInsert = dbInserts.find(i => i.table === "GAMES");
            expect(gameInsert).toBeDefined();
            expect(gameInsert.data).toMatchObject({
                source: "lichess",
                lichessGameId: "db2",
                variant: "standard",
                rated: 0,
            });
        });
    });

    it("saves moves present in the initial gameFull state", async () => {
        const gf = makeGameFull("db3", "bot", { white: "opp", black: "bot", moves: "e2e4 e7e5" });
        await runGame("db3", [gf]);

        await waitFor(() => {
            const moveInserts = dbInserts.filter(i => i.table === "GAME_MOVES");
            expect(moveInserts.length).toBeGreaterThanOrEqual(1);
            const flatMoves = moveInserts.flatMap(i =>
                Array.isArray(i.data) ? i.data : [i.data]
            );
            expect(flatMoves).toContainEqual(expect.objectContaining({ ply: 1, uci: "e2e4" }));
            expect(flatMoves).toContainEqual(expect.objectContaining({ ply: 2, uci: "e7e5" }));
        });
    });

    it("saves only NEW moves from a gameState event (no duplicates)", async () => {
        const gameId = "db4";
        const gf = makeGameFull(gameId, "bot", { white: "opp", black: "bot", moves: "e2e4" });
        const gs = {
            type: "gameState",
            moves: "e2e4 e7e5",
            wtime: 59000, btime: 60000, winc: 1000, binc: 1000,
            status: "started",
        };

        await runGame(gameId, [gf, gs]);

        await waitFor(() => {
            const flatMoves = dbInserts
                .filter(i => i.table === "GAME_MOVES")
                .flatMap(i => Array.isArray(i.data) ? i.data : [i.data]);

            const ply1s = flatMoves.filter(m => m.ply === 1 && m.uci === "e2e4");
            expect(ply1s).toHaveLength(1);

            const ply2s = flatMoves.filter(m => m.ply === 2 && m.uci === "e7e5");
            expect(ply2s).toHaveLength(1);
        });
    });

    it("writes result and termination on game over (white wins by mate)", async () => {
        const gameId = "db5";
        const gf = makeGameFull(gameId, "bot", { white: "opp", black: "bot" });
        const gs = {
            type: "gameState",
            moves: "e2e4",
            wtime: 59000, btime: 60000, winc: 0, binc: 0,
            status: "mate",
            winner: "white",
        };

        await runGame(gameId, [gf, gs]);

        await waitFor(() => {
            const update = dbUpdates.find(u => u.data?.result === "1-0");
            expect(update).toBeDefined();
            expect(update.data).toMatchObject({ result: "1-0", termination: "mate" });
            expect(update.data.finishedAt).toBeTruthy();
        });
    });

    it("writes 0-1 result when black wins", async () => {
        const gameId = "db6";
        const gf = makeGameFull(gameId, "bot", { white: "opp", black: "bot" });
        const gs = {
            type: "gameState",
            moves: "e2e4",
            wtime: 59000, btime: 60000, winc: 0, binc: 0,
            status: "resign",
            winner: "black",
        };

        await runGame(gameId, [gf, gs]);

        await waitFor(() => {
            expect(dbUpdates.some(u => u.data?.result === "0-1")).toBe(true);
        });
    });

    it("writes 1/2-1/2 for drawn games", async () => {
        const gameId = "db7";
        const gf = makeGameFull(gameId, "bot", { white: "opp", black: "bot" });
        const gs = {
            type: "gameState",
            moves: "",
            wtime: 0, btime: 0, winc: 0, binc: 0,
            status: "draw",
            winner: null,
        };

        await runGame(gameId, [gf, gs]);

        await waitFor(() => {
            expect(dbUpdates.some(u => u.data?.result === "1/2-1/2")).toBe(true);
        });
    });

    it("writes 1/2-1/2 for stalemate", async () => {
        const gameId = "db8";
        const gf = makeGameFull(gameId, "bot", { white: "opp", black: "bot" });
        const gs = {
            type: "gameState",
            moves: "",
            wtime: 0, btime: 0, winc: 0, binc: 0,
            status: "stalemate",
            winner: null,
        };

        await runGame(gameId, [gf, gs]);

        await waitFor(() => {
            expect(dbUpdates.some(u => u.data?.result === "1/2-1/2")).toBe(true);
        });
    });
});

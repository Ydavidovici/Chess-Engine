import { describe, it, expect, mock, beforeEach, afterEach, spyOn } from "bun:test";
import { EventEmitter } from "node:events";
import { LichessBot } from "../src/lichessBot.js";

/**
 * 1. Mock UCI Engine
 * We simulate the engine's behavior without spawning a real process.
 */
class MockEngine extends EventEmitter {
    constructor() {
        super();
        this.start = mock(async () => {});
        this.position = mock(async () => {});
        this.go = mock(async () => "e2e4");
        this.stop = mock(async () => {});
    }
}

/**
 * 2. Helper: Wait for a condition to be true (retries for ~1 second)
 * This fixes the race conditions where the test checks before the bot acts.
 */
async function waitFor(conditionFn, timeout = 1000) {
    const start = Date.now();
    while (Date.now() - start < timeout) {
        try {
            if (conditionFn()) return;
        } catch (e) {
        }
        await new Promise(resolve => setTimeout(resolve, 10)); // tiny sleep
    }
    conditionFn();
}

/**
 * 3. Helper to create NDJSON Streams
 */
function createMockStream(dataArray) {
    const encoder = new TextEncoder();
    return new ReadableStream({
        start(controller) {
            for (const item of dataArray) {
                controller.enqueue(encoder.encode(JSON.stringify(item) + "\n"));
            }
            controller.close();
        }
    });
}

/**
 * 4. The Test Suite
 */
describe("LichessBot Logic", () => {
    let bot;
    let engine;
    let originalFetch;

    beforeEach(() => {
        engine = new MockEngine();
        bot = new LichessBot("fake_token", engine);
        originalFetch = global.fetch;
    });

    afterEach(() => {
        global.fetch = originalFetch;
    });

    it("should fetch profile and start engine on startup", async () => {
        global.fetch = mock(async (url) => {
            if (url.includes("/api/account")) {
                return { ok: true, json: async () => ({ id: "my-bot-name" }) };
            }
            if (url.includes("/api/stream/event")) {
                return { ok: true, body: createMockStream([]) };
            }
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(bot.botProfile).toBe("my-bot-name");
            expect(engine.start).toHaveBeenCalled();
            expect(global.fetch).toHaveBeenCalledTimes(2);
        });
    });

    it("should accept challenges when received", async () => {
        const challengeEvent = {
            type: "challenge",
            challenge: { id: "challenge123" }
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "my-bot-name" }) };

            if (url.includes("/stream/event")) {
                return { ok: true, body: createMockStream([challengeEvent]) };
            }

            if (url.includes("/challenge/challenge123/accept")) {
                return { ok: true };
            }

            return { ok: false, statusText: "Not Found: " + url };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/challenge/challenge123/accept"),
                expect.objectContaining({ method: "POST" })
            );
        });
    });

    it("should join a game stream when a game starts", async () => {
        const gameStartEvent = {
            type: "gameStart",
            game: { id: "gameXYZ" }
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "my-bot-name" }) };

            if (url.includes("/stream/event")) {
                return { ok: true, body: createMockStream([gameStartEvent]) };
            }

            if (url.includes("/bot/game/stream/gameXYZ")) {
                return { ok: true, body: createMockStream([]) };
            }

            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining("/bot/game/stream/gameXYZ"),
                expect.any(Object)
            );
        });
    });

    it("should make a move when it is the bot's turn", async () => {
        const gameId = "game_turn_test";

        const eventStreamData = [{ type: "gameStart", game: { id: gameId } }];
        const gameFullEvent = {
            type: "gameFull",
            initialFen: "startpos",
            white: { id: "my-bot-name" },
            black: { id: "opponent" },
            state: {
                moves: "",
                wtime: 60000, btime: 60000, winc: 1000, binc: 1000,
                status: "started"
            }
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "my-bot-name" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream(eventStreamData) };

            if (url.includes(`/bot/game/stream/${gameId}`)) {
                return { ok: true, body: createMockStream([gameFullEvent]) };
            }

            if (url.includes(`/move/e2e4`)) {
                return { ok: true };
            }
            return { ok: false };
        });

        await bot.start();

        await waitFor(() => {
            expect(engine.position).toHaveBeenCalledWith("startpos", []);

            expect(engine.go).toHaveBeenCalled();

            expect(global.fetch).toHaveBeenCalledWith(
                expect.stringContaining(`/api/bot/game/${gameId}/move/e2e4`),
                expect.objectContaining({ method: "POST" })
            );
        });
    });

    it("should NOT make a move if it is NOT the bot's turn", async () => {
        const gameId = "game_opponent_turn";
        const eventStreamData = [{ type: "gameStart", game: { id: gameId } }];

        const gameFullEvent = {
            type: "gameFull",
            initialFen: "startpos",
            white: { id: "opponent" },
            black: { id: "my-bot-name" },
            state: {
                moves: "",
                wtime: 60000, btime: 60000, winc: 0, binc: 0,
                status: "started"
            }
        };

        global.fetch = mock(async (url) => {
            if (url.includes("/account")) return { ok: true, json: async () => ({ id: "my-bot-name" }) };
            if (url.includes("/stream/event")) return { ok: true, body: createMockStream(eventStreamData) };
            if (url.includes(`/bot/game/stream/${gameId}`)) return { ok: true, body: createMockStream([gameFullEvent]) };
            return { ok: false };
        });

        await bot.start();

        await new Promise(r => setTimeout(r, 100));

        expect(engine.go).not.toHaveBeenCalled();
    });
});
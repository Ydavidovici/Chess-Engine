import {eq} from "drizzle-orm";
import {db} from "../db/db.js";
import {players, games, gameMoves} from "../db/schema.js";
import {nullNotifier} from "./notifier.js";

export function normalizeMove(move) {
    if (move && move.length === 5) {
        return move.slice(0, 4) + move[4].toLowerCase();
    }
    return move;
}

export function computeMoveTime(remainingMs, incMs, totalTimeMs) {
    const inc = incMs ?? 0;

    if (remainingMs == null) return 5000;

    const total = totalTimeMs ?? remainingMs;

    let cap;
    if      (total >= 1_500_000) cap = 60_000;
    else if (total >=   480_000) cap = 15_000;
    else if (total >=   180_000) cap =  5_000;
    else if (total >=    60_000) cap =  2_000;
    else                         cap =  1_000;

    const estimate = Math.floor(remainingMs / 30 + inc * 0.85);
    const safety   = Math.floor(remainingMs * 0.8);

    return Math.max(200, Math.min(estimate, cap, safety));
}

export function mapResult(status, winner) {
    if (winner === "white") return "1-0";
    if (winner === "black") return "0-1";
    const draws = ["draw", "stalemate", "threefoldRepetition", "insufficient", "fiftyMoves", "outoftime", "timeout"];
    if (draws.includes(status)) return "1/2-1/2";
    return null;
}

export class LichessBot {
    constructor(token, engineFactory, options = {}) {
        this.token = token;
        this.engineFactory = engineFactory;
        this.maxConcurrentGames = options.maxConcurrentGames ?? 4;
        this.huntPollIntervalMs = options.huntPollIntervalMs ?? 1000;
        this.reconnectDelayMs = options.reconnectDelayMs ?? 5000;
        this.notifier = options.notifier ?? nullNotifier;

        this.authHeader = {Authorization: `Bearer ${this.token}`};
        this.formHeaders = {
            Authorization: `Bearer ${this.token}`,
            "Content-Type": "application/x-www-form-urlencoded",
        };

        this.activeGames = new Set();
        this.botProfile = null;

        this.eventController = null;
        this.gameControllers = new Map();
        this.gameEngines = new Map();

        this.dbGameIds = new Map();
        this.savedPlies = new Map();

        // Autoplay: when enabled, the bot fills free slots via huntWeakestBot.
        this.autoplay = null; // {limit, increment, rated, target, backoffMs, timer, huntInFlight}
    }

    startAutoplay({limit = 180, increment = 2, rated = true, target = 1, mode = "near", window = 200} = {}) {
        // target = how many active games we'd like to keep going at once.
        // Capped by maxConcurrentGames as a safety.
        const cappedTarget = Math.min(target, this.maxConcurrentGames);

        this.autoplay = {
            limit,
            increment,
            rated,
            target: cappedTarget,
            mode,       // "near" (default) or "weakest"
            window,     // only used when mode === "near"
            currentBackoffMs: 0,
            timer: null,
            huntInFlight: false,
        };

        this.notifier.info("Autoplay enabled", {limit, increment, rated, target: cappedTarget, mode, window});
        console.log(`[Autoplay] Enabled (${limit}+${increment} ${rated ? "rated" : "casual"}, target=${cappedTarget}, mode=${mode}${mode === "near" ? `, window=±${window}` : ""})`);
        this._tickAutoplay();
    }

    stopAutoplay() {
        if (!this.autoplay) return;
        if (this.autoplay.timer) clearTimeout(this.autoplay.timer);
        this.autoplay = null;
        this.notifier.info("Autoplay disabled");
        console.log("[Autoplay] Disabled");
    }

    autoplayStatus() {
        if (!this.autoplay) return {enabled: false};
        const {limit, increment, rated, target, mode, window, currentBackoffMs, huntInFlight} = this.autoplay;
        return {enabled: true, limit, increment, rated, target, mode, window, currentBackoffMs, huntInFlight, active: this.activeGames.size};
    }

    // Kick the autoplay loop. Idempotent. Called after every game ends and on a
    // slow safety-net timer so we recover even if nothing else triggers us.
    _tickAutoplay() {
        if (!this.autoplay) return;
        if (this.autoplay.timer) {
            clearTimeout(this.autoplay.timer);
            this.autoplay.timer = null;
        }

        // Already running enough games or already hunting? Just schedule a check.
        if (this.activeGames.size >= this.autoplay.target || this.autoplay.huntInFlight) {
            this.autoplay.timer = setTimeout(() => this._tickAutoplay(), 30_000);
            return;
        }

        this.autoplay.huntInFlight = true;
        const {limit, increment, rated, mode, window} = this.autoplay;

        const huntPromise = mode === "weakest"
            ? this.huntWeakestBot(limit, increment, rated)
            : this.huntNearRating(limit, increment, rated, {window});

        // Don't await — let it run in the background while we schedule the next tick.
        huntPromise
            .then(() => {
                if (!this.autoplay) return;
                this.autoplay.currentBackoffMs = 0;
            })
            .catch(err => {
                if (!this.autoplay) return;
                // Exponential backoff: 5s, 10s, 20s, ... capped at 5 min.
                const next = this.autoplay.currentBackoffMs === 0 ? 5_000 : Math.min(this.autoplay.currentBackoffMs * 2, 300_000);
                this.autoplay.currentBackoffMs = next;
                console.log(`[Autoplay] Hunt failed (${err.message}); retrying in ${next / 1000}s`);
            })
            .finally(() => {
                if (!this.autoplay) return;
                this.autoplay.huntInFlight = false;
                const wait = this.autoplay.currentBackoffMs || 1_000;
                this.autoplay.timer = setTimeout(() => this._tickAutoplay(), wait);
            });
    }

    async start() {
        console.log("[Bot] Starting...");

        const profileRes = await fetch("https://lichess.org/api/account", {
            headers: this.authHeader,
        });
        if (!profileRes.ok) throw new Error("Failed to fetch bot profile");
        const profile = await profileRes.json();
        this.botProfile = profile.id;
        console.log(`[Bot] Logged in as: ${this.botProfile} (max ${this.maxConcurrentGames} concurrent games)`);

        this.streamEvents();
    }

    stop() {
        console.log("[Bot] Stopping...");
        this.stopAutoplay();

        if (this.eventController) {
            this.eventController.abort();
            this.eventController = null;
        }

        for (const [gameId, controller] of this.gameControllers) {
            controller.abort();
            console.log(`[${gameId}] Stream aborted.`);
        }
        this.gameControllers.clear();

        for (const [gameId, engine] of this.gameEngines) {
            engine.stop().catch(err => console.error(`[${gameId}] Engine stop error:`, err));
        }
        this.gameEngines.clear();

        this.activeGames.clear();
        this.dbGameIds.clear();
        this.savedPlies.clear();

        console.log("[Bot] Stopped.");
    }

    async streamEvents() {
        this.eventController = new AbortController();
        console.log("[Bot] Listening for events...");

        try {
            const res = await fetch("https://lichess.org/api/stream/event", {
                headers: this.authHeader,
                signal: this.eventController.signal,
            });

            if (!res.ok) {
                console.error("[Bot] Event stream failed:", res.statusText);
                setTimeout(() => this.streamEvents(), this.reconnectDelayMs);
                return;
            }

            await this.readNdjsonStream(res.body, this.eventController.signal, async (event) => {
                if (event.type === "challenge") {
                    await this.handleChallenge(event.challenge);
                } else if (event.type === "gameStart") {
                    this.playGame(event.game.id);
                }
            });
        } catch (err) {
            if (err.name === "AbortError") {
                console.log("[Bot] Event stream cancelled.");
                return;
            }
            console.error("[Bot] Event stream error:", err);
            setTimeout(() => this.streamEvents(), this.reconnectDelayMs);
        }
    }

    async handleChallenge(challenge) {
        // Lichess emits challenge events for both directions. Outgoing ones are
        // tracked elsewhere (huntNearRating / huntWeakestBot poll activeGames);
        // we don't need to act on them here.
        if (challenge.direction === "out") return;

        const variant = challenge.variant?.key;

        if (variant !== "standard") {
            console.log(`[Challenge ${challenge.id}] Declining — unsupported variant: ${variant}`);
            await this.declineChallenge(challenge.id, "variant");
            return;
        }

        if (this.activeGames.size >= this.maxConcurrentGames) {
            console.log(`[Challenge ${challenge.id}] Declining — at max concurrent games (${this.maxConcurrentGames})`);
            this.notifier.info(`Declining challenge ${challenge.id}`, {reason: "at_cap", active: this.activeGames.size});
            await this.declineChallenge(challenge.id, "later");
            return;
        }

        console.log(`[Challenge ${challenge.id}] Accepting`);
        await fetch(`https://lichess.org/api/challenge/${challenge.id}/accept`, {
            method: "POST",
            headers: this.authHeader,
        });
    }

    async declineChallenge(challengeId, reason = "generic") {
        const body = new URLSearchParams({reason});
        await fetch(`https://lichess.org/api/challenge/${challengeId}/decline`, {
            method: "POST",
            headers: this.formHeaders,
            body,
        }).catch(() => {});
    }

    async playGame(gameId) {
        if (this.activeGames.has(gameId)) return;
        this.activeGames.add(gameId);
        console.log(`[${gameId}] Game started.`);
        this.notifier.info(`Game started: ${gameId}`, {active: this.activeGames.size, max: this.maxConcurrentGames});

        const gameController = new AbortController();
        this.gameControllers.set(gameId, gameController);

        let engine;
        try {
            engine = this.engineFactory();
        } catch (err) {
            // EngineCapReached or any factory failure: don't try to play. Resign and bail.
            // Existing in-flight games keep running — we just don't add another.
            console.error(`[${gameId}] Engine factory rejected:`, err);
            this.notifier.warn(`Refused to spawn engine for ${gameId}`, {message: err?.message});
            try { await this.resignGame(gameId); } catch (_) {}
            this.activeGames.delete(gameId);
            this.gameControllers.delete(gameId);
            return;
        }
        this.gameEngines.set(gameId, engine);

        engine.on("fatal_error", async (err) => {
            console.error(`[${gameId}] !! ENGINE FATAL ERROR !! Resigning game.`, err);
            this.notifier.error(`Engine fatal in game ${gameId}`, {message: err?.message});
            try { await this.resignGame(gameId); } catch (_) {}
            gameController.abort();
        });

        let myColor = null;
        let initialFen = "startpos";
        let totalTimeMs = null;

        try {
            try {
                await engine.start();
            } catch (startErr) {
                console.error(`[${gameId}] !! ENGINE START FAILED !! Resigning game.`, startErr);
                try { await this.resignGame(gameId); } catch (_) {}
                throw startErr;
            }

            const res = await fetch(`https://lichess.org/api/bot/game/stream/${gameId}`, {
                headers: this.authHeader,
                signal: gameController.signal,
            });

            await this.readNdjsonStream(res.body, gameController.signal, async (obj) => {
                if (obj.type === "chatLine" || obj.type === "opponentGone") return;

                let movesStr = "";
                let timeInfo = {};
                let status = "started";
                let winner = null;

                if (obj.type === "gameFull") {
                    const whiteUsername = obj.white?.id || obj.white?.name || "ai";
                    const blackUsername = obj.black?.id || obj.black?.name || "ai";
                    const myId = this.botProfile.toLowerCase();
                    myColor = whiteUsername.toLowerCase() === myId ? "w" : "b";
                    initialFen = obj.initialFen || "startpos";
                    movesStr = obj.state?.moves || "";
                    timeInfo = extractTime(obj.state);
                    status = obj.state?.status ?? "started";
                    winner = obj.state?.winner ?? null;

                    totalTimeMs = obj.clock?.initial ?? null;

                    await engine.uciNewGame();
                    await this.createDbGame(gameId, {
                        whiteUsername,
                        blackUsername,
                        variant: obj.variant?.key || "standard",
                        rated: obj.rated ? 1 : 0,
                        timeControl: obj.clock
                            ? `${obj.clock.initial / 1000}+${obj.clock.increment / 1000}`
                            : null,
                        whiteRating: obj.white?.rating ?? null,
                        blackRating: obj.black?.rating ?? null,
                    });

                    await this.saveNewMoves(gameId, movesStr);

                } else if (obj.type === "gameState") {
                    movesStr = obj.moves || "";
                    timeInfo = extractTime(obj);
                    status = obj.status ?? "started";
                    winner = obj.winner ?? null;

                    await this.saveNewMoves(gameId, movesStr);
                } else {
                    return;
                }

                if (status !== "started") {
                    console.log(`[${gameId}] Game over: ${status}, winner: ${winner ?? "draw"}`);
                    await this.finalizeDbGame(gameId, status, winner);
                    gameController.abort();
                    return;
                }

                if (myColor && this.isMyTurn(initialFen, movesStr, myColor)) {
                    await this.makeMove(engine, gameId, initialFen, movesStr, myColor, timeInfo, totalTimeMs);
                }
            });

        } catch (err) {
            if (err.name !== "AbortError") {
                console.error(`[${gameId}] Game stream error:`, err);
            }
        } finally {
            try { await engine.stop(); } catch (e) { console.error(`[${gameId}] Engine stop error:`, e); }
            this.gameEngines.delete(gameId);
            this.activeGames.delete(gameId);
            this.gameControllers.delete(gameId);
            console.log(`[${gameId}] Cleaned up.`);
            this._tickAutoplay();
        }
    }

    async makeMove(engine, gameId, initialFen, movesStr, myColor, timeInfo, totalTimeMs) {
        console.log(`[${gameId}] My turn (${myColor}).`);

        const movesArray = movesStr.trim() === "" ? [] : movesStr.trim().split(" ");
        await engine.position(initialFen, movesArray);

        const myTime = myColor === "w" ? timeInfo.wtime : timeInfo.btime;
        const myInc  = myColor === "w" ? timeInfo.winc  : timeInfo.binc;
        const moveTimeMs = computeMoveTime(myTime, myInc, totalTimeMs);

        const rawMove = await engine.go({moveTime: moveTimeMs});
        const bestMove = normalizeMove(rawMove);
        console.log(`[${gameId}] Engine: ${bestMove} (allocated ${moveTimeMs}ms)`);

        if (!bestMove || bestMove === "(none)" || bestMove === "0000") {
            console.warn(`[${gameId}] No valid move — resigning.`);
            await this.resignGame(gameId);
            return;
        }

        await this.sendMove(gameId, bestMove);
    }

    isMyTurn(initialFen, movesStr, myColor) {
        let startColor = "w";
        if (initialFen && initialFen !== "startpos") {
            const parts = initialFen.split(" ");
            if (parts.length >= 2) startColor = parts[1];
        }
        const moveCount = movesStr.trim() === "" ? 0 : movesStr.trim().split(" ").length;
        const currentColor = moveCount % 2 === 0 ? startColor : (startColor === "w" ? "b" : "w");
        return currentColor === myColor;
    }

    async sendMove(gameId, move) {
        const res = await fetch(`https://lichess.org/api/bot/game/${gameId}/move/${move}`, {
            method: "POST",
            headers: this.authHeader,
        });
        if (!res.ok) {
            console.warn(`[${gameId}] Move rejected (${move}): ${await res.text()}`);
        }
    }

    async resignGame(gameId) {
        await fetch(`https://lichess.org/api/bot/game/${gameId}/resign`, {
            method: "POST",
            headers: this.authHeader,
        }).catch(() => {});
    }

    async upsertPlayer(username) {
        await db.insert(players).values({name: username}).onConflictDoNothing();
        const [row] = await db.select({id: players.id})
            .from(players)
            .where(eq(players.name, username));
        return row.id;
    }

    async createDbGame(lichessGameId, {whiteUsername, blackUsername, variant, rated, timeControl, whiteRating, blackRating}) {
        // Resume path: if a row for this lichessGameId exists (e.g. backend
        // restarted mid-game), reuse it instead of trying to re-insert.
        const existing = await db.select({id: games.id})
            .from(games)
            .where(eq(games.lichessGameId, lichessGameId))
            .limit(1);
        if (existing[0]) {
            this.dbGameIds.set(lichessGameId, existing[0].id);
            const moves = await db.select({id: gameMoves.id})
                .from(gameMoves)
                .where(eq(gameMoves.gameId, existing[0].id));
            this.savedPlies.set(lichessGameId, moves.length);
            return;
        }

        const [whitePlayerId, blackPlayerId] = await Promise.all([
            this.upsertPlayer(whiteUsername),
            this.upsertPlayer(blackUsername),
        ]);

        const [row] = await db.insert(games).values({
            whiteId: whitePlayerId,
            blackId: blackPlayerId,
            source: "lichess",
            lichessGameId,
            variant,
            rated,
            timeControl,
            whiteRating,
            blackRating,
        }).returning({id: games.id});

        this.dbGameIds.set(lichessGameId, row.id);
        this.savedPlies.set(lichessGameId, 0);
    }

    async saveNewMoves(lichessGameId, movesStr) {
        const dbGameId = this.dbGameIds.get(lichessGameId);
        if (dbGameId == null) return;

        const allMoves = movesStr.trim() === "" ? [] : movesStr.trim().split(" ");
        const savedPly = this.savedPlies.get(lichessGameId) ?? 0;
        const newMoves = allMoves.slice(savedPly);
        if (newMoves.length === 0) return;

        await db.insert(gameMoves).values(
            newMoves.map((uci, i) => ({
                gameId: dbGameId,
                ply: savedPly + i + 1,
                uci,
            }))
        );
        this.savedPlies.set(lichessGameId, allMoves.length);
    }

    async finalizeDbGame(lichessGameId, status, winner) {
        const dbGameId = this.dbGameIds.get(lichessGameId);
        if (dbGameId == null) return;

        await db.update(games)
            .set({
                result: mapResult(status, winner),
                termination: status,
                finishedAt: new Date().toISOString(),
            })
            .where(eq(games.id, dbGameId));

        this.dbGameIds.delete(lichessGameId);
        this.savedPlies.delete(lichessGameId);
    }

    async createChallenge(username, limit, increment, rated = true) {
        console.log(`Challenging ${username} (${limit}+${increment}, ${rated ? "rated" : "casual"})...`);
        const body = new URLSearchParams({
            "clock.limit": limit,
            "clock.increment": increment,
            rated: rated ? "true" : "false",
        });
        const res = await fetch(`https://lichess.org/api/challenge/${username}`, {
            method: "POST",
            headers: this.formHeaders,
            body,
        });
        if (!res.ok) throw new Error(await res.text());
        return res.json();
    }

    async createOpenChallenge(limit, increment, rated = true) {
        console.log(`Creating open challenge (${limit}+${increment}, ${rated ? "rated" : "casual"})...`);
        const body = new URLSearchParams({
            "clock.limit": limit,
            "clock.increment": increment,
            rated: rated ? "true" : "false",
        });
        const res = await fetch("https://lichess.org/api/challenge/open", {
            method: "POST",
            headers: this.formHeaders,
            body,
        });
        if (!res.ok) throw new Error(await res.text());
        return res.json();
    }

    async createAiChallenge(level, limit, increment) {
        console.log(`Challenging Stockfish level ${level}...`);
        const body = new URLSearchParams({
            level,
            "clock.limit": limit,
            "clock.increment": increment,
        });
        const res = await fetch("https://lichess.org/api/challenge/ai", {
            method: "POST",
            headers: this.formHeaders,
            body,
        });
        if (!res.ok) throw new Error(await res.text());
        return res.json();
    }

    async cancelChallenge(challengeId) {
        await fetch(`https://lichess.org/api/challenge/${challengeId}/cancel`, {
            method: "POST",
            headers: this.authHeader,
        }).catch(() => {});
    }

    // Pick a Lichess perf name (bullet/blitz/rapid/classical) from a time control.
    // Uses Lichess's own classification: estimated = initialSec + 40 * incSec.
    _perfFromTc(limitSec, incrementSec) {
        const est = limitSec + 40 * incrementSec;
        if (est < 180) return "bullet";
        if (est < 480) return "blitz";
        if (est < 1500) return "rapid";
        return "classical";
    }

    async _fetchMyRating(perf) {
        const res = await fetch("https://lichess.org/api/account", {headers: this.authHeader});
        if (!res.ok) throw new Error("Failed to fetch own profile");
        const profile = await res.json();
        const rating = profile.perfs?.[perf]?.rating;
        if (rating == null) throw new Error(`No ${perf} rating on profile yet`);
        return {rating, prov: !!profile.perfs[perf].prov};
    }

    // Challenge bots within ±window of our own rating for the given TC. Tries
    // up to `maxAttempts` candidates, ordered by closeness in rating; returns
    // the first one that accepts within ~5s.
    async huntNearRating(limit, increment, rated = true, {window = 200, maxAttempts = 10} = {}) {
        const perf = this._perfFromTc(limit, increment);
        const {rating: myRating, prov} = await this._fetchMyRating(perf);
        console.log(`[Hunt] My ${perf} rating: ${myRating}${prov ? " (provisional)" : ""}; window ±${window}`);

        const res = await fetch("https://lichess.org/api/bot/online?nb=300", {
            headers: {Accept: "application/x-ndjson"},
        });
        if (!res.ok) throw new Error("Failed to fetch online bots");

        const bots = [];
        await this.readNdjsonStream(res.body, null, (bot) => { bots.push(bot); });

        const candidates = bots
            .filter(b => b.id !== this.botProfile?.toLowerCase())
            .filter(b => b.perfs?.[perf]?.rating != null)
            .map(b => ({...b, _delta: Math.abs(b.perfs[perf].rating - myRating)}))
            .filter(b => b._delta <= window)
            .sort((a, b) => a._delta - b._delta)
            .slice(0, maxAttempts);

        if (candidates.length === 0) {
            throw new Error(`No bots within ±${window} of ${perf}=${myRating} (saw ${bots.length} online)`);
        }

        console.log(`[Hunt] ${candidates.length} candidates within ±${window} of ${myRating}`);

        for (const target of candidates) {
            const targetRating = target.perfs[perf].rating;
            console.log(`[Hunt] Challenging ${target.username} (${perf}=${targetRating}, Δ${target._delta})...`);

            let challengeId;
            try {
                const body = new URLSearchParams({
                    "clock.limit": limit,
                    "clock.increment": increment,
                    rated: rated ? "true" : "false",
                });
                const cRes = await fetch(`https://lichess.org/api/challenge/${target.username}`, {
                    method: "POST",
                    headers: this.formHeaders,
                    body,
                });
                if (!cRes.ok) { continue; }
                challengeId = (await cRes.json()).id;
            } catch { continue; }

            for (let i = 0; i < 5; i++) {
                await new Promise(r => setTimeout(r, this.huntPollIntervalMs));
                if (this.activeGames.has(challengeId)) {
                    return {status: "success", message: `Playing vs ${target.username} (${targetRating})`, gameId: challengeId, myRating, targetRating};
                }
            }

            await this.cancelChallenge(challengeId);
        }

        throw new Error(`Hunt failed — none of ${candidates.length} near-rating bots accepted`);
    }

    async huntWeakestBot(limit, increment, rated = true) {
        console.log(`Hunting weakest bot (${limit}+${increment}, ${rated ? "rated" : "casual"})...`);

        const res = await fetch("https://lichess.org/api/bot/online?nb=50", {
            headers: {Accept: "application/x-ndjson"},
        });
        if (!res.ok) throw new Error("Failed to fetch online bots");

        const bots = [];
        await this.readNdjsonStream(res.body, null, (bot) => { bots.push(bot); });

        const candidates = bots
            .filter(b => b.id !== this.botProfile?.toLowerCase())
            .filter(b => b.perfs?.blitz?.rating != null)
            .sort((a, b) => a.perfs.blitz.rating - b.perfs.blitz.rating)
            .slice(0, 10);

        if (candidates.length === 0) throw new Error("No candidates found");

        for (const target of candidates) {
            console.log(`Challenging ${target.username} (${target.perfs.blitz.rating})...`);

            let challengeId;
            try {
                const body = new URLSearchParams({
                    "clock.limit": limit,
                    "clock.increment": increment,
                    rated: rated ? "true" : "false",
                });
                const cRes = await fetch(`https://lichess.org/api/challenge/${target.username}`, {
                    method: "POST",
                    headers: this.formHeaders,
                    body,
                });
                if (!cRes.ok) { console.log(`Skipping ${target.username}`); continue; }
                challengeId = (await cRes.json()).id;
            } catch { continue; }

            for (let i = 0; i < 5; i++) {
                await new Promise(r => setTimeout(r, this.huntPollIntervalMs));
                if (this.activeGames.has(challengeId)) {
                    return {status: "success", message: `Playing vs ${target.username}`, gameId: challengeId};
                }
            }

            await this.cancelChallenge(challengeId);
        }

        throw new Error("Hunt failed — all candidates ignored our challenges.");
    }

    async readNdjsonStream(readableStream, signal, callback) {
        const reader = readableStream.getReader();
        const decoder = new TextDecoder();
        let buffer = "";

        const onAbort = () => reader.cancel().catch(() => {});
        signal?.addEventListener("abort", onAbort, {once: true});

        try {
            while (true) {
                const {done, value} = await reader.read();
                if (done) break;

                buffer += decoder.decode(value, {stream: true});
                const lines = buffer.split("\n");
                buffer = lines.pop();

                for (const line of lines) {
                    if (!line.trim()) continue;
                    try {
                        await callback(JSON.parse(line));
                    } catch (e) {
                        console.error("[NDJSON] Callback error:", e);
                    }
                }
            }
        } finally {
            signal?.removeEventListener("abort", onAbort);
            reader.releaseLock();
        }
    }
}

export function extractTime(state) {
    if (!state) return {};
    return {
        wtime: state.wtime,
        btime: state.btime,
        winc: state.winc,
        binc: state.binc,
    };
}

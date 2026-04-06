import { UciEngine } from "./engineManager.js";

export class LichessBot {
    constructor(token, engine) {
        this.token = token;
        this.engine = engine;
        this.headers = {
            Authorization: `Bearer ${this.token}`,
            "Content-Type": "application/json",
        };
        this.formHeaders = {
            Authorization: `Bearer ${this.token}`,
            "Content-Type": "application/x-www-form-urlencoded"
        };
        this.authHeader = {
            Authorization: `Bearer ${this.token}`
        };

        this.activeGames = new Set();
        this.botProfile = null;

        this.engine.on("fatal_error", (err) => {
            console.error("!! ENGINE FATAL ERROR !!", err);
        });
    }

    async start() {
        console.log("Starting Lichess Bot...");

        const profileReq = await fetch("https://lichess.org/api/account", { headers: this.authHeader });
        if (!profileReq.ok) throw new Error("Failed to fetch bot profile");
        const profile = await profileReq.json();
        this.botProfile = profile.id;
        console.log(`Logged in as: ${this.botProfile} (Bot Account)`);

        await this.engine.start();

        this.streamEvents();
    }

    async streamEvents() {
        console.log("Listening for events...");
        const response = await fetch("https://lichess.org/api/stream/event", {
            headers: this.authHeader,
        });

        if (!response.ok) {
            console.error("Event stream failed:", response.statusText);
            setTimeout(() => this.streamEvents(), 5000);
            return;
        }

        await this.readNdjsonStream(response.body, async (event) => {
            if (event.type === "challenge") {
                await this.acceptChallenge(event.challenge.id);
            } else if (event.type === "gameStart") {
                this.playGame(event.game.id);
            }
        });
    }

    async acceptChallenge(challengeId) {
        console.log(`Accepting challenge: ${challengeId}`);
        await fetch(`https://lichess.org/api/challenge/${challengeId}/accept`, {
            method: "POST",
            headers: this.authHeader,
        });
    }

    async playGame(gameId) {
        if (this.activeGames.has(gameId)) return;
        this.activeGames.add(gameId);
        console.log(`[${gameId}] Game started.`);

        const response = await fetch(`https://lichess.org/api/bot/game/stream/${gameId}`, {
            headers: this.authHeader,
        });

        let myColor = null;
        let initialFen = "startpos";

        await this.readNdjsonStream(response.body, async (obj) => {
            let movesStr = "";
            let timeInfo = {};
            let isGameActive = true;

            if (obj.type === "gameFull") {
                const whiteId = obj.white.id ? obj.white.id.toLowerCase() : "ai";
                const myId = this.botProfile.toLowerCase();
                myColor = (whiteId === myId) ? 'w' : 'b';

                initialFen = obj.initialFen;
                movesStr = obj.state.moves;
                timeInfo = { wtime: obj.state.wtime, btime: obj.state.btime, winc: obj.state.winc, binc: obj.state.binc };

                if (obj.state.status && obj.state.status !== "started") isGameActive = false;
            }
            else if (obj.type === "gameState") {
                movesStr = obj.moves;
                timeInfo = { wtime: obj.wtime, btime: obj.btime, winc: obj.winc, binc: obj.binc };
                if (obj.status && obj.status !== "started") isGameActive = false;
            } else {
                return;
            }

            if (!isGameActive) {
                console.log(`[${gameId}] Game Over.`);
                return;
            }

            if (this.isMyTurn(initialFen, movesStr, myColor)) {
                console.log(`[${gameId}] My turn (${myColor}).`);

                const movesArray = movesStr.trim() === "" ? [] : movesStr.trim().split(" ");
                await this.engine.position(initialFen, movesArray);

                const bestMove = await this.engine.go({
                    whiteTime: timeInfo.wtime,
                    blackTime: timeInfo.btime,
                    whiteInc: timeInfo.winc,
                    blackInc: timeInfo.binc
                });

                console.log(`[${gameId}] Engine move: ${bestMove}`);

                if (bestMove && bestMove !== "(none)" && bestMove !== "0000") {
                    await this.sendMove(gameId, bestMove);
                }
            }
        });

        this.activeGames.delete(gameId);
    }

    isMyTurn(initialFen, movesStr, myColor) {
        let startColor = 'w';
        if (initialFen !== "startpos") {
            const parts = initialFen.split(" ");
            if (parts.length >= 2) startColor = parts[1];
        }

        const moves = movesStr.trim();
        const moveCount = moves === "" ? 0 : moves.split(" ").length;
        const isStartColorTurn = (moveCount % 2 === 0);
        const currentTurnColor = isStartColorTurn ? startColor : (startColor === 'w' ? 'b' : 'w');

        return currentTurnColor === myColor;
    }

    async sendMove(gameId, move) {
        const url = `https://lichess.org/api/bot/game/${gameId}/move/${move}`;

        const res = await fetch(url, {
            method: "POST",
            headers: this.authHeader
        });

        if (!res.ok) {
            const err = await res.text();
            console.warn(`[${gameId}] Move rejected (${move}): ${err}`);
        }
    }

    async createChallenge(username, limit, increment) {
        console.log(`Challenging ${username}...`);
        const body = new URLSearchParams();
        body.append("clock.limit", limit);
        body.append("clock.increment", increment);
        body.append("rated", "true");

        const res = await fetch(`https://lichess.org/api/challenge/${username}`, {
            method: "POST",
            headers: this.formHeaders, // Form Headers
            body: body
        });
        if (!res.ok) throw new Error(await res.text());
        return await res.json();
    }

    async createOpenChallenge(limit, increment) {
        console.log(`Creating Open Challenge...`);
        const body = new URLSearchParams();
        body.append("clock.limit", limit);
        body.append("clock.increment", increment);
        body.append("rated", "true");

        const res = await fetch("https://lichess.org/api/challenge/open", {
            method: "POST",
            headers: this.formHeaders,
            body: body
        });
        if (!res.ok) throw new Error(await res.text());
        return await res.json();
    }

    async createAiChallenge(level, limit, increment) {
        console.log(`Challenging Stockfish Level ${level}...`);
        const body = new URLSearchParams();
        body.append("level", level);
        body.append("clock.limit", limit);
        body.append("clock.increment", increment);

        const res = await fetch("https://lichess.org/api/challenge/ai", {
            method: "POST",
            headers: this.formHeaders,
            body: body
        });
        if (!res.ok) throw new Error(await res.text());
        return await res.json();
    }

    async readNdjsonStream(readableStream, callback) {
        const reader = readableStream.getReader();
        const decoder = new TextDecoder();
        let buffer = "";

        while (true) {
            const { done, value } = await reader.read();
            if (done) break;
            buffer += decoder.decode(value, { stream: true });
            let lines = buffer.split("\n");
            buffer = lines.pop();
            for (const line of lines) {
                if (line.trim()) {
                    try { await callback(JSON.parse(line)); } catch (e) { console.error(e); }
                }
            }
        }
    }

    /**
     * Scans online bots and challenges the one with the lowest rating
     */
    /**
     * NEW: Cancel a pending challenge
     */
    async cancelChallenge(challengeId) {
        try {
            await fetch(`https://lichess.org/api/challenge/${challengeId}/cancel`, {
                method: "POST",
                headers: this.authHeader
            });
        } catch (e) {
            // Ignore errors if it's already gone
        }
    }

    /**
     * "Find & Destroy" - Robust Hunting Loop
     */
    async huntWeakestBot(limit, increment) {
        console.log("Starting Hunt for Weakest Bot...");

        const res = await fetch("https://lichess.org/api/bot/online?nb=50", {
            headers: { Accept: "application/x-ndjson" }
        });
        if (!res.ok) throw new Error("Failed to fetch bots");

        const bots = [];
        const decoder = new TextDecoder();
        const reader = res.body.getReader();
        let buffer = "";
        while (true) {
            const { done, value } = await reader.read();
            if (done) break;
            buffer += decoder.decode(value, { stream: true });
            let lines = buffer.split("\n");
            buffer = lines.pop();
            for (const line of lines) {
                if (line.trim()) {
                    try { bots.push(JSON.parse(line)); } catch (e) {}
                }
            }
            if (bots.length >= 50) break;
        }

        const candidates = bots
        .filter(b => b.perfs?.blitz?.rating)
        .sort((a, b) => a.perfs.blitz.rating - b.perfs.blitz.rating)
        .slice(0, 10);

        if (candidates.length === 0) throw new Error("No candidates found");

        console.log(`Found ${candidates.length} candidates. Starting hunt...`);

        for (const target of candidates) {
            console.log(`Attacking ${target.username} (${target.perfs.blitz.rating})...`);

            let challengeData = null;
            try {
                const body = new URLSearchParams();
                body.append("clock.limit", limit);
                body.append("clock.increment", increment);
                body.append("rated", "true");

                const cRes = await fetch(`https://lichess.org/api/challenge/${target.username}`, {
                    method: "POST",
                    headers: this.formHeaders,
                    body: body
                });

                if (cRes.ok) {
                    challengeData = await cRes.json();
                } else {
                    console.log(`Failed to challenge ${target.username}, skipping.`);
                    continue;
                }
            } catch (err) {
                continue;
            }

            const challengeId = challengeData.id;
            for (let i = 0; i < 5; i++) {
                await new Promise(r => setTimeout(r, 1000));
                if (this.activeGames.has(challengeId)) {
                    console.log(`Target Acquired! Game ${challengeId} vs ${target.username} started.`);
                    return {
                        status: "success",
                        message: `Hunt Successful! Playing vs ${target.username}`,
                        gameId: challengeId
                    };
                }
            }

            console.log(`${target.username} ignored us. Cancelling and finding new target...`);
            await this.cancelChallenge(challengeId);
        }

        throw new Error("Hunt failed. All 10 weakest bots ignored our challenges.");
    }
}
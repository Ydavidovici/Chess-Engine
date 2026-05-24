import express from "express";
import cors from "cors";
import path from "node:path";
import {EngineManager, UciEngine, EngineCapReached} from "./engineManager.js";
import {LichessBot} from "./lichessBot.js";
import {Notifier, nullNotifier, wrapConsoleForNotifier} from "./notifier.js";
import {ApiTransport} from "./apiTransport.js";

export function createApp({manager, lichessEngineFactory, mainEnginePath, maxConcurrentGames = 4, notifier = nullNotifier, getToken = () => process.env.lichess_api_token} = {}) {
    const app = express();

    app.use(cors({
        origin: ["http://localhost:5173", "http://127.0.0.1:5173"],
    }));
    app.use(express.json());

    let lichessBotInstance = null;

    app.get("/api/health", (req, res) => {
        try {
            const mainEngine = manager.getEngine("Main");
            res.json({
                status: "ok",
                engine: mainEngine.ready ? "ready" : "starting",
                engineCount: manager.count(),
                botRunning: !!lichessBotInstance,
                activeGames: lichessBotInstance ? lichessBotInstance.activeGames.size : 0,
                uptimeSec: Math.round(process.uptime()),
            });
        } catch (err) {
            res.status(503).json({status: "degraded", error: err.message});
        }
    });

    app.post("/api/engine/analysis", async (req, res) => {
        try {
            const {fen, depth = 10} = req.body ?? {};
            if (!fen) return res.status(400).json({error: "FEN required"});

            const mainEngine = manager.getEngine("Main");
            await mainEngine.position(fen);
            const bestMove = await mainEngine.go({depth});

            res.json({bestMove, depth});
        } catch (err) {
            console.error("Analysis Error:", err);
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/engine/go", async (req, res) => {
        try {
            const {fen, moves, options} = req.body ?? {};
            const mainEngine = manager.getEngine("Main");

            await mainEngine.position(fen || "startpos", moves || []);
            const bestMove = await mainEngine.go(options || {depth: 7});

            res.json({bestMove});
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/engine/reset", async (req, res) => {
        await manager.getEngine("Main").uciNewGame();
        res.json({status: "reset_complete"});
    });

    app.post("/api/engine/bench", async (req, res) => {
        try {
            const {mode = "depth", depth = 9, timeLimit = 30000, evalTime = 2000} = req.body ?? {};
            console.log(`Starting benchmark [Mode: ${mode}, Depth: ${depth}, Time: ${timeLimit}ms]...`);

            const mainEngine = manager.getEngine("Main");
            const results = await mainEngine.bench({mode, depth, timeLimit, evalTime});

            console.log("Benchmark results:", results);
            res.json({status: "success", data: results});
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/engine/cancel", async (req, res) => {
        try {
            await manager.shutdownEngine("Main");
            await manager.registerEngine("Main", mainEnginePath);
            res.json({status: "success", message: "Engine reset to cancel task."});
        } catch (err) {
            console.error("Cancel failed:", err);
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/lichess/start", async (req, res) => {
        if (lichessBotInstance) {
            return res.status(400).json({error: "Bot is already running."});
        }

        const token = getToken();
        if (!token) {
            return res.status(400).json({error: "Missing Lichess Token"});
        }

        const instance = new LichessBot(token, lichessEngineFactory, {maxConcurrentGames, notifier});
        try {
            await instance.start();
            lichessBotInstance = instance;
            notifier.info("Lichess bot started", {maxConcurrentGames});
            res.json({status: "success", message: `Lichess Bot started (max ${maxConcurrentGames} concurrent games).`});
        } catch (err) {
            console.error("Failed to start Lichess Bot:", err);
            notifier.error("Lichess bot failed to start", {message: err?.message});
            try { instance.stop(); } catch (_) {}
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/lichess/stop", async (req, res) => {
        if (!lichessBotInstance) {
            return res.json({status: "ignored", message: "Bot was not running."});
        }

        try {
            if (typeof lichessBotInstance.stop === "function") {
                lichessBotInstance.stop();
            }
            lichessBotInstance = null;
            res.json({status: "success", message: "Lichess Bot stopped."});
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.get("/api/lichess/status", (req, res) => {
        const rateLimitedFor = lichessBotInstance ? lichessBotInstance._rateLimitRemainingSec() : 0;
        if (lichessBotInstance && !lichessBotInstance.botProfile) {
            lichessBotInstance._ensureProfile().catch(() => {});
        }
        res.json({
            running: !!lichessBotInstance,
            profile: lichessBotInstance ? lichessBotInstance.botProfile : null,
            activeGames: lichessBotInstance ? Array.from(lichessBotInstance.activeGames) : [],
            maxConcurrentGames: lichessBotInstance ? lichessBotInstance.maxConcurrentGames : maxConcurrentGames,
            rateLimitedFor,
            declinedCount: lichessBotInstance ? lichessBotInstance.recentlyDeclined.size : 0,
        });
    });

    app.post("/api/lichess/challenge/open", async (req, res) => {
        if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
        const {limit = 180, increment = 0, rated = true} = req.body ?? {};
        try {
            const result = await lichessBotInstance.createOpenChallenge(limit, increment, rated);
            res.json({status: "success", data: result});
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/lichess/challenge/ai", async (req, res) => {
        if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
        const {level = 1, limit = 180, increment = 0} = req.body ?? {};
        try {
            const result = await lichessBotInstance.createAiChallenge(level, limit, increment);
            res.json({status: "success", data: result});
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/lichess/challenge/weakest", async (req, res) => {
        if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
        const {limit = 180, increment = 0, rated = true} = req.body ?? {};
        try {
            const result = await lichessBotInstance.huntWeakestBot(limit, increment, rated);
            res.json(result);
        } catch (err) {
            res.status(500).json({error: err.message});
        }
    });

    app.post("/api/lichess/autoplay/start", (req, res) => {
        if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
        const {limit = 180, increment = 2, rated = true, target = 1, mode = "near", window = 200} = req.body ?? {};
        lichessBotInstance.startAutoplay({limit, increment, rated, target, mode, window});
        res.json({status: "success", autoplay: lichessBotInstance.autoplayStatus()});
    });

    app.post("/api/lichess/autoplay/stop", (req, res) => {
        if (!lichessBotInstance) return res.status(400).json({error: "Bot not running"});
        lichessBotInstance.stopAutoplay();
        res.json({status: "success"});
    });

    app.get("/api/lichess/autoplay/status", (req, res) => {
        if (!lichessBotInstance) return res.json({enabled: false, botRunning: false});
        res.json(lichessBotInstance.autoplayStatus());
    });

    return {app, getBotInstance: () => lichessBotInstance};
}

if (import.meta.main) {
    const PROD_PATH = path.join(import.meta.dir, "myengine");
    const DEV_PATH = path.resolve(import.meta.dir, "../../engines/myengine/build/myengine.exe");
    const FIX_DEV_PATH = path.resolve(import.meta.dir, "../../../engines/myengine/build/myengine");

    let MY_ENGINE_PATH = null;

    // Explicit override for deployments (e.g. systemd units). Wins over auto-detection.
    if (process.env.ENGINE_PATH) {
        if (await Bun.file(process.env.ENGINE_PATH).exists()) {
            console.log("✅ Using ENGINE_PATH from environment");
            MY_ENGINE_PATH = process.env.ENGINE_PATH;
        } else {
            console.error(`❌ CRITICAL: ENGINE_PATH set but not found: ${process.env.ENGINE_PATH}`);
            process.exit(1);
        }
    } else if (await Bun.file(PROD_PATH).exists()) {
        console.log("✅ Running in Production Mode (Local Binary)");
        MY_ENGINE_PATH = PROD_PATH;
    } else if (await Bun.file(DEV_PATH).exists()) {
        console.log("⚠️  Running in Dev Mode (External Binary)");
        MY_ENGINE_PATH = DEV_PATH;
    } else if (await Bun.file(FIX_DEV_PATH).exists()) {
        console.log("⚠️  Running in Dev Mode (Deep Nested Fallback)");
        MY_ENGINE_PATH = FIX_DEV_PATH;
    }

    if (!MY_ENGINE_PATH) {
        console.error("❌ CRITICAL: Could not find chess engine binary!");
        process.exit(1);
    }
    console.log(`♟️  Engine Path: ${MY_ENGINE_PATH}`);

    // --- Notifier ---
    // Note: no ConsoleTransport here on purpose. wrapConsoleForNotifier below
    // forwards every console.* call through the notifier (so Discord/etc. see
    // them) AND still prints to the terminal — adding ConsoleTransport would
    // double-print every notify call.
    const transports = [];
    const apiTransport = new ApiTransport();
    if (apiTransport.enabled) {
        transports.push(apiTransport);
        console.log(`[Server] ApiTransport enabled → ${apiTransport.url}`);
    } else {
        console.log("[Server] ApiTransport disabled (set API_NOTIFY_URL + API_NOTIFY_TOKEN to enable).");
    }
    const notifier = new Notifier({transports});
    const restoreConsole = wrapConsoleForNotifier(notifier);

    // --- Manager (with hard cap independent of LICHESS_MAX_GAMES) ---
    // Default: one slot for "Main" analysis engine + Lichess game slots. Buffer of 2 for safety.
    const LICHESS_MAX_GAMES = parseInt(process.env.LICHESS_MAX_GAMES ?? "4", 10);
    const ENGINE_HARD_CAP = parseInt(process.env.ENGINE_HARD_CAP ?? String(LICHESS_MAX_GAMES + 3), 10);

    const manager = new EngineManager({maxEngines: ENGINE_HARD_CAP, notifier});
    await manager.registerEngine("Main", MY_ENGINE_PATH);

    // Lichess engine factory: route every spawn through the manager's cap.
    // Game engines aren't long-lived registered engines (they're per-game), so we
    // build them through reserveEngine and track them ourselves via gameEngines.
    const lichessEngineFactory = () => {
        // The reservation check is what enforces the cap. Existing engines are untouched.
        const label = `game-${Date.now()}-${Math.random().toString(36).slice(2, 7)}`;
        if (!manager.hasCapacity()) {
            throw new EngineCapReached(manager.maxEngines, manager.count());
        }
        return new UciEngine(MY_ENGINE_PATH, {notifier, label});
    };

    const {app} = createApp({manager, lichessEngineFactory, mainEnginePath: MY_ENGINE_PATH, maxConcurrentGames: LICHESS_MAX_GAMES, notifier});

    const PORT = process.env.PORT || 8000;
    const server = app.listen(PORT, () => {
        console.log(`Backend listening on http://localhost:${PORT}`);
        notifier.info("Backend started", {port: PORT, engineCap: ENGINE_HARD_CAP, lichessMax: LICHESS_MAX_GAMES});
    });

    // --- Discord bot (opt-in) ---
    let discordBot = null;
    if (process.env.DISCORD_BOT_TOKEN && process.env.DISCORD_CHANNEL_ID) {
        try {
            const {createDiscordBot} = await import("./discordBot.js");
            discordBot = await createDiscordBot({
                token: process.env.DISCORD_BOT_TOKEN,
                channelId: process.env.DISCORD_CHANNEL_ID,
                notifier,
                healthUrl: process.env.HEALTH_URL ?? `http://localhost:${PORT}/api/health`,
            });
            console.log("[Server] Discord bot connected.");
        } catch (err) {
            console.error("[Server] Discord bot init failed:", err);
            notifier.warn("Discord bot init failed", {message: err?.message});
        }
    } else {
        console.log("[Server] Discord disabled (set DISCORD_BOT_TOKEN + DISCORD_CHANNEL_ID to enable).");
    }

    // --- Shutdown path used by signals and fatal errors ---
    let shuttingDown = false;
    const shutdown = async (reason, exitCode = 0) => {
        if (shuttingDown) return;
        shuttingDown = true;
        console.log(`\n[Server] Shutting down (${reason})...`);
        try { await notifier.flush(); } catch (_) {}
        try { server.close(); } catch (_) {}
        try { await manager.shutdownAll(); } catch (e) { console.error("[Server] shutdownAll error:", e); }
        if (discordBot) {
            try { await discordBot.stop(); } catch (e) { console.error("[Server] Discord stop error:", e); }
        }
        try { restoreConsole(); } catch (_) {}
        process.exit(exitCode);
    };

    process.on("SIGINT",  () => shutdown("SIGINT",  0));
    process.on("SIGTERM", () => shutdown("SIGTERM", 0));

    // --- Global error handlers: log + notify + exit (supervisor restarts) ---
    const handleFatal = async (kind, err) => {
        console.error(`[Server] !! ${kind} !!`, err);
        try { await notifier.fatal(`${kind}: process exiting`, {message: err?.message, stack: err?.stack?.split("\n").slice(0, 5).join("\n")}); } catch (_) {}
        await shutdown(kind, 1);
    };

    process.on("uncaughtException",  (err)    => handleFatal("uncaughtException",  err));
    process.on("unhandledRejection", (reason) => handleFatal("unhandledRejection", reason instanceof Error ? reason : new Error(String(reason))));
}

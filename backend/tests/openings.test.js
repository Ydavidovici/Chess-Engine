import { describe, it, expect, mock, beforeEach } from "bun:test";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { LichessBot } from "../src/lichessBot.js";
import { spawn } from "bun";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

describe("Opening Book Features", () => {
    describe("downloadBook.js", () => {
        it("should execute the script successfully without crashing", async () => {
            // We use a fast, tiny URL (the repo's README) just to test the streaming logic
            // without downloading a 15MB file and spamming Github in CI tests.
            const fakeUrl = "https://raw.githubusercontent.com/gmcheems-org/free-opening-books/master/README.md";
            const scriptPath = path.resolve(__dirname, "../src/scripts/downloadBook.js");
            
            const testOutPath = path.resolve(__dirname, "test-book-output.bin");
            const proc = spawn({
                cmd: ["bun", "run", scriptPath, fakeUrl],
                env: { ...process.env, BOOK_OUT_PATH: testOutPath },
                stdout: "pipe",
                stderr: "pipe"
            });
            
            const exitCode = await proc.exited;
            expect(exitCode).toBe(0);
            
            const stdout = await new Response(proc.stdout).text();
            expect(stdout).toContain("Successfully downloaded and saved");
            
            // Clean up the test artifact
            await Bun.file(testOutPath).delete().catch(() => {});
        }, 10000);
    });

    describe("LichessBot Autoplay Openings", () => {
        let fakeEngine;
        let bot;

        beforeEach(() => {
            fakeEngine = {
                position: mock(async () => {}),
                setOption: mock(async () => {}),
                start: mock(async () => {}),
                go: mock(async () => "a1a2"),
                goWithEval: mock(async () => ({ bestMove: "a1a2" }))
            };
            bot = new LichessBot("fake_token", () => fakeEngine, { maxConcurrentGames: 1 });
            bot.sendMove = mock(async () => true);
        });

        it("should force specific opening move if openingId matches OPENINGS dict", async () => {
            // Sicilian is defined as: e2e4 c7c5
            bot.startAutoplay({ openingId: "sicilian" });
            bot.gameOpenings.set("game123", "sicilian");
            
            // Scenario: We are black. White just played e2e4.
            // Expected: Bot skips engine and instantly plays c7c5
            await bot.makeMove(fakeEngine, "game123", "startpos", "e2e4", "black", {}, 1000);
            
            expect(bot.sendMove).toHaveBeenCalledWith("game123", "c7c5");
            expect(fakeEngine.position).not.toHaveBeenCalled();
        });

        it("should fallback to engine if moves diverge from forced opening", async () => {
            bot.startAutoplay({ openingId: "sicilian" }); // e2e4 c7c5
            bot.gameOpenings.set("game123", "sicilian");
            
            // Scenario: White played d2d4 (diverged from e2e4)
            await bot.makeMove(fakeEngine, "game123", "startpos", "d2d4", "black", {}, 1000);
            
            // It should NOT force c7c5, but send the engine's move a1a2
            expect(bot.sendMove).toHaveBeenCalledWith("game123", "a1a2");
            // It SHOULD ask the engine
            expect(fakeEngine.position).toHaveBeenCalledWith("startpos", ["d2d4"]);
        });

        it("should fallback to engine if the specific opening is over", async () => {
            bot.startAutoplay({ openingId: "sicilian" }); // e2e4 c7c5
            bot.gameOpenings.set("game123", "sicilian");
            
            // Scenario: Both e2e4 and c7c5 have been played. Next move is White's. Then Black's turn again.
            await bot.makeMove(fakeEngine, "game123", "startpos", "e2e4 c7c5 g1f3", "black", {}, 1000);
            
            expect(bot.sendMove).toHaveBeenCalledWith("game123", "a1a2");
            expect(fakeEngine.position).toHaveBeenCalledWith("startpos", ["e2e4", "c7c5", "g1f3"]);
        });

        it("should apply whiteOpeningId when playing as white and blackOpeningId when playing as black", async () => {
            bot.startAutoplay({ whiteOpeningId: "c4", blackOpeningId: "sicilian" });
            
            // White game (movesStr is empty initially)
            await bot.makeMove(fakeEngine, "gameW", "startpos", "", "white", {}, 1000);
            expect(bot.gameOpenings.get("gameW")).toBe("c4");
            expect(bot.sendMove).toHaveBeenCalledWith("gameW", "c2c4");
            
            // Black game (white played e2e4)
            await bot.makeMove(fakeEngine, "gameB", "startpos", "e2e4", "black", {}, 1000);
            expect(bot.gameOpenings.get("gameB")).toBe("sicilian");
            expect(bot.sendMove).toHaveBeenCalledWith("gameB", "c7c5");
        });
    });
});

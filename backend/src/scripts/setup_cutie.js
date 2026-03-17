import { spawn } from "node:child_process";
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

// Adjusted paths: __dirname is backend/src/scripts/
// Go up 3 levels to reach the project root
const TOOLS_DIR = path.join(__dirname, "../../../tools");
const ENGINES_DIR = path.join(__dirname, "../../../engines");
const BOOK_PATH = path.join(TOOLS_DIR, "UHO_4060_v1.epd");

async function downloadAndExtract(url, destFolder, toolName) {
    console.log(`[${toolName}] Downloading from ${url}...`);

    const zipPath = path.join(destFolder, `${toolName.replace(/\s+/g, "_")}.zip`);
    await fs.mkdir(destFolder, { recursive: true });

    const response = await fetch(url);
    if (!response.ok) throw new Error(`Failed to fetch ${toolName}: ${response.statusText}`);

    const arrayBuffer = await response.arrayBuffer();
    await fs.writeFile(zipPath, Buffer.from(arrayBuffer));
    console.log(`[${toolName}] Extracting...`);

    await new Promise((resolve, reject) => {
        const ps = spawn("powershell.exe", [
            "-nologo", "-noprofile", "-command",
            `Expand-Archive -Path '${zipPath}' -DestinationPath '${destFolder}' -Force`
        ]);

        ps.on("close", (code) => {
            if (code === 0) resolve();
            else reject(new Error(`Extraction failed with code ${code}`));
        });
    });

    await fs.unlink(zipPath);
    console.log(`[${toolName}] ✅ Ready.`);
}

async function downloadGithubEngine(repo, destFolder, toolName) {
    console.log(`[${toolName}] Fetching latest release info from GitHub (${repo})...`);

    const res = await fetch(`https://api.github.com/repos/${repo}/releases/latest`);
    if (!res.ok) throw new Error(`GitHub API failed for ${repo}: ${res.statusText}`);

    const data = await res.json();
    if (!data.assets) throw new Error(`No assets found for ${repo}`);

    const asset = data.assets.find(a =>
        a.name.toLowerCase().includes("win") &&
        a.name.toLowerCase().endsWith(".zip") &&
        !a.name.toLowerCase().includes("arm")
    );

    if (!asset) throw new Error(`No Windows zip found for ${repo}`);

    await downloadAndExtract(asset.browser_download_url, destFolder, toolName);
}

async function runSetup() {
    console.log("=== Setting up Massive Chess Engine Tournament Environment ===\n");
    await fs.mkdir(TOOLS_DIR, { recursive: true });
    await fs.mkdir(ENGINES_DIR, { recursive: true });

    // 1. Download Massive Opening Book (UHO 246,000 positions)
    // 1. Download Massive Opening Book (UHO 246,000 positions)
    console.log("[Book] Downloading massive UHO opening book...");
    const bookUrl = "https://github.com/official-stockfish/books/raw/master/UHO_4060_v1.epd.zip";

    // We can just reuse our zip helper instead of trying to fetch raw text!
    await downloadAndExtract(bookUrl, TOOLS_DIR, "UHO_Book");

    console.log(`[Book] ✅ Saved 246k positions to: ${BOOK_PATH}\n`);

    console.log("[Cutechess] Finding latest release...");
    await downloadGithubEngine("cutechess/cutechess", path.join(TOOLS_DIR, "cutechess"), "Cutechess-CLI");
    console.log("");

    // 3. Download the Baseline Engines
    const tscpUrl = "http://www.tckerrigan.com/Chess/TSCP/tscp181.zip";
    await downloadAndExtract(tscpUrl, path.join(ENGINES_DIR, "tscp"), "TSCP");

    // We download Stockfish once, but we will spawn it multiple times with different node restrictions
    await downloadGithubEngine("official-stockfish/Stockfish", path.join(ENGINES_DIR, "stockfish"), "Stockfish");

    console.log("\n🚀 Setup Complete!");
    console.log("Your engines and massive opening book are ready.");
}

runSetup().catch(console.error);
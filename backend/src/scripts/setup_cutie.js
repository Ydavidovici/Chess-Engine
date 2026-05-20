import { spawn } from "node:child_process";
import fs from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const TOOLS_DIR = path.join(__dirname, "../../../tools");
const ENGINES_DIR = path.join(__dirname, "../../../engines");
const BOOK_PATH = path.join(TOOLS_DIR, "UHO_4060_v1.epd");

async function exists(path) {
    try {
        await fs.access(path);
        return true;
    } catch {
        return false;
    }
}

async function downloadAndExtract(url, destFolder, toolName) {
    if (await exists(destFolder)) {
        const files = await fs.readdir(destFolder);
        if (files.length > 0) {
            console.log(`[${toolName}] ✅ Folder exists and is not empty. Skipping download.`);
            return;
        }
    }

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
    if (await exists(destFolder)) {
        const files = await fs.readdir(destFolder);
        if (files.length > 0) {
            console.log(`[${toolName}] ✅ Folder exists. Skipping GitHub fetch.`);
            return;
        }
    }

    console.log(`[${toolName}] Fetching latest release info from GitHub (${repo})...`);
    const res = await fetch(`https://api.github.com/repos/${repo}/releases/latest`);
    if (!res.ok) throw new Error(`GitHub API failed for ${repo}: ${res.statusText}`);

    const data = await res.json();
    const asset = data.assets.find(a =>
        a.name.toLowerCase().includes("win") &&
        a.name.toLowerCase().endsWith(".zip") &&
        !a.name.toLowerCase().includes("arm")
    );

    if (!asset) throw new Error(`No Windows zip found for ${repo}`);

    await downloadAndExtract(asset.browser_download_url, destFolder, toolName);
}

async function runSetup() {
    console.log("=== Setting up Chess Engine Tournament Environment ===\n");
    await fs.mkdir(TOOLS_DIR, { recursive: true });
    await fs.mkdir(ENGINES_DIR, { recursive: true });

    if (await exists(BOOK_PATH)) {
        console.log("[Book] ✅ UHO book already exists. Skipping.\n");
    } else {
        console.log("[Book] Downloading UHO opening book...");
        const bookUrl = "https://github.com/official-stockfish/books/raw/master/UHO_4060_v1.epd.zip";
        await downloadAndExtract(bookUrl, TOOLS_DIR, "UHO_Book");
        console.log(`[Book] ✅ Saved to: ${BOOK_PATH}\n`);
    }

    await downloadGithubEngine("cutechess/cutechess", path.join(TOOLS_DIR, "cutechess"), "Cutechess-CLI");

    const MADCHESS_URL = "https://github.com/jbtucheck/MadChess/releases/download/v3.2/MadChess-3.2.zip";
    await downloadAndExtract(MADCHESS_URL, path.join(ENGINES_DIR, "madchess"), "MadChess");
    await downloadGithubEngine("official-stockfish/Stockfish", path.join(ENGINES_DIR, "stockfish"), "Stockfish");

    await downloadGithubEngine("mclarkson/Stash", path.join(ENGINES_DIR, "stash"), "Stash");

    await downloadGithubEngine("jessev/Wasp", path.join(ENGINES_DIR, "wasp"), "Wasp");

    console.log("\n🚀 Setup Complete!");
    console.log("Your engines and opening book are ready.");
}

runSetup().catch(console.error);
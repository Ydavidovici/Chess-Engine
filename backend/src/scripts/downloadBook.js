import path from "node:path";
import { fileURLToPath } from "node:url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const targetPath = path.resolve(__dirname, "../../../engines/myengine/book.bin");

async function downloadBook() {
    const url = process.argv[2];
    
    if (!url) {
        console.error("❌ Error: No download URL provided.");
        console.error("\nThe default Titans URL is currently down or has moved.");
        console.error("Please provide a direct link to any Polyglot (.bin) chess opening book.");
        console.error("\nUsage: bun run download:book <URL>");
        console.error("Example: bun run download:book https://your-server.com/Titans.bin\n");
        process.exit(1);
    }
    
    console.log(`Downloading opening book from: ${url}`);
    console.log(`Target destination: ${targetPath}`);
    console.log(`Please wait, downloading may take a moment depending on file size...`);
    
    try {
        const response = await fetch(url);
        if (!response.ok) {
            throw new Error(`Failed to fetch: ${response.status} ${response.statusText}`);
        }
        
        // Bun handles streaming response to disk extremely efficiently
        const bytesWritten = await Bun.write(targetPath, response);
        console.log(`\n✅ Successfully downloaded and saved ${(bytesWritten / 1024 / 1024).toFixed(2)} MB!`);
    } catch (err) {
        console.error("\n❌ Error downloading book:", err.message);
        process.exit(1);
    }
}

downloadBook();

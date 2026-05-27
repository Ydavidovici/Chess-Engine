import { mock, describe, it, expect, beforeEach, afterEach } from "bun:test";
import { LichessBot, LichessRateLimited } from "../src/lichessBot.js";
import { nullNotifier } from "../src/notifier.js";

describe("LichessBot - Caching and Throttling", () => {
    let originalFetch;
    let bot;
    
    beforeEach(() => {
        originalFetch = global.fetch;
        global.fetch = mock();
        bot = new LichessBot("dummy-token", () => ({}), {
            notifier: nullNotifier,
            challengeSpacingMs: 50, // Keep short for fast tests
            huntAcceptTimeoutMs: 100,
        });
        
        // Mock Date.now for time manipulation
        const RealDateNow = Date.now;
        let currentTime = 1000000;
        global.Date.now = mock(() => currentTime);
        global.Date.now.advance = (ms) => { currentTime += ms; };
        global.Date.now.restore = () => { global.Date.now = RealDateNow; };
    });

    afterEach(() => {
        global.fetch = originalFetch;
        if (global.Date.now.restore) global.Date.now.restore();
    });

    it("should cache profile fetches for 60 seconds", async () => {
        global.fetch.mockResolvedValueOnce(new Response(JSON.stringify({ perfs: { blitz: { rating: 1500 } } }), { status: 200 }));
        
        const r1 = await bot._fetchMyRating("blitz");
        expect(r1.rating).toBe(1500);
        expect(global.fetch).toHaveBeenCalledTimes(1);

        // Fetching again immediately should use cache
        const r2 = await bot._fetchMyRating("blitz");
        expect(r2.rating).toBe(1500);
        expect(global.fetch).toHaveBeenCalledTimes(1); // Still 1

        // Advance 30 seconds, still cached
        global.Date.now.advance(30000);
        const r3 = await bot._fetchMyRating("blitz");
        expect(r3.rating).toBe(1500);
        expect(global.fetch).toHaveBeenCalledTimes(1);

        // Advance another 31 seconds, cache expires
        global.Date.now.advance(31000);
        global.fetch.mockResolvedValueOnce(new Response(JSON.stringify({ perfs: { blitz: { rating: 1550 } } }), { status: 200 }));
        
        const r4 = await bot._fetchMyRating("blitz");
        expect(r4.rating).toBe(1550);
        expect(global.fetch).toHaveBeenCalledTimes(2); // Now 2
    });

    it("should cache online bots for 30 seconds", async () => {
        const botsPayload = "{\"id\":\"bot1\",\"username\":\"Bot1\"}\n{\"id\":\"bot2\",\"username\":\"Bot2\"}\n";
        global.fetch.mockResolvedValueOnce(new Response(botsPayload, { status: 200 }));
        
        const b1 = await bot._fetchOnlineBots(500);
        expect(b1.length).toBe(2);
        expect(global.fetch).toHaveBeenCalledTimes(1);

        // Fetching again should use cache
        const b2 = await bot._fetchOnlineBots(200);
        expect(b2.length).toBe(2);
        expect(global.fetch).toHaveBeenCalledTimes(1);

        // Advance 15 seconds, still cached
        global.Date.now.advance(15000);
        await bot._fetchOnlineBots(500);
        expect(global.fetch).toHaveBeenCalledTimes(1);

        // Advance 16 seconds, cache expires
        global.Date.now.advance(16000);
        global.fetch.mockResolvedValueOnce(new Response("{\"id\":\"bot3\",\"username\":\"Bot3\"}\n", { status: 200 }));
        
        const b3 = await bot._fetchOnlineBots(500);
        expect(b3.length).toBe(1);
        expect(global.fetch).toHaveBeenCalledTimes(2);
    });

    it("should throttle global challenges", async () => {
        // Mock fetch to just return 200 OK so _lichessFetch succeeds
        global.fetch.mockResolvedValue(new Response("{}", { status: 200 }));
        
        // No wait on first call
        await bot._throttleGlobalChallenge();
        expect(bot.lastChallengeTime).toBe(global.Date.now());
        
        // Second call right after should block for 50ms (mocked challengeSpacingMs)
        let resolved = false;
        const p = bot._throttleGlobalChallenge().then(() => { resolved = true; });
        
        // Advance time a little but not enough
        global.Date.now.advance(20);
        await new Promise(r => setTimeout(r, 10)); // Yield to event loop
        expect(resolved).toBe(false);
        
        // It shouldn't resolve until the timeout finishes, but since our tests
        // mock Date.now but NOT setTimeout, the actual Promise resolves after
        // the real setTimeout(..., 30) finishes.
        await p;
        expect(resolved).toBe(true);
    });
});

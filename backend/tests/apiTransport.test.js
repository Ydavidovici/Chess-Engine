import {expect, test, mock, describe, beforeEach, afterEach} from "bun:test";
import {ApiTransport} from "../src/apiTransport.js";

describe("ApiTransport", () => {
    let originalFetch;

    beforeEach(() => {
        originalFetch = global.fetch;
    });

    afterEach(() => {
        global.fetch = originalFetch;
    });

    test("formats regular logs correctly and sends to logs channel", async () => {
        const fetchMock = mock(async () => ({ok: true}));
        global.fetch = fetchMock;

        const transport = new ApiTransport({url: "http://test/notify", token: "test_token"});
        await transport.send({
            level: "info",
            subject: "[Server] Started",
            details: {port: 8000}
        });

        expect(fetchMock).toHaveBeenCalled();
        const callArgs = fetchMock.mock.calls[0];
        expect(callArgs[0]).toBe("http://test/notify");
        expect(callArgs[1].headers.Authorization).toBe("Bearer test_token");
        
        const body = JSON.parse(callArgs[1].body);
        expect(body.channel).toBe("logs");
        expect(body.message).toBe(`\`\`\`\n[ INFO ] [Server] Started\n{\n  "port": 8000\n}\n\`\`\``);
        expect(body.status).toBeUndefined();
    });

    test("routes autoplay restart events to notifications channel with success status", async () => {
        const fetchMock = mock(async () => ({ok: true}));
        global.fetch = fetchMock;

        const transport = new ApiTransport({url: "http://test/notify", token: "test_token"});
        await transport.send({
            level: "info",
            subject: "[Autoplay] Autoplay restarted after rate limit",
        });

        const callArgs = fetchMock.mock.calls[0];
        const body = JSON.parse(callArgs[1].body);
        expect(body.channel).toBe("notifications");
        expect(body.status).toBe("success");
        expect(body.message).toBe("\`\`\`\n[ INFO ] [Autoplay] Autoplay restarted after rate limit\n\`\`\`");
    });

    test("routes rate limit events to notifications channel", async () => {
        const fetchMock = mock(async () => ({ok: true}));
        global.fetch = fetchMock;

        const transport = new ApiTransport({url: "http://test/notify", token: "test_token"});
        await transport.send({
            level: "warn",
            subject: "[Autoplay] Rate-limited; skipping tick",
        });

        const callArgs = fetchMock.mock.calls[0];
        const body = JSON.parse(callArgs[1].body);
        expect(body.channel).toBe("notifications");
        expect(body.status).toBe("success");
    });

    test("sets error status for fatal and error levels", async () => {
        const fetchMock = mock(async () => ({ok: true}));
        global.fetch = fetchMock;

        const transport = new ApiTransport({url: "http://test/notify", token: "test_token"});
        await transport.send({
            level: "error",
            subject: "[EngineManager] Engine crashed",
        });

        const callArgs = fetchMock.mock.calls[0];
        const body = JSON.parse(callArgs[1].body);
        expect(body.channel).toBe("logs");
        expect(body.status).toBe("error");
    });

    test("handles empty details", async () => {
        const fetchMock = mock(async () => ({ok: true}));
        global.fetch = fetchMock;

        const transport = new ApiTransport({url: "http://test/notify", token: "test_token"});
        await transport.send({
            level: "warn",
            subject: "Something happened",
        });

        const callArgs = fetchMock.mock.calls[0];
        const body = JSON.parse(callArgs[1].body);
        expect(body.message).toBe("\`\`\`\n[ WARN ] Something happened\n\`\`\`");
    });
});

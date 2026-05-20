import {EventEmitter} from "node:events";

export const LEVELS = Object.freeze({
    INFO: "info",
    WARN: "warn",
    ERROR: "error",
    FATAL: "fatal",
});

const LEVEL_RANK = {info: 10, warn: 20, error: 30, fatal: 40};

export class Notifier extends EventEmitter {
    constructor({minLevel = LEVELS.INFO, transports = []} = {}) {
        super();
        this.minLevel = minLevel;
        this.transports = [];
        for (const t of transports) this.addTransport(t);
    }

    addTransport(transport) {
        if (!transport || typeof transport.send !== "function") {
            throw new Error("Transport must implement send(event)");
        }
        this.transports.push(transport);
    }

    setMinLevel(level) {
        if (!(level in LEVEL_RANK)) throw new Error(`Unknown level: ${level}`);
        this.minLevel = level;
    }

    info(subject, details)  { return this.notify(LEVELS.INFO,  subject, details); }
    warn(subject, details)  { return this.notify(LEVELS.WARN,  subject, details); }
    error(subject, details) { return this.notify(LEVELS.ERROR, subject, details); }
    fatal(subject, details) { return this.notify(LEVELS.FATAL, subject, details); }

    notify(level, subject, details) {
        if (LEVEL_RANK[level] < LEVEL_RANK[this.minLevel]) return Promise.resolve();

        const event = {
            level,
            subject: String(subject ?? ""),
            details: details ?? null,
            timestamp: new Date().toISOString(),
        };

        this.emit("notify", event);

        // Send to all transports in parallel; never let one failure block the others.
        return Promise.allSettled(this.transports.map(t => {
            try {
                return Promise.resolve(t.send(event));
            } catch (err) {
                return Promise.reject(err);
            }
        })).then(results => {
            for (const r of results) {
                if (r.status === "rejected") {
                    console.error("[Notifier] Transport failed:", r.reason);
                }
            }
        });
    }

    // Best-effort flush: lets transports drain any buffered work before exit.
    async flush(timeoutMs = 3000) {
        const flushes = this.transports
            .filter(t => typeof t.flush === "function")
            .map(t => Promise.resolve(t.flush()).catch(err => {
                console.error("[Notifier] Flush failed:", err);
            }));

        const timeout = new Promise(r => setTimeout(r, timeoutMs));
        await Promise.race([Promise.all(flushes), timeout]);
    }
}

export class ConsoleTransport {
    constructor({stream = console, prefix = "[Notify]"} = {}) {
        this.stream = stream;
        this.prefix = prefix;
    }

    send(event) {
        const line = `${this.prefix} [${event.level.toUpperCase()}] ${event.subject}`;
        const fn = event.level === "error" || event.level === "fatal"
            ? this.stream.error.bind(this.stream)
            : event.level === "warn"
                ? this.stream.warn.bind(this.stream)
                : this.stream.log.bind(this.stream);

        if (event.details != null) fn(line, event.details);
        else fn(line);
    }
}

// Drop-in no-op notifier so callers don't have to null-check.
export const nullNotifier = new Notifier({transports: []});

export class ApiTransport {
    constructor({url, token, project = ""} = {}) {
        this.url = url || process.env.API_NOTIFY_URL || "http://localhost:3535/notify";
        this.token = token || process.env.API_NOTIFY_TOKEN || "jewkie_chess_9a7b4c2d1e3f8a0b";
        this.project = project;
    }

    async send(event) {
        let channel = "logs";
        let status = undefined;

        const subjectLower = event.subject.toLowerCase();
        
        // Route specific events to notifications channel
        if (
            subjectLower.includes("autoplay restart") || 
            subjectLower.includes("autoplay enabled") ||
            subjectLower.includes("rate limit") ||
            subjectLower.includes("rate-limited")
        ) {
            channel = "notifications";
        }

        if (event.level === "error" || event.level === "fatal") {
            status = "error";
        } else if (channel === "notifications") {
            status = "success";
        }

        const levelStr = event.level.toUpperCase();
        let bodyText = `[ ${levelStr} ] ${event.subject}`;
        if (event.details && Object.keys(event.details).length > 0) {
            bodyText += `\n${JSON.stringify(event.details, null, 2)}`;
        }
        const message = `\`\`\`\n${bodyText}\n\`\`\``;

        const payload = {
            message,
            channel,
            project: this.project
        };
        if (status) {
            payload.status = status;
        }

        try {
            const res = await fetch(this.url, {
                method: "POST",
                headers: {
                    "Content-Type": "application/json",
                    "Authorization": `Bearer ${this.token}`
                },
                body: JSON.stringify(payload)
            });
            if (!res.ok) {
                // Log to console rather than Notifier to avoid feedback loops
                console.error(`[ApiTransport] HTTP ${res.status}: ${await res.text().catch(() => "")}`);
            }
        } catch (err) {
            console.error("[ApiTransport] Request failed:", err.message);
        }
    }
}

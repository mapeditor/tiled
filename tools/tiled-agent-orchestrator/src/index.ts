import path from "node:path";

import { createAgentServer } from "./http-server.js";
import { ProcessBridgeClient } from "./process-bridge-client.js";
import { SessionManager } from "./session-manager.js";

const repoRoot = path.resolve(import.meta.dirname, "../../..");
const bridgeBinaryPath =
  process.env.TILED_AGENT_BRIDGE_BINARY ??
  path.join(repoRoot, "build", "tiledagentbridge", "tiledagentbridge");
const port = Number(process.env.TILED_AGENT_PORT ?? "3017");

const bridgeClient = new ProcessBridgeClient({
  binaryPath: bridgeBinaryPath,
});
const sessionManager = new SessionManager(bridgeClient);
const server = await createAgentServer({
  sessionManager,
  port,
});

const shutdown = async () => {
  await server.close();
  await bridgeClient.dispose();
};

process.on("SIGINT", async () => {
  await shutdown();
  process.exit(0);
});

process.on("SIGTERM", async () => {
  await shutdown();
  process.exit(0);
});

console.error(`Tiled Agent orchestrator listening on ${server.baseUrl}`);

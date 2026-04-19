import path from "node:path";

import { McpServer } from "@modelcontextprotocol/sdk/server/mcp.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import * as z from "zod/v4";

import type { BridgeCommandRequest } from "./contracts.js";
import { ProcessBridgeClient } from "./process-bridge-client.js";
import { SessionManager } from "./session-manager.js";

const repoRoot = path.resolve(import.meta.dirname, "../../..");
const bridgeBinaryPath =
  process.env.TILED_AGENT_BRIDGE_BINARY ??
  path.join(repoRoot, "build", "tiledagentbridge", "tiledagentbridge");

const bridgeClient = new ProcessBridgeClient({
  binaryPath: bridgeBinaryPath,
});
const sessionManager = new SessionManager(bridgeClient);

const toStructuredContent = (value: unknown): Record<string, unknown> =>
  JSON.parse(JSON.stringify(value)) as Record<string, unknown>;

const server = new McpServer({
  name: "tiled-agent",
  version: "0.1.0",
});

server.registerTool(
  "tiled_open_document",
  {
    description: "Create a workspace/session and open a Tiled map or tileset document.",
    inputSchema: {
      rootPath: z.string(),
      documentPath: z.string(),
    },
  },
  async ({ rootPath, documentPath }) => {
    const workspace = sessionManager.createWorkspace(rootPath);
    const session = sessionManager.createSession(workspace.id);
    const opened = await sessionManager.openDocument(session.id, documentPath);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(
            {
              workspace,
              session,
              opened,
            },
            null,
            2,
          ),
        },
      ],
      structuredContent: {
        workspace,
        session,
        opened,
      } satisfies Record<string, unknown>,
    };
  },
);

server.registerTool(
  "tiled_get_snapshot",
  {
    description: "Fetch the current normalized snapshot for a session.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    const snapshot = await sessionManager.getSnapshot(sessionId);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(snapshot, null, 2),
        },
      ],
      structuredContent: toStructuredContent(snapshot),
    };
  },
);

server.registerTool(
  "tiled_execute_command",
  {
    description: "Execute a typed editing command against the current session.",
    inputSchema: {
      sessionId: z.string(),
      type: z.string(),
      payload: z.record(z.string(), z.any()),
    },
  },
  async ({ sessionId, type, payload }) => {
    const result = await sessionManager.executeCommand(sessionId, {
      type,
      payload,
    } as BridgeCommandRequest);

    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(result, null, 2),
        },
      ],
      structuredContent: toStructuredContent(result),
    };
  },
);

server.registerTool(
  "tiled_validate",
  {
    description: "Validate the currently open document and return diagnostics.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    const report = await sessionManager.validateDocument(sessionId);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(report, null, 2),
        },
      ],
      structuredContent: toStructuredContent(report),
    };
  },
);

server.registerTool(
  "tiled_save",
  {
    description: "Persist the current session document to disk using its original path.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    const saved = await sessionManager.saveDocument(sessionId);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(saved, null, 2),
        },
      ],
      structuredContent: toStructuredContent(saved),
    };
  },
);

server.registerTool(
  "tiled_undo",
  {
    description: "Undo the last bridge command in the session.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    const result = await sessionManager.undo(sessionId);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(result, null, 2),
        },
      ],
      structuredContent: toStructuredContent(result),
    };
  },
);

server.registerTool(
  "tiled_redo",
  {
    description: "Redo the last undone bridge command in the session.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    const result = await sessionManager.redo(sessionId);
    return {
      content: [
        {
          type: "text",
          text: JSON.stringify(result, null, 2),
        },
      ],
      structuredContent: toStructuredContent(result),
    };
  },
);

server.registerTool(
  "tiled_close_session",
  {
    description: "Close a bridge-backed editing session.",
    inputSchema: {
      sessionId: z.string(),
    },
  },
  async ({ sessionId }) => {
    await sessionManager.closeSession(sessionId);
    return {
      content: [
        {
          type: "text",
          text: `Closed session ${sessionId}`,
        },
      ],
      structuredContent: {
        closed: true,
        sessionId,
      },
    };
  },
);

const transport = new StdioServerTransport();
await server.connect(transport);

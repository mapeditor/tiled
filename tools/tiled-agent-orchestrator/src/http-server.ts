import { createServer, type IncomingMessage, type Server, type ServerResponse } from "node:http";
import { readFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

import {
  SessionManager,
  SessionNotFoundError,
  WorkspaceNotFoundError,
  WorkspacePathError,
} from "./session-manager.js";

interface CreateAgentServerOptions {
  sessionManager: SessionManager;
  port: number;
}

interface StartedAgentServer {
  baseUrl: string;
  close: () => Promise<void>;
}

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);
const debugPagePath = path.resolve(__dirname, "../public/index.html");

async function readJsonBody(request: IncomingMessage): Promise<unknown> {
  const chunks: Buffer[] = [];
  for await (const chunk of request) {
    chunks.push(Buffer.from(chunk));
  }

  if (chunks.length === 0) {
    return {};
  }

  return JSON.parse(Buffer.concat(chunks).toString("utf8"));
}

function sendJson(response: ServerResponse, statusCode: number, body: unknown): void {
  response.writeHead(statusCode, {
    "content-type": "application/json; charset=utf-8",
  });
  response.end(JSON.stringify(body));
}

async function sendDebugPage(response: ServerResponse): Promise<void> {
  const contents = await readFile(debugPagePath, "utf8");
  response.writeHead(200, {
    "content-type": "text/html; charset=utf-8",
  });
  response.end(contents);
}

function mapError(error: unknown): { statusCode: number; body: { error: { code: string; message: string } } } {
  if (error instanceof WorkspacePathError) {
    return {
      statusCode: 400,
      body: {
        error: {
          code: "WORKSPACE_PATH_ERROR",
          message: error.message,
        },
      },
    };
  }

  if (error instanceof WorkspaceNotFoundError || error instanceof SessionNotFoundError) {
    return {
      statusCode: 404,
      body: {
        error: {
          code: error.name,
          message: error.message,
        },
      },
    };
  }

  return {
    statusCode: 500,
    body: {
      error: {
        code: "INTERNAL_ERROR",
        message: error instanceof Error ? error.message : "Unknown error",
      },
    },
  };
}

async function routeRequest(
  request: IncomingMessage,
  response: ServerResponse,
  sessionManager: SessionManager,
): Promise<void> {
  const requestUrl = new URL(request.url ?? "/", "http://localhost");
  const pathname = requestUrl.pathname;

  if (request.method === "GET" && pathname === "/") {
    await sendDebugPage(response);
    return;
  }

  if (request.method === "POST" && pathname === "/workspaces") {
    const payload = (await readJsonBody(request)) as { rootPath: string };
    const workspace = sessionManager.createWorkspace(payload.rootPath);
    sendJson(response, 201, { workspace });
    return;
  }

  if (request.method === "POST" && pathname === "/sessions") {
    const payload = (await readJsonBody(request)) as { workspaceId: string };
    const session = sessionManager.createSession(payload.workspaceId);
    sendJson(response, 201, { session });
    return;
  }

  const openMatch = pathname.match(/^\/sessions\/([^/]+)\/documents\/open$/);
  if (request.method === "POST" && openMatch) {
    const payload = (await readJsonBody(request)) as { documentPath: string };
    const result = await sessionManager.openDocument(openMatch[1], payload.documentPath);
    sendJson(response, 200, result);
    return;
  }

  const snapshotMatch = pathname.match(/^\/sessions\/([^/]+)\/snapshot$/);
  if (request.method === "GET" && snapshotMatch) {
    const snapshot = await sessionManager.getSnapshot(snapshotMatch[1]);
    sendJson(response, 200, { snapshot });
    return;
  }

  const commandMatch = pathname.match(/^\/sessions\/([^/]+)\/commands$/);
  if (request.method === "POST" && commandMatch) {
    const payload = await readJsonBody(request);
    const result = await sessionManager.executeCommand(commandMatch[1], payload as any);
    sendJson(response, 200, result);
    return;
  }

  const validateMatch = pathname.match(/^\/sessions\/([^/]+)\/validate$/);
  if (request.method === "POST" && validateMatch) {
    const report = await sessionManager.validateDocument(validateMatch[1]);
    sendJson(response, 200, report);
    return;
  }

  const saveMatch = pathname.match(/^\/sessions\/([^/]+)\/save$/);
  if (request.method === "POST" && saveMatch) {
    const result = await sessionManager.saveDocument(saveMatch[1]);
    sendJson(response, 200, result);
    return;
  }

  const undoMatch = pathname.match(/^\/sessions\/([^/]+)\/undo$/);
  if (request.method === "POST" && undoMatch) {
    const result = await sessionManager.undo(undoMatch[1]);
    sendJson(response, 200, result);
    return;
  }

  const redoMatch = pathname.match(/^\/sessions\/([^/]+)\/redo$/);
  if (request.method === "POST" && redoMatch) {
    const result = await sessionManager.redo(redoMatch[1]);
    sendJson(response, 200, result);
    return;
  }

  sendJson(response, 404, {
    error: {
      code: "NOT_FOUND",
      message: `Unknown route: ${request.method} ${pathname}`,
    },
  });
}

export async function createAgentServer(options: CreateAgentServerOptions): Promise<StartedAgentServer> {
  const server = createServer(async (request, response) => {
    try {
      await routeRequest(request, response, options.sessionManager);
    } catch (error) {
      const mapped = mapError(error);
      sendJson(response, mapped.statusCode, mapped.body);
    }
  });

  await new Promise<void>((resolve, reject) => {
    server.once("error", reject);
    server.listen(options.port, "127.0.0.1", () => {
      server.off("error", reject);
      resolve();
    });
  });

  const address = server.address();
  if (!address || typeof address === "string") {
    throw new Error("Unable to determine listening address");
  }

  return {
    baseUrl: `http://127.0.0.1:${address.port}`,
    close: async () =>
      await new Promise<void>((resolve, reject) => {
        server.close((error?: Error) => {
          if (error) {
            reject(error);
            return;
          }
          resolve();
        });
      }),
  };
}

export type { Server };

import { afterEach, describe, expect, it } from "vitest";

import type { BridgeClient, DocumentSnapshot, OpenDocumentResult, ValidationReport } from "../src/contracts.js";
import { createAgentServer } from "../src/http-server.js";
import { SessionManager } from "../src/session-manager.js";

class StubBridgeClient implements BridgeClient {
  async openDocument(_sessionId: string, _documentPath: string): Promise<OpenDocumentResult> {
    return {
      documentType: "map",
      revision: 0,
      snapshot: {
        documentType: "map",
        map: {
          width: 4,
          height: 4,
          tileWidth: 16,
          tileHeight: 16,
          orientation: "orthogonal",
          infinite: false,
          tilesets: [],
          layers: [],
        },
      },
    };
  }

  async getSnapshot(_sessionId: string): Promise<DocumentSnapshot> {
    return {
      documentType: "map",
      map: {
        width: 4,
        height: 4,
        tileWidth: 16,
        tileHeight: 16,
        orientation: "orthogonal",
        infinite: false,
        tilesets: [],
        layers: [],
      },
    };
  }

  async executeCommand(): Promise<any> {
    return {
      revisionBefore: 0,
      revisionAfter: 1,
      changedEntities: [],
      warnings: [],
      errors: [],
      undoDepth: 1,
      redoDepth: 0,
    };
  }

  async validateDocument(): Promise<ValidationReport> {
    return {
      revision: 0,
      diagnostics: [],
    };
  }

  async saveDocument(): Promise<{ path: string; revision: number }> {
    return {
      path: "tests/data/mapobject.tmx",
      revision: 0,
    };
  }

  async undo(): Promise<any> {
    return {
      revisionBefore: 1,
      revisionAfter: 0,
      changedEntities: [],
      warnings: [],
      errors: [],
      undoDepth: 0,
      redoDepth: 1,
    };
  }

  async redo(): Promise<any> {
    return {
      revisionBefore: 0,
      revisionAfter: 1,
      changedEntities: [],
      warnings: [],
      errors: [],
      undoDepth: 1,
      redoDepth: 0,
    };
  }

  async closeSession(): Promise<void> {}
}

const servers: Array<{ close: () => Promise<void> }> = [];

afterEach(async () => {
  while (servers.length > 0) {
    await servers.pop()!.close();
  }
});

describe("HTTP server", () => {
  it("세션 생성, 문서 열기, 스냅샷 조회를 노출한다", async () => {
    const manager = new SessionManager(new StubBridgeClient());
    const server = await createAgentServer({
      sessionManager: manager,
      port: 0,
    });
    servers.push(server);

    const workspaceResponse = await fetch(`${server.baseUrl}/workspaces`, {
      method: "POST",
      headers: {
        "content-type": "application/json",
      },
      body: JSON.stringify({
        rootPath: process.cwd(),
      }),
    });
    expect(workspaceResponse.status).toBe(201);
    const workspace = await workspaceResponse.json();

    const sessionResponse = await fetch(`${server.baseUrl}/sessions`, {
      method: "POST",
      headers: {
        "content-type": "application/json",
      },
      body: JSON.stringify({
        workspaceId: workspace.workspace.id,
      }),
    });
    expect(sessionResponse.status).toBe(201);
    const session = await sessionResponse.json();

    const openResponse = await fetch(`${server.baseUrl}/sessions/${session.session.id}/documents/open`, {
      method: "POST",
      headers: {
        "content-type": "application/json",
      },
      body: JSON.stringify({
        documentPath: "tests/data/mapobject.tmx",
      }),
    });
    expect(openResponse.status).toBe(200);

    const snapshotResponse = await fetch(`${server.baseUrl}/sessions/${session.session.id}/snapshot`);
    expect(snapshotResponse.status).toBe(200);

    const snapshot = await snapshotResponse.json();
    expect(snapshot.snapshot.documentType).toBe("map");
    expect(snapshot.snapshot.map.width).toBe(4);
  });

  it("디버그 UI 페이지를 제공한다", async () => {
    const manager = new SessionManager(new StubBridgeClient());
    const server = await createAgentServer({
      sessionManager: manager,
      port: 0,
    });
    servers.push(server);

    const response = await fetch(server.baseUrl);
    expect(response.status).toBe(200);
    expect(await response.text()).toContain("Tiled Agent Debug");
  });
});

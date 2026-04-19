import { describe, expect, it } from "vitest";

import type {
  BridgeClient,
  BridgeCommandRequest,
  BridgeCommandResult,
  DocumentSnapshot,
  OpenDocumentResult,
  ValidationReport,
} from "../src/contracts.js";
import {
  SessionManager,
  SessionNotFoundError,
  WorkspacePathError,
} from "../src/session-manager.js";

class FakeBridgeClient implements BridgeClient {
  public openedPaths: string[] = [];
  public executedCommands: BridgeCommandRequest[] = [];

  async openDocument(sessionId: string, documentPath: string): Promise<OpenDocumentResult> {
    this.openedPaths.push(`${sessionId}:${documentPath}`);
    return {
      documentType: "map",
      revision: 0,
      snapshot: {
        documentType: "map",
        map: {
          width: 8,
          height: 8,
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
        width: 8,
        height: 8,
        tileWidth: 16,
        tileHeight: 16,
        orientation: "orthogonal",
        infinite: false,
        tilesets: [],
        layers: [],
      },
    };
  }

  async executeCommand(_sessionId: string, command: BridgeCommandRequest): Promise<BridgeCommandResult> {
    this.executedCommands.push(command);
    return {
      revisionBefore: 0,
      revisionAfter: 1,
      changedEntities: [
        {
          entityType: "layer",
          entityId: 1,
          changeType: "created",
        },
      ],
      warnings: [],
      errors: [],
      undoDepth: 1,
      redoDepth: 0,
    };
  }

  async validateDocument(_sessionId: string): Promise<ValidationReport> {
    return {
      revision: 1,
      diagnostics: [],
    };
  }

  async saveDocument(_sessionId: string): Promise<{ path: string; revision: number }> {
    return {
      path: "/tmp/test.tmx",
      revision: 1,
    };
  }

  async undo(_sessionId: string): Promise<BridgeCommandResult> {
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

  async redo(_sessionId: string): Promise<BridgeCommandResult> {
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

  async closeSession(_sessionId: string): Promise<void> {}
}

describe("SessionManager", () => {
  it("워크스페이스와 세션을 만들고 문서를 연다", async () => {
    const manager = new SessionManager(new FakeBridgeClient());

    const workspace = manager.createWorkspace(process.cwd());
    const session = manager.createSession(workspace.id);
    const result = await manager.openDocument(session.id, "tests/data/mapobject.tmx");

    expect(result.documentType).toBe("map");
    expect(result.revision).toBe(0);

    const snapshot = await manager.getSnapshot(session.id);
    expect(snapshot.documentType).toBe("map");
    if (snapshot.documentType !== "map") {
      throw new Error("expected map snapshot");
    }
    expect(snapshot.map.layers).toEqual([]);
  });

  it("워크스페이스 루트 밖 경로를 거부한다", async () => {
    const manager = new SessionManager(new FakeBridgeClient());
    const workspace = manager.createWorkspace(process.cwd());
    const session = manager.createSession(workspace.id);

    await expect(manager.openDocument(session.id, "../outside.tmx")).rejects.toBeInstanceOf(
      WorkspacePathError,
    );
  });

  it("세션이 없으면 명령 실행을 거부한다", async () => {
    const manager = new SessionManager(new FakeBridgeClient());

    await expect(
      manager.executeCommand("missing", {
        type: "create_layer",
        payload: {
          layerType: "tile",
          name: "Ground",
        },
      }),
    ).rejects.toBeInstanceOf(SessionNotFoundError);
  });
});

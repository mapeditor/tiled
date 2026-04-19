import path from "node:path";
import { randomUUID } from "node:crypto";

import type {
  BridgeClient,
  BridgeCommandRequest,
  BridgeCommandResult,
  DocumentSnapshot,
  OpenDocumentResult,
  ValidationReport,
} from "./contracts.js";

export class SessionNotFoundError extends Error {
  constructor(sessionId: string) {
    super(`Unknown session: ${sessionId}`);
    this.name = "SessionNotFoundError";
  }
}

export class WorkspaceNotFoundError extends Error {
  constructor(workspaceId: string) {
    super(`Unknown workspace: ${workspaceId}`);
    this.name = "WorkspaceNotFoundError";
  }
}

export class WorkspacePathError extends Error {
  constructor(documentPath: string) {
    super(`Path escapes workspace root: ${documentPath}`);
    this.name = "WorkspacePathError";
  }
}

export interface WorkspaceRecord {
  id: string;
  rootPath: string;
  createdAt: string;
}

export interface SessionRecord {
  id: string;
  workspaceId: string;
  createdAt: string;
  documentPath?: string;
  revision?: number;
}

interface SessionState {
  record: SessionRecord;
}

export class SessionManager {
  private readonly workspaces = new Map<string, WorkspaceRecord>();
  private readonly sessions = new Map<string, SessionState>();

  constructor(private readonly bridgeClient: BridgeClient) {}

  createWorkspace(rootPath: string): WorkspaceRecord {
    const resolvedRootPath = path.resolve(rootPath);
    const workspace: WorkspaceRecord = {
      id: randomUUID(),
      rootPath: resolvedRootPath,
      createdAt: new Date().toISOString(),
    };

    this.workspaces.set(workspace.id, workspace);
    return workspace;
  }

  createSession(workspaceId: string): SessionRecord {
    this.getWorkspace(workspaceId);

    const record: SessionRecord = {
      id: randomUUID(),
      workspaceId,
      createdAt: new Date().toISOString(),
    };

    this.sessions.set(record.id, { record });
    return record;
  }

  getWorkspace(workspaceId: string): WorkspaceRecord {
    const workspace = this.workspaces.get(workspaceId);
    if (!workspace) {
      throw new WorkspaceNotFoundError(workspaceId);
    }
    return workspace;
  }

  getSession(sessionId: string): SessionRecord {
    return this.getSessionState(sessionId).record;
  }

  async openDocument(sessionId: string, documentPath: string): Promise<OpenDocumentResult> {
    const sessionState = this.getSessionState(sessionId);
    const workspace = this.getWorkspace(sessionState.record.workspaceId);
    const resolvedPath = this.resolveWithinWorkspace(workspace.rootPath, documentPath);

    const result = await this.bridgeClient.openDocument(sessionId, resolvedPath);
    sessionState.record.documentPath = resolvedPath;
    sessionState.record.revision = result.revision;
    return result;
  }

  async getSnapshot(sessionId: string): Promise<DocumentSnapshot> {
    this.getSessionState(sessionId);
    return this.bridgeClient.getSnapshot(sessionId);
  }

  async executeCommand(sessionId: string, command: BridgeCommandRequest): Promise<BridgeCommandResult> {
    const sessionState = this.getSessionState(sessionId);
    const result = await this.bridgeClient.executeCommand(sessionId, command);
    sessionState.record.revision = result.revisionAfter;
    return result;
  }

  async validateDocument(sessionId: string): Promise<ValidationReport> {
    this.getSessionState(sessionId);
    return this.bridgeClient.validateDocument(sessionId);
  }

  async saveDocument(sessionId: string): Promise<{ path: string; revision: number }> {
    const sessionState = this.getSessionState(sessionId);
    const result = await this.bridgeClient.saveDocument(sessionId);
    sessionState.record.revision = result.revision;
    return result;
  }

  async undo(sessionId: string): Promise<BridgeCommandResult> {
    const sessionState = this.getSessionState(sessionId);
    const result = await this.bridgeClient.undo(sessionId);
    sessionState.record.revision = result.revisionAfter;
    return result;
  }

  async redo(sessionId: string): Promise<BridgeCommandResult> {
    const sessionState = this.getSessionState(sessionId);
    const result = await this.bridgeClient.redo(sessionId);
    sessionState.record.revision = result.revisionAfter;
    return result;
  }

  async closeSession(sessionId: string): Promise<void> {
    this.getSessionState(sessionId);
    await this.bridgeClient.closeSession(sessionId);
    this.sessions.delete(sessionId);
  }

  private getSessionState(sessionId: string): SessionState {
    const session = this.sessions.get(sessionId);
    if (!session) {
      throw new SessionNotFoundError(sessionId);
    }
    return session;
  }

  private resolveWithinWorkspace(rootPath: string, documentPath: string): string {
    const resolvedPath = path.resolve(rootPath, documentPath);
    const relativePath = path.relative(rootPath, resolvedPath);
    if (relativePath.startsWith("..") || path.isAbsolute(relativePath)) {
      throw new WorkspacePathError(documentPath);
    }
    return resolvedPath;
  }
}

import { spawn, type ChildProcess } from "node:child_process";
import readline from "node:readline";

import type {
  BridgeClient,
  BridgeCommandRequest,
  BridgeCommandResult,
  DocumentSnapshot,
  OpenDocumentResult,
  ValidationReport,
} from "./contracts.js";

interface ProcessBridgeClientOptions {
  binaryPath: string;
  args?: string[];
  cwd?: string;
}

interface JsonRpcError {
  code: string;
  message: string;
}

interface PendingRequest {
  resolve: (value: any) => void;
  reject: (reason?: unknown) => void;
}

export class ProcessBridgeClient implements BridgeClient {
  private readonly pendingRequests = new Map<number, PendingRequest>();
  private nextId = 1;
  private child?: ChildProcess;
  private stdoutReader?: readline.Interface;

  constructor(private readonly options: ProcessBridgeClientOptions) {
    this.ensureStarted();
  }

  async openDocument(sessionId: string, documentPath: string): Promise<OpenDocumentResult> {
    return await this.request("openDocument", { sessionId, documentPath });
  }

  async getSnapshot(sessionId: string): Promise<DocumentSnapshot> {
    const result = await this.request<{
      snapshot: DocumentSnapshot;
    }>("getSnapshot", { sessionId });
    return result.snapshot;
  }

  async executeCommand(sessionId: string, command: BridgeCommandRequest): Promise<BridgeCommandResult> {
    return await this.request("executeCommand", { sessionId, command });
  }

  async validateDocument(sessionId: string): Promise<ValidationReport> {
    return await this.request("validateDocument", { sessionId });
  }

  async saveDocument(sessionId: string): Promise<{ path: string; revision: number }> {
    return await this.request("saveDocument", { sessionId });
  }

  async undo(sessionId: string): Promise<BridgeCommandResult> {
    return await this.request("undo", { sessionId });
  }

  async redo(sessionId: string): Promise<BridgeCommandResult> {
    return await this.request("redo", { sessionId });
  }

  async closeSession(sessionId: string): Promise<void> {
    await this.request("closeSession", { sessionId });
  }

  async dispose(): Promise<void> {
    if (!this.child) {
      return;
    }

    this.stdoutReader?.close();
    this.stdoutReader = undefined;

    const child = this.child;
    this.child = undefined;

    await new Promise<void>((resolve) => {
      child.once("exit", () => resolve());
      child.kill();
    });
  }

  private ensureStarted(): void {
    if (this.child) {
      return;
    }

    const child = spawn(this.options.binaryPath, this.options.args ?? [], {
      cwd: this.options.cwd,
      stdio: ["pipe", "pipe", "inherit"],
    });
    this.child = child;

    child.on("exit", (code, signal) => {
      const message = signal
        ? `Bridge exited with signal ${signal}`
        : `Bridge exited with code ${code ?? 0}`;

      for (const pending of this.pendingRequests.values()) {
        pending.reject(new Error(message));
      }
      this.pendingRequests.clear();
      this.child = undefined;
      this.stdoutReader = undefined;
    });

    if (!child.stdout) {
      throw new Error("Bridge stdout is not available");
    }

    this.stdoutReader = readline.createInterface({
      input: child.stdout,
    });

    this.stdoutReader.on("line", (line) => {
      const parsed = JSON.parse(line) as {
        id: number;
        result?: unknown;
        error?: JsonRpcError;
      };

      const pending = this.pendingRequests.get(parsed.id);
      if (!pending) {
        return;
      }

      this.pendingRequests.delete(parsed.id);
      if (parsed.error) {
        pending.reject(new Error(`${parsed.error.code}: ${parsed.error.message}`));
        return;
      }

      pending.resolve(parsed.result);
    });
  }

  private async request<TResult>(method: string, params: Record<string, unknown>): Promise<TResult> {
    this.ensureStarted();
    if (!this.child) {
      throw new Error("Bridge process is not available");
    }
    const stdin = this.child.stdin;
    if (!stdin) {
      throw new Error("Bridge stdin is not available");
    }

    const id = this.nextId++;
    const payload = JSON.stringify({
      jsonrpc: "2.0",
      id,
      method,
      params,
    });

    const result = await new Promise<TResult>((resolve, reject) => {
      this.pendingRequests.set(id, { resolve, reject });
      stdin.write(`${payload}\n`, "utf8", (error) => {
        if (error) {
          this.pendingRequests.delete(id);
          reject(error);
        }
      });
    });

    return result;
  }
}

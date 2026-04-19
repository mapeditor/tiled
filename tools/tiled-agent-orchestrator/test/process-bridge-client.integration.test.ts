import { afterAll, beforeAll, describe, expect, it } from "vitest";
import { existsSync } from "node:fs";
import path from "node:path";
import { execFileSync } from "node:child_process";

import { ProcessBridgeClient } from "../src/process-bridge-client.js";

const repoRoot = path.resolve(import.meta.dirname, "../../..");
const bridgeBuildDir = path.join(repoRoot, "build", "tiledagentbridge");
const bridgeBinary = path.join(bridgeBuildDir, "tiledagentbridge");

describe("ProcessBridgeClient", () => {
  let bridgeClient: ProcessBridgeClient | undefined;

  beforeAll(() => {
    if (!existsSync(bridgeBinary)) {
      execFileSync("cmake", ["--build", bridgeBuildDir, "--target", "tiledagentbridge", "-j4"], {
        cwd: repoRoot,
        stdio: "inherit",
      });
    }

    bridgeClient = new ProcessBridgeClient({
      binaryPath: bridgeBinary,
    });
  });

  afterAll(async () => {
    await bridgeClient?.dispose();
  });

  it("실제 bridge 프로세스와 문서 열기/명령 실행/undo를 수행한다", async () => {
    const sessionId = "integration-session";
    const documentPath = path.join(repoRoot, "tests", "automapping", "simple-2x2-rule", "map.tmx");

    const opened = await bridgeClient!.openDocument(sessionId, documentPath);
    expect(opened.documentType).toBe("map");
    if (opened.snapshot.documentType !== "map") {
      throw new Error("expected map snapshot");
    }
    expect(opened.snapshot.map.layers).toHaveLength(1);

    const command = await bridgeClient!.executeCommand(sessionId, {
      type: "create_layer",
      payload: {
        layerType: "object",
        name: "Objects",
      },
    });

    expect(command.revisionAfter).toBe(1);
    expect(command.changedEntities[0]?.entityType).toBe("layer");

    const undone = await bridgeClient!.undo(sessionId);
    expect(undone.revisionAfter).toBe(0);
  });
});

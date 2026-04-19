export type DocumentType = "map" | "tileset";

export interface ChangedEntity {
  entityType: "layer" | "object" | "map" | "tileset" | "property";
  entityId: string | number;
  changeType: "created" | "updated" | "deleted" | "moved";
}

export interface TileCellSummary {
  x: number;
  y: number;
  gid: number;
}

export interface LayerSummary {
  id: number;
  name: string;
  type: "tilelayer" | "objectgroup" | "imagelayer" | "grouplayer";
  visible: boolean;
  opacity: number;
  x: number;
  y: number;
  properties: Record<string, unknown>;
  cells?: TileCellSummary[];
  objects?: ObjectSummary[];
}

export interface ObjectSummary {
  id: number;
  name: string;
  className: string;
  x: number;
  y: number;
  width: number;
  height: number;
  rotation: number;
  visible: boolean;
  shape: string;
  properties: Record<string, unknown>;
}

export interface TilesetSummary {
  firstGid?: number;
  name: string;
  tileWidth: number;
  tileHeight: number;
  tileCount: number;
  columns: number;
  source?: string;
  image?: string;
}

export interface MapSnapshot {
  width: number;
  height: number;
  tileWidth: number;
  tileHeight: number;
  orientation: string;
  infinite: boolean;
  tilesets: TilesetSummary[];
  layers: LayerSummary[];
}

export interface StandaloneTilesetSnapshot {
  name: string;
  tileWidth: number;
  tileHeight: number;
  tileCount: number;
  columns: number;
  image?: string;
  source?: string;
}

export type DocumentSnapshot =
  | {
      documentType: "map";
      map: MapSnapshot;
    }
  | {
      documentType: "tileset";
      tileset: StandaloneTilesetSnapshot;
    };

export interface OpenDocumentResult {
  documentType: DocumentType;
  revision: number;
  snapshot: DocumentSnapshot;
}

export interface BridgeCommandRequest {
  type:
    | "create_layer"
    | "delete_layer"
    | "move_layer"
    | "set_layer_properties"
    | "paint_tiles"
    | "erase_tiles"
    | "create_object"
    | "update_object"
    | "delete_object"
    | "move_object"
    | "set_custom_properties";
  payload: Record<string, unknown>;
}

export interface BridgeCommandResult {
  revisionBefore: number;
  revisionAfter: number;
  changedEntities: ChangedEntity[];
  warnings: string[];
  errors: string[];
  undoDepth: number;
  redoDepth: number;
}

export interface ValidationDiagnostic {
  severity: "warning" | "error";
  code: string;
  message: string;
  path?: string;
  target?: string;
}

export interface ValidationReport {
  revision: number;
  diagnostics: ValidationDiagnostic[];
}

export interface BridgeClient {
  openDocument(sessionId: string, documentPath: string): Promise<OpenDocumentResult>;
  getSnapshot(sessionId: string): Promise<DocumentSnapshot>;
  executeCommand(sessionId: string, command: BridgeCommandRequest): Promise<BridgeCommandResult>;
  validateDocument(sessionId: string): Promise<ValidationReport>;
  saveDocument(sessionId: string): Promise<{ path: string; revision: number }>;
  undo(sessionId: string): Promise<BridgeCommandResult>;
  redo(sessionId: string): Promise<BridgeCommandResult>;
  closeSession(sessionId: string): Promise<void>;
}

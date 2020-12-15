declare const __filename: string;

interface rect {
  /**
   * X coordinate of the rectangle.
   */
  x: number;

  /**
   * Y coordinate of the rectangle.
   */
  y: number;

  /**
   * Width of the rectangle.
   */
  width: number;

  /**
   * Height of the rectangle.
   */
  height: number;
}

interface region {
  /**
   * Bounding rectangle of the region.
   */
  readonly boundingRect: rect;
}

interface point {
  /**
   * X coordinate of the point.
   */
  x: number;

  /**
   * Y coordinate of the point.
   */
  y: number;
}

interface size {
  width: number;
  height: number;
}

type Polygon = point[];

interface ObjectRef {
    id: number;
}

declare namespace Qt {
  export function point(x: number, y: number): point;
  export function rect(
    x: number,
    y: number,
    width: number,
    height: number
  ): rect;
}

declare namespace TextFile {
  export const ReadOnly = 1;
  export const WriteOnly = 2;
  export const ReadWrite = 3;
  export const Append = 4;

  type OpenMode =
    | typeof ReadOnly
    | typeof WriteOnly
    | typeof ReadWrite
    | typeof Append;
}

/**
 * The `TextFile` object is used to read and write files in text mode.
 *
 * When using `TextFile.WriteOnly`, you need to call {@link TextFile#commit()} when you’re
 * done writing otherwise the operation will be aborted without effect.
 */
declare class TextFile {
  /**
   * The path of the file.
   */
  public readonly filePath: string;

  /**
   * True if no more data can be read.
   */
  public readonly atEof: boolean;

  /**
   * The text codec.
   */
  public codec: string;

  /**
   * Opens a text file in the given mode.
   * @param filePath
   * @param mode
   */
  constructor(filePath: string, mode?: BinaryFile.OpenMode);

  /**
   * Reads one line of text from the file and returns it. The returned string does not contain the
   * newline characters.
   */
  public readLine(): string;

  /**
   * Reads all data from the file and returns it.
   */
  public readAll(): string;

  /**
   * Truncates the file, that is, gives it the size of zero, removing all content.
   */
  public truncate(): void;

  /**
   * Writes a string to the file.
   * @param text 
   */
  public write(text: string): void;

  /**
   * Writes a string to the file and appends a newline character.
   * @param text 
   */
  public writeLine(text: string): void;
  
  /**
    * Commits all written text to disk and closes the file. Should be called when writing to files in WriteOnly mode. Failing to call this function will result in cancelling the operation, unless safe writing to files is disabled.
    */
  public commit(): void;

  /**
    * Closes the file. It is recommended to always call this function as soon as you are finished with the file.
    */
  public close(): void;
}

declare namespace BinaryFile {
  export const ReadOnly = 1;
  export const WriteOnly = 2;
  export const ReadWrite = 3;

  type OpenMode = typeof ReadOnly | typeof WriteOnly | typeof ReadWrite;
}

/**
 * The `BinaryFile` object is used to read and write files in binary mode.
 *
 * When using `BinaryFile.WriteOnly`, you need to call {@link BinaryFile#commit()} when you’re
 * done writing otherwise the operation will be aborted without effect.
 */
declare class BinaryFile {
  /**
   * The path of the file.
   */
  public readonly filePath: string;
  /**
   * True if no more data can be read.
   */
  public readonly atEof: boolean;
  /**
   * The size of the file (in bytes).
   */
  public size: number;
  /**
   * The position that data is written to or read from.
   */
  public pos: number;

  /**
   * Opens a binary file in the given mode.
   * @param filePath
   * @param mode
   */
  constructor(filePath: string, mode?: BinaryFile.OpenMode);

  /**
   * Sets the file size (in bytes). If `size` is larger than the file currently is, the new bytes
   * will be set to 0; if `size` is smaller, the file is truncated.
   * @param size
   */
  public resize(size: number): void;

  /**
   * Sets the current position to `pos`.
   * @param pos
   */
  public seek(pos: number): void;

  /**
   * Reads at most `size` bytes of data from the file and returns it as an {@link ArrayBuffer}.
   * @param size
   */
  public read(size: number): ArrayBuffer;

  /**
   * Reads all data from the file and returns it as an {@link ArrayBuffer}.
   */
  public readAll(): ArrayBuffer;

  /**
   * Commits all written data to disk and closes the file. Should be called when writing to files
   * in WriteOnly mode. Failing to call this function will result in cancelling the operation,
   * unless safe writing to files is disabled.
   * @param data
   */
  public write(data: ArrayBuffer): void;

  /**
   * Closes the file. It is recommended to always call this function as soon as you are finished
   * with the file.
   */
  public close(): void;
}

interface Action {
  text: string;
  checkable: boolean;
  shortcut: string;
}

interface Asset {}

interface MapFormat {
  readonly name: string;
  readonly extension: string;
  read?(fileName: string): TileMap;
  write?(map: TileMap, fileName: string): string | undefined;
  outputFiles?: (map: TileMap, fileName: string) => string[];
}

interface MapEditor {}

interface Layer {}

declare class TileMap implements Asset {
  constructor();

  public autoMap(rulesFule?: string): void;
  public autoMap(region: region | rect, rulesFile?: string): void;
  public setSize(width: number, height: number): void;
  public setTileSize(width: number, height: number): void;
  public layerAt(index: number): Layer;
  public removeLayerAt(index: number): void;
  public removeLayer(layer: Layer): void;
  public insertLayerAt(index: number, layer: Layer): void;
  public addLayer(layer: Layer): void;
  public addTileset(tileset: Tileset): boolean;
  public replaceTileset(oldTileset: Tileset, newTileset: Tileset): boolean;
  public removeTileset(tileset: Tileset): boolean;
  public usedTilesets(): Tileset[];
  public merge(map: TileMap, canJoin?: boolean): void;
  public resize(size: size, offset?: point, removeObjects?: boolean): void;
  public screenToTile(x: number, y: number): point;
  public screenToTile(position: point): point;
  public tileToScreen(x: number, y: number): point;
  public tileToScreen(position: point): point;

  /**
   * Converts the given position from screen to pixel coordinates.
   * @param x
   * @param y
   */
  public screenToPixel(x: number, y: number): point;

  /**
   * Converts the given position from screen to pixel coordinates.
   * @param position
   */
  public screenToPixel(position: point): point;

  /**
   * Converts the given position from pixel to screen coordinates.
   * @param x
   * @param y
   */
  public pixelToScreen(x: number, y: number): point;

  /**
   * Converts the given position from pixel to screen coordinates.
   * @param position
   */
  public pixelToScreen(position: point): point;

  /**
   * Converts the given position from pixel to tile coordinates.
   * @param x
   * @param y
   */
  public tileToPixel(x: number, y: number): point;

  /**
   * Converts the given position from pixel to tile coordinates.
   * @param position
   */
  public tileToPixel(position: point): point;

  /**
   * Converts the given position from tile to pixel coordinates.
   * @param position
   */
  public pixelToTile(x: number, y: number): point;

  /**
   * Converts the given position from tile to pixel coordinates.
   * @param position
   */
  public pixelToTile(position: point): point;
}

interface Tileset {}

interface TilesetFormat {
  readonly name: string;
  readonly extension: string;
  read?: (fileName: string) => Tileset;
  write?: (tileset: Tileset, fileName: string) => string | undefined;
}

interface TilesetEditor {}

interface Tool {
    name: string;
    map: TileMap;
    selectedTile: any;
    preview: TileMap;
    tilePosition: point;
    statusInfo: string;
    enabled: boolean;
    activated(): void;
    deactivated(): void;
    keyPresed(key, modifiers): void;
}

declare namespace tiled {
  export const version: string;
  export const platform: string;
  export const arch: string;
  export const actions: string[];
  export const menus: string[];
  export let activeAsset: Asset;
  export const openAssets: Asset[];
  export const tilesetFormats: string[];
  export const mapFormats: string[];

  export function trigger(action: string): void;
  export function executeCommand(name: string, inTerminal: boolean): void;
  export function open(fileName: string): Asset | null;
  export function close(asset: Asset): boolean;
  export function reload(asset: Asset): Asset | null;
  export function alert(text: string, title?: string): void;
  export function confirm(text: string, title?: string): void;
  export function prompt(label: string, text?: string, title?: string): void;
  export function log(text: string): void;
  export function warn(text: string, activated: () => void): void;
  export function error(text: string, activated: () => void): void;

  export function extendMenu(
    shortName: string,
    menu: any
  ): void;

  export function registerAction(
    id: string,
    callback: (Action) => void
  ): Action;


  /**
   * Registers a new map format that can then be used to open and/or save
   * maps in that format.
   *
   * If a map format is already registered with the same `shortName`, the
   * existing format is replaced. The short name can also be used to
   * specify the format when using `--export-map` on the command-line, in
   * case the file extension is ambiguous or a different one should be
   * used.
   * 
   * @example
   * Example that produces a simple JSON representation of a map:
   * ``` js
   * var customMapFormat = {
   *     name: "Custom map format",
   *     extension: "custom",
   *
   *     write: function(map, fileName) {
   *         var m = {
   *             width: map.width,
   *             height: map.height,
   *             layers: []
   *         };
   *
   *         for (var i = 0; i < map.layerCount; ++i) {
   *             var layer = map.layerAt(i);
   *             if (layer.isTileLayer) {
   *                 var rows = [];
   *                 for (y = 0; y < layer.height; ++y) {
   *                     var row = [];
   *                     for (x = 0; x < layer.width; ++x)
   *                         row.push(layer.cellAt(x, y).tileId);
   *                     rows.push(row);
   *                 }
   *                 m.layers.push(rows);
   *             }
   *         }
   *
   *         var file = new TextFile(fileName, TextFile.WriteOnly);
   *         file.write(JSON.stringify(m));
   *         file.commit();
   *     },
   * }
   *
   * tiled.registerMapFormat("custom", customMapFormat)
   * ```
   */
  export function registerMapFormat(
    shortName: string,
    mapFormat: MapFormat
  ): void;
  export function registerTilesetFormat(
    shortName: string,
    tilesetFormat: TilesetFormat
  ): void;
}

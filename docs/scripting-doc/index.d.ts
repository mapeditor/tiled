/**
 * Tiled can be extended with the use of JavaScript.
 *
 * Scripts can be used to implement {@link tiled.registerMapFormat | custom map formats},
 * {@link tiled.registerAction | custom actions} and {@link tiled.registerTool | new tools}.
 * Scripts can also {@link Signal | automate actions based on signals}.
 *
 * See the [Tiled Manual](https://doc.mapeditor.org/en/stable/reference/scripting) for more information on writing or installing extensions.
 *
 * ### Type Definitions
 *
 * TypeScript type definitions for this API are available by installing the
 * [`@mapeditor/tiled-api`](https://www.npmjs.com/package/@mapeditor/tiled-api)
 * package, which allows you to write scripts using TypeScript and can provide
 * auto-completion in your editor (also when using plain JavaScript).
 */

/**
 * The file path of the current file being evaluated. Only available during
 * initial evaluation of the file and not when later functions in that file
 * get called. If you need it there, copy the value to local scope.
 */
// declare const __filename: string; // collides with nodejs types

/**
 * {@link Qt.rect} can be used to create a rectangle.
 */
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

/**
 * {@link Qt.point} can be used to create a point object.
 */
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

/**
 * {@link Qt.size} can be used to create a size object.
 */
interface size {
  /**
   * Width.
   */
  width: number;

  /**
   * Height.
   */
  height: number;
}

/**
 * A polygon is not strictly a custom type. It is an array of objects that each
 * have an ``x`` and ``y`` property, representing the points of the polygon.
 *
 * To modify the polygon of a {@link MapObject}, change or set up the
 * polygon array and then assign it to {@link MapObject.polygon}.
 */
type Polygon = point[];

/**
 * The value of a property of type 'object', which refers to a
 * {@link MapObject} by its ID. Generally only used as a fallback when an
 * object property cannot be resolved to an actual object. Can be created with
 * {@link tiled.objectRef}.
 */
interface ObjectRef {
  /**
   * The ID of the referenced object.
   */
  id: number;
}

interface MenuAction {
  /**
   * ID of a registered action that the menu item will represent.
   */
  action: string;

  /**
   * ID of the action before which this menu item should be added
   * (optional).
   */
  before?: string;
}

interface MenuSeparator {
  /**
   * Set to `true` if this item is a menu separator (optional).
   */
  separator: boolean;
}

type Menu = MenuAction|MenuSeparator

/**
 * Used as the value for custom 'file' properties. Can be created with
 * {@link tiled.filePath}.
 */
interface FilePath {
  /**
   * The URL of the file.
   */
  url: string;
}

/**
 * An object representing an event.
 *
 * Functions can be connected to the signal object, after which they will get
 * called when the signal is emitted. The {@link tiled} module provides several
 * useful signals, like {@link tiled.assetAboutToBeSaved}.
 *
 * Properties usually will have related signals which can be used to detect
 * changes to that property, but most of those are currently not implemented.
 *
 * To connect to a signal, call its {@link Signal.connect | connect} function and
 * pass in a function object. In the following example, newly created maps
 * automatically get their first tile layer removed:
 *
 * ```js
 * tiled.assetCreated.connect(function(asset) {
 *     if (asset.layerCount > 0) {
 *         asset.removeLayerAt(0)
 *         tiled.log("assetCreated: Removed automatically added tile layer.")
 *     }
 * })
 * ```
 *
 * In some cases it will be necessary to later disconnect the function from
 * the signal again. This can be done by defining the function separately
 * and passing it into the {@link Signal.disconnect | disconnect} function:
 *
 * ```js
 * function onAssetCreated(asset) {
 *     // Do something...
 * }
 *
 * tiled.assetCreated.connect(onAssetCreated)
 * // ...
 * tiled.assetCreated.disconnect(onAssetCreated)
 * ```
 */
interface Signal<Arg> {
  connect(callback: (arg: Arg) => void): void;
  disconnect(callback: (arg: Arg) => void): void;
}

/**
 * A global object with useful enums and functions from Qt.
 *
 * Only a small subset of available members in the `Qt` object are documented here.
 * See the [Qt QML Type reference](https://doc.qt.io/qt-5/qml-qtqml-qt.html) for the full documentation
 * (keep in mind, that the QtQuick module is not currently loaded).
 */
declare namespace Qt {
  export function point(x: number, y: number): point;
  export function rect(
    x: number,
    y: number,
    width: number,
    height: number
  ): rect;
  export function size(width: number, height: number): size;

  /**
   * Alignment is given by a set of flags.
   * To align to the top while horizontally centering, the value can be set to `Qt.AlignTop | Qt.AlignHCenter`.
   */
  type Alignment = number;

  const AlignLeft: Alignment;
  const AlignRight: Alignment;
  const AlignVCenter: Alignment;
  const AlignHCenter: Alignment;
  const AlignJustify: Alignment;
  const AlignTop: Alignment;
  const AlignBottom: Alignment;
  const AlignCenter: Alignment;
}

/**
 * The `TextFile` object is used to read and write files in text mode.
 *
 * When using {@link TextFile.WriteOnly}, you need to call {@link commit} when you’re
 * done writing otherwise the operation will be aborted without effect.
 *
 * To read and write files in binary mode, use {@link BinaryFile} instead.
 */
declare class TextFile {
  static readonly ReadOnly: unique symbol;
  static readonly WriteOnly: unique symbol;
  static readonly ReadWrite: unique symbol;
  static readonly Append: unique symbol;

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
   */
  constructor(filePath: string, mode?: typeof TextFile.ReadOnly | typeof TextFile.WriteOnly | typeof TextFile.ReadWrite | typeof TextFile.Append);

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
   */
  public write(text: string): void;

  /**
   * Writes a string to the file and appends a newline character.
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

/**
 * The `BinaryFile` object is used to read and write files in binary mode.
 *
 * When using {@link BinaryFile.WriteOnly}, you need to call {@link commit} when you’re
 * done writing otherwise the operation will be aborted without effect.
 *
 * To read and write files in text mode, use {@link TextFile} instead.
 */
declare class BinaryFile {
  static readonly ReadOnly: unique symbol;
  static readonly WriteOnly: unique symbol;
  static readonly ReadWrite: unique symbol;

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
   */
  constructor(filePath: string, mode?: typeof BinaryFile.ReadOnly | typeof BinaryFile.WriteOnly | typeof BinaryFile.ReadWrite);

  /**
   * Sets the file size (in bytes). If `size` is larger than the file currently is, the new bytes
   * will be set to 0; if `size` is smaller, the file is truncated.
   */
  public resize(size: number): void;

  /**
   * Sets the current position to `pos`.
   */
  public seek(pos: number): void;

  /**
   * Reads at most `size` bytes of data from the file and returns it as an `ArrayBuffer`.
   */
  public read(size: number): ArrayBuffer;

  /**
   * Reads all data from the file and returns it as an `ArrayBuffer`.
   */
  public readAll(): ArrayBuffer;

  /**
   * Writes data into the file at the current position.
   */
  public write(data: ArrayBuffer): void;

  /**
   * Commits all written data to disk and closes the file. Should be called when writing to files
   * in WriteOnly mode. Failing to call this function will result in cancelling the operation,
   * unless safe writing to files is disabled.
   */
  public commit(): void;

  /**
   * Closes the file. It is recommended to always call this function as soon as you are finished
   * with the file.
   */
  public close(): void;
}

/**
 * An action that was registered with {@link tiled.registerAction}.
 *
 * This class is used to change the properties of the action.
 * It can be added to a menu using {@link tiled.extendMenu}.
 */
interface Action {
  /**
   * The ID this action was registered with.
   */
  readonly id: string;

  /**
   * The text used when the action is part of a menu.
   */
  text: string;

  /**
   * Whether the action is checked.
   */
  checked: boolean;

  /**
   * Whether the action can be checked.
   */
  checkable: boolean;

  /**
   * Whether the action is enabled.
   */
  enabled: boolean;

  /**
   * File name of an icon.
   */
  icon: string;

  /**
   * Whether the action should show an icon
   * in a menu.
   */
  iconVisibleInMenu: boolean;

  /**
   * The shortcut (can be assigned a string like "Ctrl+K").
   */
  shortcut: string;

  /**
   * Whether the action is visible.
   */
  visible: boolean;

  /**
   * Triggers the action.
   */
  trigger(): void;

  /**
   * Changes the checked state to its opposite state.
   */
  toggle(): void;
}

/**
 * The "ObjectGroup" is a type of layer that can contain objects. It will
 * henceforth be referred to as a layer.
 */
declare class ObjectGroup extends Layer {
  static readonly UnknownOrder: unique symbol
  static readonly TopDownOrder: unique symbol
  static readonly IndexOrder: unique symbol

  /**
   * Array of all objects on this layer.
   */
  readonly objects : MapObject[]

  /**
   * Number of objects on this layer.
   */
  readonly objectCount : number

  /**
   * Color of shape and point objects on this layer (when not set by object type).
   */
  color : color

  /**
   * The objects can either be drawn top down (sorted by their y-coordinate) or
   * by index (manual stacking order).
   *
   * The default is top down.
   */
  drawOrder : typeof ObjectGroup.TopDownOrder | typeof ObjectGroup.IndexOrder | typeof ObjectGroup.UnknownOrder;

  /**
   * Constructs a new object layer, which can be added to a TileMap.
   */
  constructor(name? : string)

  /**
   * Returns a reference to the object at the given index. When the object is removed, the reference turns into a standalone copy of the object.
   */
  objectAt(index : number) : MapObject

  /**
   * Removes the object at the given index.
   */
  removeObjectAt(index : number) : void

  /**
   * Removes the given object from this layer. The object reference turns into a standalone copy of the object.
   */
  removeObject(object : MapObject) : void

  /**
   * Inserts the object at the given index. The object can’t already be part of a layer.
   */
  insertObjectAt(index : number, object : MapObject) : void

  /**
   * Adds the given object to the layer. The object can’t already be part of a layer.
   */
  addObject(object : MapObject) : void

}

type TiledObjectPropertyValue = number | string | boolean | ObjectRef | FilePath | MapObject | undefined

interface TiledObjectProperties {
  [name:string]:TiledObjectPropertyValue
}

/**
 * The base of most data types in Tiled. Provides the ability to associate
 * custom properties with the data.
 */
declare class TiledObject {
  /**
   * The asset this object is part of, or `null`.
   */
  readonly asset: Asset;

  /**
   * Whether the object is read-only.
   */
  readonly readOnly: boolean;

  /**
   * Returns the value of the custom property with the given name, or
   * `undefined` if no such property is set on the object. Does not
   * include inherited values (see {@link resolvedProperty}).
   *
   * `file` properties are returned as {@link FilePath}.
   *
   * `object` properties are returned as {@link MapObject} when possible,
   * or {@link ObjectRef} when the object could not be found.
   */
  property(name: string): TiledObjectPropertyValue;

  /**
   * Sets the value of the custom property with the given name. Supported
   * types are `bool`, `number`, `string`, {@link FilePath},
   * {@link ObjectRef} and {@link MapObject}.
   *
   * When setting a `number`, the property type will be set to either
   * `int` or `float`, depending on whether it is a whole number.
   *
   * @note Support for setting `color` properties is currently missing.
   */
  setProperty(name: string, value: TiledObjectPropertyValue): void;

  /**
   * Returns all custom properties set on this object.
   *
   * Modifications to the properties will not affect the original object.
   * Does not include inherited values (see {@link resolvedProperties}).
   */
  properties(): TiledObjectProperties;

  /**
   * Replaces all currently set custom properties with a new set of
   * properties.
   */
  setProperties(properties: TiledObjectProperties): void;

  /**
   * Removes the custom property with the given name.
   */
  removeProperty(name: string): void;

  /**
   * Returns the value of the custom property with the given name, or
   * `undefined` if no such property is set. Includes values inherited
   * from object types, templates and tiles where applicable.
   */
  resolvedProperty(name: string): TiledObjectPropertyValue;

  /**
   * Returns all custom properties set on this object. Modifications to
   * the properties will not affect the original object. Includes values
   * inherited from object types, templates and tiles where applicable.
   */
  resolvedProperties(): TiledObjectProperties;
}


interface Font {
  /**
   * The font family.
   */
  family : string

  /**
   * Font size in pixels.
   */
  pixelSize : number

  /**
   * Whether the font is bold.
   */
  bold : boolean

  /**
   * Whether the font is italic.
   */
  italic : boolean

  /**
   * Whether the text is underlined.
   */
  underline : boolean

  /**
   * Whether the text is striked through.
   */
  strikeOut : boolean

  /**
   * Whether to use kerning when rendering the text.
   */
  kerning : boolean
}

declare class MapObject extends TiledObject {
  static readonly Rectangle: unique symbol
  static readonly Polygon: unique symbol
  static readonly Polyline: unique symbol
  static readonly Ellipse: unique symbol
  static readonly Text: unique symbol
  static readonly Point: unique symbol

  /**
   * Unique (map-wide) ID of the object.
   */
  readonly id: number;

  /**
   * Shape of the object.
   */
  shape: typeof MapObject.Rectangle | typeof MapObject.Polygon | typeof MapObject.Polyline | typeof MapObject.Ellipse | typeof MapObject.Text | typeof MapObject.Point

  /**
   * Name of the object.
   */
  name: string;

  /**
   * Type of the object.
   */
  type: string;

  /**
   * X coordinate of the object in pixels.
   */
  x: number;

  /**
   * Y coordinate of the object in pixels.
   */
  y: number;

  /**
   * Position of the object in pixels.
   */
  pos: point;

  /**
   * Width of the object in pixels.
   */
  width: number;

  /**
   * Height of the object in pixels.
   */
  height: number;

  /**
   * Size of the object in pixels.
   */
  size: size;

  /**
   * Rotation of the object in degrees clockwise.
   */
  rotation: number;

  /**
   *
   */
  visible: boolean;

  /**
   * Polygon of the object.
   */
  polygon: Polygon;

  /**
   * The text of a text object.
   */
  text: string;

  /**
   * The font of a text object.
   */
  font: Font;

  /**
   * The alignment of a text object.
   */
  textAlignment: Qt.Alignment;

  /**
   * Whether the text of a text object wraps based on the width of the object.
   */
  wordWrap: boolean;

  /**
   * Color of a text object.
   */
  textColor: color;

  /**
   * Tile of the object.
   */
  tile: Tile;

  /**
   * Whether the tile is flipped horizontally.
   */
  tileFlippedHorizontally: boolean;

  /**
   * Whether the tile is flipped vertically.
   */
  tileFlippedVertically: boolean;

  /**
   * Whether the object is selected.
   */
  selected: boolean;

  /**
   * Layer this object is part of (or `null` in case of a standalone
   * object).
   */
  layer: ObjectGroup;

  /**
   * Map this object is part of (or `null` in case of a
   * standalone object).
   */
  readonly map: TileMap;

  /**
   * Constructs a new map object, which can be added to an {@link ObjectGroup}.
   */
  constructor(name? : string)
}

/**
 * Represents any top-level data type that can be saved to a file.
 *
 * Currently either a {@link TileMap} or a {@link Tileset}.
 *
 * For assets that are loaded in the editor, all modifications and
 * modifications to their contained parts create undo commands. This
 * includes both modifying functions that are called as well as simply
 * assigning to a writable property.
 */
declare class Asset extends TiledObject {
  /**
   * File name of the asset.
   */
  readonly fileName: string;

  /**
   * Whether the asset was modified after it was saved or loaded.
   */
  readonly modified: boolean;

  /**
   * Whether the asset is a {@link TileMap}.
   */
  readonly isTileMap: boolean;

  /**
   * Whether the asset is a {@link Tileset}.
   */
  readonly isTileset: boolean;

  /**
   * Creates a single undo command that wraps all changes applied to this
   * asset by the given callback. Recommended to avoid spamming the undo
   * stack with small steps that the user does not care about.
   *
   * Example function that changes visibility of multiple layers in one
   * step:
   *
   * ```js
   * tileMap.macro((visible ? "Show" : "Hide") + " Selected Layers", function() {
   *     tileMap.selectedLayers.forEach(function(layer) {
   *         layer.visible = visible
   *     })
   * })
   * ```
   *
   * The returned value is whatever the callback function returned.
   */
  macro<T>(text: string, callback: () => T): T;

  /**
   * Undoes the last applied change.
   *
   * @note The undo system is only enabled for assets loaded in the editor!
   */
  undo(): void;

  /**
   * Redoes the last change that was undone.
   *
   * @note The undo system is only enabled for assets loaded in the editor!
   */
  redo(): void;
}

/**
 * Common functionality for file format readers and writers.
 *
 * @since 1.4
 */
interface FileFormat {
  /**
   * Whether this format supports reading files.
   */
  readonly canRead: boolean;

  /**
   * Whether this format supports writing files.
   */
  readonly canWrite: boolean;

  /**
   * Returns whether the given file is readable by this format.
   */
  supportsFile(fileName: string): boolean;
}

/**
 * An object that can read or write map files.
 *
 * Implementations of this interface are returned from {@link tiled.mapFormat} and {@link tiled.mapFormatForFile}.
 *
 * @since 1.4
 */
interface MapFormat extends FileFormat {
  /**
   * Reads the given file as a map.
   *
   * This function will throw an error if reading is not supported.
   */
  read(fileName : string) : TileMap

  /**
   * Writes the given map to a file.
   *
   * This function will throw an error if writing is not supported.
   *
   * If there is an error writing the file, it will return a description of the error; otherwise, it will return "".
   */
  write(map : TileMap, fileName : string) : string
}

/**
 * An object that can read or write tileset files.
 *
 * Implementations of this interface are returned from {@link tiled.tilesetFormat} and {@link tiled.tilesetFormatForFile}.
 *
 * @since 1.4
 */
interface TilesetFormat extends FileFormat {
  /**
   * Reads the given file as a tileset.
   *
   * This function will throw an error if reading is not supported.
   */
  read(fileName : string) : Tileset

  /**
   * Writes the given tileset to a file.
   *
   * This function will throw an error if writing is not supported.
   *
   * If there is an error writing the file, it will return a description of the error; otherwise, it will return "".
   */
  write(tileset : Tileset, fileName : string) : string
}

/**
 * Offers various operations on file paths, such as turning absolute paths
 * into relative ones, splitting a path into its components, and so on.
 */
interface FileInfo {  // TODO: namespace instead of interface?
  /**
   * Returns the file name of `filePath` up to (but not including) the
   * first '.' character.
   */
  baseName(filePath: string): string;

  /**
   * Returns a canonicalized `filePath`, i.e. an absolute path without
   * symbolic links or redundant "." or ".." elements. On Windows,
   * drive substitutions are also resolved.
   *
   * It is recommended to use `canonicalPath` in only those cases where
   * canonical paths are really necessary. In most cases, `cleanPath`
   * should be enough.
   */
  canonicalPath(filePath: string): string;

  /**
   * Returns `filePath` without redundant separators and with resolved
   * occurrences of `.` and `..` components. For
   * instance, `/usr/local//../bin/` becomes `/usr/bin`.
   */
  cleanPath(filePath: string): string;

  /**
   * Returns the file name of `filePath` up to (but not including) the
   * last `.` character.
   */
  completeBaseName(filePath: string): string;

  /**
   * Returns the file suffix of `filePath` from (but not including) the
   * last `.` character.
   */
  completeSuffix(filePath: string): string;

  /**
   * Returns the last component of `filePath`, that is, everything after
   * the last `/` character.
   */
  fileName(filePath: string): string;

  /**
   * On Windows, returns `filePath` with all `\` characters replaced
   * by `/`. On other operating systems, it returns the input
   * unmodified.
   */
  fromNativeSeparators(filePath: string): string;

  /**
   * Returns true if `filePath` is an absolute path and false
   * if it is a relative one.
   */
  isAbsolutePath(filePath: string): boolean;

  /**
   * Concatenates the given paths using the `/` character.
   */
  joinPaths(...paths:string[]) : string;

  /**
   * Returns the part of `filePath` that is not the file name, that is,
   * everything up to (but not including) the last `/` character. If
   * `filePath` is just a file name, then `.` is returned. If
   * `filePath` ends with a `/` character, then the file name is
   * assumed to be empty for the purpose of the above definition.
   */
  path(filePath: string): string;

  /**
   * Returns the path to `filePath` relative to the directory `dirPath`.
   * If necessary, `..` components are inserted.
   */
  relativePath(dirPath: string, filePath: string): string;

  /**
   * Returns the file suffix of `filePath` from (but not including) the
   * first `.` character.
   */
  suffix(filePath: string): string;

  /**
   * On Windows, returns `filePath` with all `/` characters replaced by
   * `\`. On other operating systems, it returns the input unmodified.
   */
  toNativeSeparators(filePath: string): string;
}

interface File {  // TODO: namespace instead of interface?
  readonly Dirs: 0x001
  readonly Files: 0x002
  readonly Drives: 0x004
  readonly NoSymLinks: 0x008
  readonly AllEntries: 0x007
  readonly TypeMask: 0x00f
  readonly Readable: 0x010
  readonly Writable: 0x020
  readonly Executable: 0x040
  readonly PermissionMask: 0x070
  readonly Modified: 0x080
  readonly Hidden: 0x100
  readonly System: 0x200
  readonly AccessMask: 0x3F0
  readonly AllDirs: 0x400
  readonly CaseSensitive: 0x800
  readonly NoDot: 0x2000
  readonly NoDotDot: 0x4000
  readonly NoDotAndDotDot: 0x6000
  readonly NoFilter: -1
  readonly Name: 0x00
  readonly Time: 0x01
  readonly Size: 0x02
  readonly Unsorted: 0x03
  readonly SortByMask: 0x03
  readonly DirsFirst: 0x04
  readonly Reversed: 0x08
  readonly IgnoreCase: 0x10
  readonly DirsLast: 0x20
  readonly LocaleAware: 0x40
  readonly Type: 0x80
  readonly NoSort: -1

  /**
   * Copies `sourceFilePath` to `targetFilePath`. Any directory components
   * in `targetFilePath` that do not yet exist will be created. If `sourceFilePath`
   * is a directory, a recursive copy will be made. If an error occurs, a
   * JavaScript exception will be thrown.
   *
   * @note `targetFilePath` must be the counterpart of `sourceFilePath` at the new
   * location, not the new parent directory. This allows the copy to have a
   * different name and is true even if `sourceFilePath` is a directory.
   *
   * @note The file is not copied if the source file timestamp is older than the
   * destination file timestamp. If you want to replace the newer file, you need to
   * remove it first via {@link File.remove}.
   */
  copy(sourceFilePath: string, targetFilePath: string): boolean;

  /**
   * Returns true if and only if there is a file at `filePath`.
   */
  exists(filePath: string): boolean;

  /**
   * Returns a list of the directory `path`'s contents non-recursively, filtered by
   * the given `filters` and sorted by the given `sortFlags` (defaults to sorting by
   * name).
   *
   * The values for `filters` are equivalent to Qt's `QDir::Filter`. The `sortFlags`
   * are equivalent to `QDir::SortFlags`.
   */
  directoryEntries(path: string, filters?: number, sortFlags?: number): string[];

  /**
   * Returns the time of last modification for the file at `filePath`. The
   * concrete semantics of the returned value are platform-specific. You should
   * only rely on the property that a smaller value indicates an older timestamp.
   */
  lastModified(filePath: string): Date;

  /**
   * Makes the directory at `path`, creating intermediate directories if necessary.
   * Conceptually equivalent to `mkdir -p`.
   */
  makePath(path: string): boolean;

  /**
   * Renames the file `sourceFile` to `targetFile`. Returns `true` if successful;
   * otherwise returns `false`.
   *
   * The `overwrite` parameter is `true` by default.
   *
   * If a file with the name `targetFile` already exists, and overwrite is `false`,
   * `move()` returns `false` (that is, the file will not be overwritten).
   */
  move(sourceFile: string, targetFile: string, overwrite?: boolean): boolean;

  /**
   * Removes the file at `filePath`. In case of a directory, it will be removed
   * recursively.
   */
  remove(filePath: string): boolean;
}

/**
 * A layer that groups several other layers.
 */
declare class GroupLayer extends Layer {
  /**
   * Number of child layers the group layer has.
   */
  readonly layerCount: number;

  /**
   * Constructs a new group layer.
   */
  constructor(name? : string)

  /**
   * Returns a reference to the child layer at the given index.
   */
  layerAt(index: number): Layer;

  /**
   * Removes the child layer at the given index. When a reference to the
   * layer still exists and this group layer isn't already standalone,
   * that reference becomes a standalone copy of the layer.
   */
  removeLayerAt(index: number): void

  /**
   * Removes the given layer from the group. If this group wasn't
   * standalone, the reference to the layer becomes a standalone copy.
   */
  removeLayer(layer: Layer): void;

  /**
   * Inserts the layer at the given index. The layer can't already be
   * part of a map.
   *
   * When adding a {@link TileLayer} to a map, the layer's width and height
   * are automatically initialized to the size of the map (since Tiled 1.4.2).
   */
  insertLayerAt(index: number, layer: Layer): void;

  /**
   * Adds the layer to the group, above all existing layers. The layer
   * can't already be part of a map.
   *
   * When adding a {@link TileLayer} to a map, the layer's width and height
   * are automatically initialized to the size of the map (since Tiled 1.4.2).
   */
  addLayer(layer: Layer): void;
}

/**
 * Can be used to create, load, save and modify images. Also useful when
 * writing an importer, where the image can be set on a tileset or its
 * tiles ({@link Tileset.loadFromImage} and {@link Tile.setImage}).
 *
 * @since 1.5
 */
declare class Image {
  static readonly Format_Invalid: unique symbol
  static readonly Format_Mono: unique symbol
  static readonly Format_MonoLSB: unique symbol
  static readonly Format_Indexed8: unique symbol
  static readonly Format_RGB32: unique symbol
  static readonly Format_ARGB32: unique symbol
  static readonly Format_ARGB32_Premultiplied: unique symbol
  static readonly Format_RGB16: unique symbol
  static readonly Format_ARGB8565_Premultiplied: unique symbol
  static readonly Format_RGB666: unique symbol
  static readonly Format_ARGB6666_Premultiplied: unique symbol
  static readonly Format_RGB555: unique symbol
  static readonly Format_ARGB8555_Premultiplied: unique symbol
  static readonly Format_RGB888: unique symbol
  static readonly Format_RGB444: unique symbol
  static readonly Format_ARGB4444_Premultiplied: unique symbol
  static readonly Format_RGBX8888: unique symbol
  static readonly Format_RGBA8888: unique symbol
  static readonly Format_RGBA8888_Premultiplied: unique symbol
  static readonly Format_BGR30: unique symbol
  static readonly Format_A2BGR30_Premultiplied: unique symbol
  static readonly Format_RGB30: unique symbol
  static readonly Format_A2RGB30_Premultiplied: unique symbol
  static readonly Format_Alpha8: unique symbol
  static readonly Format_Grayscale8: unique symbol
  static readonly Format_RGBX64: unique symbol
  static readonly Format_RGBA64: unique symbol
  static readonly Format_RGBA64_Premultiplied: unique symbol
  static readonly Format_Grayscale16: unique symbol
  static readonly Format_BGR888: unique symbol

  static readonly IgnoreAspectRatio: unique symbol
  static readonly KeepAspectRatio: unique symbol
  static readonly KeepAspectRatioByExpanding: unique symbol

  static readonly FastTransformation: unique symbol
  static readonly SmoothTransformation: unique symbol

  /**
   * Width of the image in pixels.
   */
  readonly width: number;

  /**
   * Height of the image in pixels.
   */
  readonly height: number;

  /**
   * Number of bits used to store a single pixel.
   */
  readonly depth: number;

  /**
   * Size of the image in pixels.
   */
  readonly size: size;

  /**
   * Format of the image. The format is defined by one of the `Image.Format_` values.
   */
  readonly format: number;

  /**
   * Constructs an empty image.
   */
  constructor();

  /**
   * Constructs an image of the given size using the given format. The format is defined by one of the `Image.Format_` values.
   */
  constructor(width: number, height: number, format: number);

  /**
   * Constructs an image from the given data, interpreting it in the
   * specified format and size. The format is defined by one of the `Image.Format_` values.
   */
  constructor(
    data: ArrayBuffer,
    width: number,
    height: number,
    format: number
    );

  /**
   * Constructs an image from the given data, interpreting it in the
   * specified format and size. The `bytesPerLine` argument
   * specifies the stride and can be useful for referencing a sub-image.
   * The format is defined by one of the `Image.Format_` values.
   */
  constructor(
    data: ArrayBuffer,
    width: number,
    height: number,
    bytesPerLine: number,
    format: number
    );

  /**
   * Construct an image by loading it from the given file name. When no
   * format is given it will be auto-detected (can be "bmp", "png",
   * etc.).
   */
  constructor(fileName: string, format?: string);

  /**
   * Returns the 32-bit color value.
   */
  pixel(x: number, y: number): number;

  /**
   * Returns the color at the given position as string like "#rrggbb".
   */
  pixelColor(x: number, y: number): string;

  /**
   * Sets the color at the specified location to the given 32-bit color
   * value or color table index.
   */
  setPixel(x: number, y: number, index_or_rgb: number): void;

  /**
   * Sets the color at the specified location to the given color by
   * string (supports values like "#rrggbb").
   */
  setPixelColor(x: number, y: number, color: string): void;

  /**
   * Fills the image with the given 32-bit color value or color table
   * index.
   */
  fill(index_or_rgb: number): void;

  /**
   * Fills the image with the given color by string (supports values like
   * "#rrggbb").
   */
  fill(color: string): void;

  /**
   * Loads the image from the given file name. When no format is given it
   * will be auto-detected (can be "bmp", "png", etc.).
   */
  load(fileName: string, format?: string): void;

  /**
   * Loads the image from the given data interpreted with the given
   * format (can be "bmp", "png", etc.).
   */
  loadFromData(data: ArrayBuffer, format: string): void;

  /**
   * Saves the image to the given file.
   *
   * When no format is given it will be auto-detected based on the file extension.
   */
  save(fileName : string, format? : string, quality? : number) : boolean

  /**
   * Saves the image to an ArrayBuffer in the given format (can be "bmp", png", etc.).
   */
  saveToData(format : string, quality? : number) : ArrayBuffer

  /**
   * Returns the 32-bit color value at the given index in the color
   * table.
   */
  color(index: number): number;

  /**
   * Returns the color table as an array of 32-bit color values.
   */
  colorTable(): number[];

  /**
   * Sets the color at the given index in the color table to a given
   * 32-bit color value.
   */
  setColor(index: number, rgb: number): void;

  /**
   * Sets the color at the given index in the color table to a color by
   * string (supports values like "#rrggbb").
   */
  setColor(index: number, color: string) : void;

  /**
   * Sets the color table given by an array of either 32-bit color values
    or strings (supports values like "#rrggbb").
   */
  setColorTable(colors: number[] | string[]): void;

  /**
   * Copies the given rectangle to a new image object.
   */
  copy(x: number, y: number, width: number, height: number) : Image;

  /**
   * Returns a scaled copy of this image. Default `aspectRatioMode`
   * behavior is to ignore the aspect ratio. Default `mode` is a fast
   * transformation.
   */
  scaled(width: number, height: number,
         aspectRatioMode?: typeof Image.IgnoreAspectRatio  | typeof Image.KeepAspectRatio  | typeof Image.KeepAspectRatioByExpanding,
         transformationMode?: typeof Image.FastTransformation  | typeof Image.SmoothTransformation): Image;

  /**
   * Returns a mirrored copy of this image.
   */
  mirrored(horizontal: boolean, vertical: boolean) : Image;
}

declare class ImageLayer extends Layer {
  /**
   * Color used as transparent color when rendering the image.
   */
  transparentColor: number;

  /**
   * Reference to the image rendered by this layer.
   */
  imageSource: string;

  /**
   * Constructs a new image layer.
   */
  constructor(name? : string);

  /**
   * Sets the image for this layer to the given image, optionally also
   * setting the source of the image.
   *
   * @warning This function has no undo!
   */
  loadFromImage(image: Image, source?: string) : void;
}

/**
 * The interface that should be implemented for objects passed to {@link tiled.registerMapFormat}.
 */
interface ScriptedMapFormat {
  /**
   * Name of the format as shown in the file dialog.
   */
  readonly name: string;

  /**
   * The file extension used by the format.
   */
  readonly extension: string;

  /**
   * A function that reads a map from the given file. Can use
   * {@link TextFile} or {@link BinaryFile} to read the file.
   */
  read?(fileName: string): TileMap;

  /**
   * A function that writes a map to the given
   * file. Can use {@link TextFile} or {@link BinaryFile} to write the file. * When a non-empty string is returned, it is shown as error message.
   */
  write?(map: TileMap, fileName: string): string | undefined;

  /**
   * A function that returns the list of files that will
   * be written when exporting the given map (optional).
   */
  outputFiles?(map: TileMap, fileName: string): string[];
}

interface MapEditor {
  /**
   * Get or set the currently used tile brush.
   */
  currentBrush : TileMap

  /**
   * Access the current map view.
   */
  readonly currentMapView : MapView

  /**
   * Access the Tilesets view
   */
  readonly tilesetsView: TilesetsView
}

interface TilesetsView {
  /**
   * Access or change the currently displayed tileset
   */
  currentTileset: Tileset
  /**
   * A list of the tiles that are selected in the current tileset.
   */
  selectedTiles: Tile[]
}

interface frame {
  /**
   * The local tile ID used to represent the frame.
   */
  tileId : number

  /**
   * Duration of the frame in milliseconds.
   */
  duration : number
}

declare class Tile extends TiledObject {
  static readonly FlippedHorizontally: 0x01
  static readonly FlippedVertically: 0x02
  static readonly FlippedAntiDiagonally: 0x04
  static readonly RotatedHexagonal120: 0x08

  /**
   * ID of this tile within its tileset.
   */
  readonly id : number

  /**
   * Width of the tile in pixels.
   */
  readonly width : number

  /**
   * Height of the tile in pixels.
   */
  readonly height : number

  /**
   * Size of the tile in pixels.
   */
  readonly size : size

  /**
   * Type of the tile.
   */
  type : string

  /**
   * File name of the tile image (when the tile is part of an image collection tileset).
   */
  imageFileName : string

  /**
   * Probability that the tile gets chosen relative to other tiles.
   */
  probability : number

  /**
   * The ObjectGroup associated with the tile in case collision shapes were defined. Returns null if no collision shapes were defined for this tile.
   */
  objectGroup : ObjectGroup

  /**
   * This tile’s animation as an array of frames.
   */
  frames : frame[]

  /**
   * Indicates whether this tile is animated.
   */
  readonly animated : boolean

  /**
   * The tileset of the tile.
   */
  readonly tileset : Tileset

  /**
   * Sets the image of this tile.
   *
   * @warning This function has no undo and does not affect the saved tileset!
   */
  setImage(image : Image) : void
}

declare class Layer extends TiledObject {
  /**
   * Unique (map-wide) ID of the layer
   *
   * @since 1.5
   */
  readonly id: number;

  /**
   * Name of the layer.
   */
  name: string;

  /**
   * Opacity of the layer, from 0 (fully transparent) to 1 (fully opaque).
   */
  opacity: any;

  /**
   * Tint color of the layer. Will be used to tint any images rendered by this
   * layer or by any child layers. Affects tile layers, image layers and tile
   * objects.
   */
  tintColor: color;

  /**
   * Whether the layer is visible (affects child layer visibility for group layers).
   */
  visible: boolean;

  /**
   * Whether the layer is locked (affects whether child layers are locked for group layers).
   */
  locked: boolean;

  /**
   * Offset in pixels that is applied when this layer is rendered.
   */
  offset: point;

  /**
   * Map that this layer is part of (or `null` in case of a standalone layer).
   */
  map: TileMap;

  /**
   * Whether the layer is selected.
   */
  selected: boolean;

  /**
   * Whether this layer is a {@link TileLayer}.
   */
  readonly isTileLayer: boolean;

  /**
   * Whether this layer is an {@link ObjectGroup}.
   */
  readonly isObjectLayer: boolean;

  /**
   * Whether this layer is a {@link GroupLayer}.
   */
  readonly isGroupLayer: boolean;

  /**
   * Whether this layer is an {@link ImageLayer}.
   */
  readonly isImageLayer: boolean;
}

interface SelectedArea {
  /**
   * Bounding rectangle of the selected area.
   */
  readonly boundingRect: rect;

  /**
   * Returns the selected region.
   */
  get() : region

  /**
   * Sets the selected area to the given rectangle.
   */
  set(rect : rect) : void

  /**
   * Sets the selected area to the given region.
   */
  set(region : region) : void

  /**
   * Adds the given rectangle to the selected area.
   */
  add(rect : rect) : void

  /**
   * Adds the given region to the selected area.
   */
  add(region : region) : void

  /**
   * Subtracts the given rectangle from the selected area.
   */
  subtract(rect : rect) : void

  /**
   * Subtracts the given region from the selected area.
   */
  subtract(region : region) : void

  /**
   * Sets the selected area to the intersection of the current selected area and the given rectangle.
   */
  intersect(rect : rect) : void

  /**
   * Sets the selected area to the intersection of the current selected area and the given region.
   */
  intersect(region : region) : void
}

declare class TileMap extends Asset {
  static readonly Unknown: unique symbol
  static readonly Orthogonal: unique symbol
  static readonly Isometric: unique symbol
  static readonly Staggered: unique symbol
  static readonly Hexagonal: unique symbol

  static readonly XML: unique symbol
  static readonly Base64: unique symbol
  static readonly Base64Gzip: unique symbol
  static readonly Base64Zlib: unique symbol
  static readonly Base64Zstandard: unique symbol
  static readonly CSV: unique symbol

  static readonly RightDown: unique symbol
  static readonly RightUp: unique symbol
  static readonly LeftDown: unique symbol
  static readonly LeftUp: unique symbol

  static readonly StaggerX: unique symbol
  static readonly StaggerY: unique symbol

  static readonly StaggerOdd: unique symbol
  static readonly StaggerEven: unique symbol


  /**
   * Width of the map in tiles (only relevant for non-infinite maps).
   */
  width : number

  /**
   * Height of the map in tiles (only relevant for non-infinite maps).
   */
  height : number

  /**
   * Size of the map in tiles (only relevant for non-infinite maps).
   */
  readonly size : size

  /**
   * Tile width (used by tile layers).
   */
  tileWidth : number

  /**
   * Tile height (used by tile layers).
   */
  tileHeight : number

  /**
   * Whether this map is infinite.
   */
  infinite : boolean

  /**
   * Length of the side of a hexagonal tile (used by tile layers on hexagonal maps).
   */
  hexSideLength : number

  /**
   * For staggered and hexagonal maps, determines which axis (X or Y) is staggered.
   */
  staggerAxis : typeof TileMap.StaggerX | typeof TileMap.StaggerY

  /**
   * General map orientation
   */
  orientation : typeof TileMap.Orthogonal | typeof TileMap.Isometric | typeof TileMap.Staggered | typeof TileMap.Hexagonal | typeof TileMap.Unknown

  /**
   * Tile rendering order (only implemented for orthogonal maps)
   */
  renderOrder : typeof TileMap.RightDown | typeof TileMap.RightUp | typeof TileMap.LeftDown | typeof TileMap.LeftUp

  /**
   * For staggered and hexagonal maps, determines whether the even or odd indexes along the staggered axis are shifted.
   */
  staggerIndex : typeof TileMap.StaggerOdd | typeof TileMap.StaggerEven

  /**
   * Background color of the map.
   */
  backgroundColor : color

  /**
   * The format in which the layer data is stored, taken into account by TMX, JSON and Lua map formats.
   */
  layerDataFormat : typeof TileMap.XML | typeof TileMap.Base64 | typeof TileMap.Base64Gzip | typeof TileMap.Base64Zlib | typeof TileMap.Base64Zstandard | typeof TileMap.CSV

  /**
   * Number of top-level layers the map has.
   */
  readonly layerCount : number

  /**
   * The list of tilesets referenced by this map. To determine which tilesets are actually used, call usedTilesets().
   */
  tilesets : Tileset[]

  /**
   * The selected area of tiles.
   */
  selectedArea : SelectedArea

  /**
   * The current layer.
   */
  currentLayer : Layer

  /**
   * Selected layers.
   */
  selectedLayers : Layer[]

  /**
   * Selected objects.
   */
  selectedObjects : MapObject[]

  /**
   * Constructs a new map.
   */
  constructor();

  /**
   * Applies [Automapping](https://doc.mapeditor.org/en/stable/manual/automapping/) using the given rules file, or using the default rules file is none is given.
   *
   * This operation can only be applied to maps loaded from a file.
   */
  public autoMap(rulesFule?: string): void;

  /**
   * Applies [Automapping](https://doc.mapeditor.org/en/stable/manual/automapping/) in the given region using the given rules file, or using the default rules file is none is given.
   *
   * This operation can only be applied to maps loaded from a file.
   */
  public autoMap(region: region | rect, rulesFile?: string): void;

  /**
   * Sets the size of the map in tiles. This does not affect the contents of the map.
   *
   * See also {@link resize}.
   */
  public setSize(width: number, height: number): void;

  /**
   * Sets the tile size of the map in pixels. This affects the rendering of all tile layers.
   */
  public setTileSize(width: number, height: number): void;

  /**
   * Returns a reference to the top-level layer at the given index. When the layer gets removed from the map, the reference changes to a standalone copy of the layer.
   */
  public layerAt(index: number): Layer;

  /**
   * Removes the top-level layer at the given index. When a reference to the layer still exists, that reference becomes a standalone copy of the layer.
   */
  public removeLayerAt(index: number): void;

  /**
   * Removes the given layer from the map. The reference to the layer becomes a standalone copy.
   */
  public removeLayer(layer: Layer): void;

  /**
   * Inserts the layer at the given index. The layer can’t already be part of a map.
   */
  public insertLayerAt(index: number, layer: Layer): void;

  /**
   * Adds the layer to the map, above all existing layers. The layer can’t already be part of a map.
   */
  public addLayer(layer: Layer): void;

  /**
   * Adds the given tileset to the list of tilesets referenced by this map. Returns true if the tileset was added, or false if the tileset was already referenced by this map.
   */
  public addTileset(tileset: Tileset): boolean;

  /**
   * Replaces all occurrences of oldTileset with newTileset. Returns true on success, or false when either the old tileset was not referenced by the map, or when the new tileset was already referenced by the map.
   */
  public replaceTileset(oldTileset: Tileset, newTileset: Tileset): boolean;

  /**
   * Removes the given tileset from the list of tilesets referenced by this map. Returns true on success, or false when the given tileset was not referenced by this map or when the tileset was still in use by a tile layer or tile object.
   */
  public removeTileset(tileset: Tileset): boolean;

  /**
   * Returns the list of tilesets actually used by this map. This is generally a subset of the tilesets referenced by the map (the TileMap.tilesets property).
   */
  public usedTilesets(): Tileset[];

  /**
   * Removes the given objects from this map. The object references turn into a standalone copy of the object.
   */
  public removeObjects(objects : MapObject[]);

  /**
   * Merges the tile layers in the given map with this one. If only a single tile layer exists in the given map, it will be merged with the currentLayer.
   *
   * This operation can currently only be applied to maps loaded from a file.
   *
   * If `canJoin` is true, the operation joins with the previous one on the undo stack when possible. Useful for reducing the amount of undo commands.
   */
  public merge(map: TileMap, canJoin?: boolean): void;

  /**
   * Resizes the map to the given size, optionally applying an offset (in tiles).
   *
   * This operation can currently only be applied to maps loaded from a file.
   *
   * See also {@link setSize}.
   */
  public resize(size: size, offset?: point, removeObjects?: boolean): void;

  /**
   * Converts the given position from screen to tile coordinates.
   */
  public screenToTile(x: number, y: number): point;

  /**
   * Converts the given position from screen to tile coordinates.
   */
  public screenToTile(position: point): point;

  /**
   * Converts the given position from tile to screen coordinates.
   */
  public tileToScreen(x: number, y: number): point;

  /**
   * Converts the given position from tile to screen coordinates.
   */
  public tileToScreen(position: point): point;

  /**
   * Converts the given position from screen to pixel coordinates.
   */
  public screenToPixel(x: number, y: number): point;

  /**
   * Converts the given position from screen to pixel coordinates.
   */
  public screenToPixel(position: point): point;

  /**
   * Converts the given position from pixel to screen coordinates.
   */
  public pixelToScreen(x: number, y: number): point;

  /**
   * Converts the given position from pixel to screen coordinates.
   */
  public pixelToScreen(position: point): point;

  /**
   * Converts the given position from pixel to tile coordinates.
   */
  public pixelToTile(x: number, y: number): point;

  /**
   * Converts the given position from pixel to tile coordinates.
   */
  public pixelToTile(position: point): point;

  /**
   * Converts the given position from tile to pixel coordinates.
   */
  public tileToPixel(x: number, y: number): point;

  /**
   * Converts the given position from tile to pixel coordinates.
   */
  public tileToPixel(position: point): point;
}

/**
 * A cell on a {@link TileLayer}.
 */
interface cell {
  /**
   * The local tile ID of the tile, or -1 if the cell is empty.
   */
  tileId : number

  /**
   * Whether the cell is empty.
   */
  empty : boolean

  /**
   * Whether the tile is flipped horizontally.
   */
  flippedHorizontally : boolean

  /**
   * Whether the tile is flipped vertically.
   */
  flippedVertically : boolean

  /**
   * Whether the tile is flipped anti-diagonally.
   */
  flippedAntiDiagonally : boolean

  /**
   * Whether the tile is rotated by 120 degrees (for hexagonal maps, the anti-diagonal flip is interpreted as a 60-degree rotation).
   */
  rotatedHexagonal120 : boolean
}

/**
 * A tile layer.
 *
 * Note that while tile layers have a size, the size is generally ignored on
 * infinite maps. Even for fixed size maps, nothing in the scripting API stops you
 * from changing the layer outside of its boundaries and changing the size of the
 * layer has no effect on its contents. If you want to change the size while
 * affecting the contents, use the {@link resize} function.
 */
declare class TileLayer extends Layer {
  /**
   * Width of the layer in tiles (only relevant for non-infinite maps).
   */
  width : number

  /**
   * Height of the layer in tiles (only relevant for non-infinite maps).
   */
  height : number

  /**
   * Size of the layer in tiles (only relevant for non-infinite maps).
   */
  size : size

  /**
   * Constructs a new tile layer, which can be added to a {@link TileMap}.
   */
  constructor(name? : string)

  /**
   * Returns the region of the layer that is covered with tiles.
   */
  region() : region

  /**
   * Resizes the layer, erasing the part of the contents that falls outside of the layer’s new size.
   * The offset parameter can be used to shift the contents by a certain distance in tiles before applying the resize.
   */
  resize(size : size, offset : point) : void

  /**
   * Returns the value of the cell at the given position. Can be used to query the flags and the tile ID, but does not currently allow getting a tile reference (see {@link tileAt}).
   */
  cellAt(x : number, y : number) : cell

  /**
   * Returns the flags used for the tile at the given position.
   */
  flagsAt(x : number, y : number) : number

  /**
   * Returns the tile used at the given position, or null for empty spaces.
   */
  tileAt(x : number, y : number) : Tile | null

  /**
   * Returns an object that enables making modifications to the tile layer.
   */
  edit() : TileLayerEdit
}

/**
 * This object enables modifying the tiles on a tile layer. Tile layers can't be
 * modified directly for reasons of efficiency. The {@link apply}
 * function needs to be called when you're done making changes.
 *
 * An instance of this object is created by calling {@link TileLayer.edit}.
 */
interface TileLayerEdit {
  /**
   * The target layer of this edit object.
   */
  readonly target : TileLayer

  /**
   * Whether applied edits are mergeable with previous edits. Starts out as false and is automatically set to true by {@link apply}.
   */
  mergeable : boolean

  /**
   * Sets the tile at the given location, optionally specifying tile flags.
   */
  setTile(x : number, y : number, tile : Tile , flags? : number) : void

  /**
   * Applies all changes made through this object. This object can be reused to make further changes.
   */
  apply() : void
}

/**
 * @since 1.5
 */
declare class WangSet {
  static readonly Edge: unique symbol;
  static readonly Corner: unique symbol;
  static readonly Mixed: unique symbol;

  /**
   * Name of the Wang set.
   */
  name : string

  /**
   * Type of the Wang set.
   */
  type : typeof WangSet.Edge | typeof WangSet.Corner | typeof WangSet.Mixed;

  /**
   * The tile used to represent the Wang set.
   */
  imageTile : Tile

  /**
   * The number of colors used by this Wang set.
   */
  colorCount : number

  /**
   * The tileset to which this Wang set belongs.
   */
  readonly tileset : Tileset

  /**
   * Returns the current Wang ID associated with the given tile.
   *
   * The Wang ID is given by an array of 8 numbers, indicating the colors associated with each index in the following order: [Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, TopLeft].
   * A value of 0 indicates that no color is associated with a given index.
   */
  public wangId(tile : Tile) : number[]

  /**
   * Sets the Wang ID associated with the given tile.
   *
   * The Wang ID is given by an array of 8 numbers, indicating the colors associated with each index in the following order: [Top, TopRight, Right, BottomRight, Bottom, BottomLeft, Left, TopLeft].
   * A value of 0 indicates that no color is associated with a given index.
   *
   * Make sure the Wang set color count is set before calling this function, because it will raise an error when the Wang ID refers to non-existing colors.
   */
  public setWangId(tile : Tile, wangId : number[]) : void
}

interface color {}

declare class Tileset extends Asset {
  static readonly Unspecified: unique symbol
  static readonly TopLeft: unique symbol
  static readonly Top: unique symbol
  static readonly TopRight: unique symbol
  static readonly Left: unique symbol
  static readonly Center: unique symbol
  static readonly Right: unique symbol
  static readonly BottomLeft: unique symbol
  static readonly Bottom: unique symbol
  static readonly BottomRight: unique symbol

  static readonly Orthogonal: unique symbol
  static readonly Isometric: unique symbol

  /**
   * Name of the tileset.
   */
  name : string

  /**
   * The file name of the image used by this tileset. Empty in case of image collection tilesets.
   */
  image : string

  /**
   * Array of all tiles in this tileset. Note that the index of a tile in this array does not always match with its ID.
   */
  readonly tiles : Tile[]

  /**
   * Array of all Wang sets in this tileset.
   */
  readonly wangSets : WangSet[]

  /**
   * The number of tiles in this tileset.
   */
  tileCount : number

  /**
   * The ID of the next tile that would be added to this tileset. All existing tiles have IDs that are lower than this ID.
   */
  nextTileId : number

  /**
   * Tile width for tiles in this tileset in pixels.
   */
  tileWidth : number

  /**
   * Tile Height for tiles in this tileset in pixels.
   */
  tileHeight : number

  /**
   * Tile size for tiles in this tileset in pixels.
   */
  tileSize : size

  /**
   * Width of the tileset image in pixels.
   */
  readonly imageWidth : number

  /**
   * Height of the tileset image in pixels.
   */
  readonly imageHeight : number

  /**
   * Size of the tileset image in pixels.
   */
  readonly imageSize : size

  /**
   * Spacing between tiles in this tileset in pixels.
   */
  readonly tileSpacing : number

  /**
   * Margin around the tileset in pixels (only used at the top and left sides of the tileset image).
   */
  readonly margin : number

  /**
   * The alignment to use for tile objects (when Unspecified, uses Bottom alignment on isometric maps and BottomLeft alignment for all other maps).
   */
  objectAlignment : typeof Tileset.Unspecified | typeof Tileset.TopLeft | typeof Tileset.Top | typeof Tileset.TopRight | typeof Tileset.Left | typeof Tileset.Center | typeof Tileset.Right | typeof Tileset.BottomLeft | typeof Tileset.Bottom | typeof Tileset.BottomRight

  /**
   * Offset in pixels that is applied when tiles from this tileset are rendered.
   */
  tileOffset : point

  /**
   * The orientation of this tileset (used when rendering overlays and in the tile collision editor).
   */
  orientation : typeof Tileset.Orthogonal | typeof Tileset.Isometric

  /**
   * Background color for this tileset in the Tilesets view.
   */
  backgroundColor : color

  /**
   * Whether this tileset is a collection of images (same as checking whether image is an empty string).
   */
  readonly isCollection : boolean

  /**
   * Selected tiles (in the tileset editor).
   */
  selectedTiles : Tile[]

  /**
   * Constructs a new Tileset.
   */
  constructor(name? : string)

  /**
   * Returns a reference to the tile with the given ID. Raises an error if no such tile exists. When the tile gets removed from the tileset, the reference changes to a standalone copy of the tile.
   *
   * Note that the tiles in a tileset are only guaranteed to have consecutive IDs for tileset-image based tilesets. For image collection tilesets there will be gaps when tiles have been removed from the tileset.
   */
  public tile(id : number) : Tile

  /**
   * Sets the tile size for this tileset. If an image has been specified as well, the tileset will be (re)loaded. Can’t be used on image collection tilesets.
   */
  public setTileSize(width : number, height : number) : void

  /**
   * Creates the tiles in this tileset by cutting them out of the given image, using the current tile size, tile spacing and margin parameters. These values should be set before calling this function.
   *
   * Optionally sets the source file of the image. This may be useful, but be careful since Tiled will try to reload the tileset from that source when the tileset parameters are changed.
   *
   * @warning This function has no undo!
   */
  public loadFromImage(image : Image, source?: string) : void

  /**
   * Adds a new tile to this tileset and returns it. Only works for image collection tilesets.
   */
  public addTile() : Tile

  /**
   * Removes the given tiles from this tileset. Only works for image collection tilesets.
   */
  public removeTiles(tiles : Tile[]) : void

  /**
   * Add a new Wang set to this tileset with the given name and type.
   */
  public addWangSet(name : string, type : number) : WangSet

  /**
   * Removes the given Wang set from this tileset.
   */
  public removeWangSet(wangSet : WangSet) : void
}

/**
 * The interface that should be implemented for objects passed to {@link tiled.registerTilesetFormat}.
 */
interface ScriptedTilesetFormat {
  /**
   * Name of the format as shown in the file dialog.
   */
  readonly name: string;

  /**
   * The file extension used by the format.
   */
  readonly extension: string;

  /**
   * A function that reads a tileset from the given file.
   *
   * Can use {@link TextFile} or {@link BinaryFile} to read the file.
   */
  read?(fileName: string): Tileset;

  /**
   * A function that writes a tileset to the given file.
   *
   * Can use {@link TextFile} or {@link BinaryFile} to write the file.
   * When a non-empty string is returned, it is shown as error message.
   */
  write?(tileset: Tileset, fileName: string) : string | undefined;
}

/**
 * The view displaying the map.
 */
interface MapView {
  /**
   * The scale of the view.
   */
  scale : number

  /**
   * The center of the view.
   */
  center : point

  /**
   * Centers the view at the given location in screen coordinates. Same as assigning to the center property.
   */
  centerOn(x : number, y : number) : void
}

interface TileCollisionEditor {
  /**
   * Selected objects.
   */
  selectedObjects : MapObject[]

  /**
   * The map view used by the Collision Editor.
   */
  view : MapView

  /**
   * Focuses the given object in the collision editor view and makes sure its
   * visible in its objects list. Does not automatically select the object.
   */
  focusObject(object : MapObject) : void
}

interface TilesetEditor {
  /**
   * Access the collision editor within the tileset editor.
   */
  collisionEditor : TileCollisionEditor
}

interface Tool {
  /**
   * Name of the tool as shown on the tool bar.
   */
  name: string;

  /**
   * File name of an icon. If set, the icon is shown on the tool bar and the name becomes the tool tip.
   */
  icon: string;

  /**
   * Currently active tile map.
   */
  map: TileMap;

  /**
   * The last clicked tile for the active map. See also the {@link MapEditor.currentBrush} property.
   */
  selectedTile: any;

  /**
   * Get or set the preview for tile layer edits.
   */
  preview: TileMap;

  /**
   * Mouse cursor position in tile coordinates.
   */
  tilePosition: point;

  /**
   * Text shown in the status bar while the tool is active.
   */
  statusInfo: string;

  /**
   * Whether this tool is enabled.
   */
  enabled: boolean;

  /**
   * Called when the tool was activated.
   */
  activated(): void;

  /**
   * Called when the tool was deactivated.
   */
  deactivated(): void;

  /**
   * Called when a key was pressed while the tool was active.
   */
  keyPressed(key:string, modifiers:any): void;

  /**
   * Called when the mouse entered the map view.
   */
  mouseEntered(): void;

  /**
   * Called when the mouse left the map view.
   */
  mouseLeft(): void;

  /**
   * Called when the mouse position in the map scene changed.
   */
  mouseMoved(x: number, y: number, modifiers:any): void;

  /**
   * Called when a mouse button was pressed.
   */
  mousePressed(button:any, x: number, y: number, modifiers:any): void;

  /**
   * Called when a mouse button was released.
   */
  mouseReleased(button:any, x: number, y: number, modifiers:any): void;

  /**
   * Called when a mouse button was double-clicked.
   */
  mouseDoubleClicked(button:any, x: number, y: number, modifiers:any): void;

  /**
   * Called when the active modifier keys changed.
   */
  modifiersChanged(modifiers:any): void;

  /**
   * Called when the language was changed.
   */
  languageChanged(): void;

  /**
   * Called when the active map was changed.
   */
  mapChanged(oldMap: TileMap, newMap: TileMap): void;

  /**
   * Called when the hovered tile position changed.
   */
  tilePositionChanged(): void;

  /**
   * Called when the hovered tile position changed.
   *
   * Used to override the default updating of the status bar text.
   */
  updateStatusInfo(): void;

  /**
   * Called when the map or the current layer changed.
   */
  updateEnabledState(): void;
}

/**
 * The ``tiled`` module is the main entry point and provides properties,
 * functions and signals which are documented below.
 */
declare namespace tiled {
  /**
   * Currently used version of Tiled.
   */
  export const version: string;

  /**
   * Operating system. One of `windows`, `macos`, `linux` or
   * `unix` (for any other UNIX-like system).
   */
  export const platform: string;

  /**
   * Processor architecture. One of `x64`, `x86` or `unknown`.
   */
  export const arch: string;

  /**
   * The system- and user-specific path where global extensions are installed.
   *
   * Note that normally it is not necessary to use this path, since the "ext:"
   * prefix can be used to refer to files shipping with extensions. Also, there
   * is a `__filename` property containing the full file path of the currently
   * evaluated file.
   *
   * Also note that a Tiled project can have its own additional extensions
   * directory, to make it easier to share extensions with a team or keep them
   * under version control.
   */
  export const extensionsPath: string;

  /**
   * Available actions for {@link trigger | tiled.trigger()}.
   */
  export const actions: string[];

  /**
   * Available menus for {@link extendMenu | tiled.extendMenu()}.
   */
  export const menus: string[];

  /**
   * Currently selected asset, or `null` if no file is open.
   * Can be assigned any open asset in order to change the active asset.
   */
  export let activeAsset: Asset;

  /**
   * List of currently opened {@link Asset | assets}.
   */
  export const openAssets: Asset[];

  /**
   * List of supported tileset format names. Use {@link tilesetFormat}
   * to get the corresponding format object to read and write files.
   *
   * @since 1.4
   */
  export const tilesetFormats: string[];

  /**
   * List of supported map format names. Use {@link mapFormat} to get
   * the corresponding format object to read and write files.
   *
   * @since 1.4
   */
  export const mapFormats: string[];

  /**
   * Access the editor used when editing maps.
   */
  export const mapEditor: MapEditor;

  /**
   * Access the editor used when editing tilesets.
   */
  export const tilesetEditor: TilesetEditor;

  /**
   * This function can be used to trigger any registered action. This
   * includes most actions you would normally trigger through the menu or
   * by using their shortcut.
   *
   * Use the {@link actions | tiled.actions} property to get a list
   * of all available actions.
   *
   * Actions that are checkable will toggle when triggered.
   */
  export function trigger(action: string): void;

  /**
   * Executes the first custom command with the given name, as if it was
   * triggered manually. Works also with commands that are not currently
   * enabled.
   *
   * Raises a script error if the command is not found.
   *
   * For more control over the executed binary, use {@link Process} instead.
   */
  export function executeCommand(name: string, inTerminal: boolean): void;

  /**
   * Requests to open the asset with the given file name. Returns a
   * reference to the opened asset, or `null` in case there was a
   * problem.
   */
  export function open(fileName: string): Asset | null;

  /**
   * Closes the given asset without checking for unsaved changes (to
   * confirm the loss of any unsaved changes, set {@link activeAsset} and
   * trigger the "Close" action instead).
   */
  export function close(asset: Asset): boolean;

  /**
   * Reloads the given asset from disk, without checking for unsaved
   * changes. This invalidates the previous script reference to the
   * asset, hence the new reference is returned for convenience. Returns
   * `null` if reloading failed.
   */
  export function reload(asset: Asset): Asset | null;

  /**
   * Shows a modal warning dialog to the user with the given text and
   * optional title.
   */
  export function alert(text: string, title?: string): void;

  /**
   * Shows a yes/no dialog to the user with the given text and optional
    title. Returns `true` or `false`.
   */
  export function confirm(text: string, title?: string): boolean;

  /**
   * Shows a dialog that asks the user to enter some text, along with the
   * given label and optional title. The optional `text` parameter
   * provides the initial value of the text. Returns the entered text.
   */
  export function prompt(label: string, text?: string, title?: string): string;

  /**
   * Outputs the given text in the Console window as regular text.
   */
  export function log(text: string): void;

  /**
   * Outputs the given text in the Console window as warning message and
   * creates an issue in the Issues window.
   *
   * When the issue is activated (with double-click or Enter key) the
   * given callback function is invoked.
   */
  export function warn(text: string, activated: () => void): void;

  /**
   * Outputs the given text in the Console window as error message and
   * creates an issue in the Issues window.
   *
   * When the issue is activated (with double-click or Enter key) the
   * given callback function is invoked.
   */
  export function error(text: string, activated: () => void): void;

  /**
   * Extends the menu with the given ID. Supports both a list of items or
   * a single item. Available menu IDs can be obtained using the
   * {@link tiled.menus} property.
   *
   * If a menu item does not include a `before` property, the value is
   * inherited from the previous item. When this property is not set at
   * all, the items are appended to the end of the menu.
   *
   * Example that adds a custom action to the "Edit" menu, before the
   * "Select All" action and separated by a separator:
   *
   * ```js
   * tiled.extendMenu("Edit", [
   *     { action: "CustomAction", before: "SelectAll" },
   *     { separator: true }
   * ]);
   * ```
   *
   * The "CustomAction" will need to have been registered before using
   * {@link registerAction | tiled.registerAction()}.
   */
  export function extendMenu(
    shortName: string,
    menu: Menu[]
  ): void;

  /**
   * Registers a new action with the given `id` and `callback` (which is
   * called when the action is triggered). The returned action object can
   * be used to set (and update) various properties of the action.
   *
   * The shortcut will currently only work when the action is added to a
   * menu using {@link extendMenu | tiled.extendMenu()}.
   *
   * @example
   * ```js
   * var action = tiled.registerAction("CustomAction", function(action) {
   *     tiled.log(action.text + " was " + (action.checked ? "checked" : "unchecked"))
   * })
   *
   * action.text = "My Custom Action"
   * action.checkable = true
   * action.shortcut = "Ctrl+K"
   * ```
   */
  export function registerAction(
    id: string,
    callback: (action: Action) => void
  ): Action;

  /**
   * Registers a custom tool that will become available on the Tools tool
   * bar of the Map Editor.
   *
   * If a tool is already registered with the same `shortName` the
   * existing tool is replaced.
   *
   * @example
   * Here is an example tool that places a rectangle each time the mouse
   * has moved by 32 pixels:
   *
   * ```js
   * var tool = tiled.registerTool("PlaceRectangles", {
   *     name: "Place Rectangles",
   *
   *     mouseMoved: function(x, y, modifiers) {
   *         if (!this.pressed)
   *             return
   *
   *         var dx = Math.abs(this.x - x)
   *         var dy = Math.abs(this.y - y)
   *
   *         this.distance += Math.sqrt(dx*dx + dy*dy)
   *         this.x = x
   *         this.y = y
   *
   *         if (this.distance > 32) {
   *             var objectLayer = this.map.currentLayer
   *
   *             if (objectLayer && objectLayer.isObjectLayer) {
   *                 var object = new MapObject(++this.counter)
   *                 object.x = Math.min(this.lastX, x)
   *                 object.y = Math.min(this.lastY, y)
   *                 object.width = Math.abs(this.lastX - x)
   *                 object.height = Math.abs(this.lastY - y)
   *                 objectLayer.addObject(object)
   *             }
   *
   *             this.distance = 0
   *             this.lastX = x
   *             this.lastY = y
   *         }
   *     },
   *
   *     mousePressed: function(button, x, y, modifiers) {
   *         this.pressed = true
   *         this.x = x
   *         this.y = y
   *         this.distance = 0
   *         this.counter = 0
   *         this.lastX = x
   *         this.lastY = y
   *     },
   *
   *     mouseReleased: function(button, x, y, modifiers) {
   *         this.pressed = false
   *     },
   * })
   * ```
   */
  export function registerTool(shortName: string, tool: Tool): Tool;

  /**
   * Returns the tileset format object with the given name, or
    `undefined` if no object was found. See the
    {@link tilesetFormats} property for more info.
   */
  export function tilesetFormat(shortName: string): TilesetFormat;

  /**
   * Returns the tileset format object that can read the given file, or
    `undefined` if no object was found.
   */
  export function tilesetFormatForFile(fileName: string): TilesetFormat;

  /**
   * Returns the map format object with the given name, or
   * `undefined` if no object was found. See the
   * {@link mapFormats} property for more info.
   */
  export function mapFormat(shortName: string): MapFormat;

  /**
   * Returns the map format object that can read the given file, or
   * `undefined` if no object was found.
   */
  export function mapFormatForFile(fileName: string): MapFormat;

  /**
   * Creates a {@link FilePath} object with the given URL.
   */
  export function filePath(path: string): FilePath;

  /**
   * Creates an {@link ObjectRef} object with the given ID.
   */
  export function objectRef(id: number): ObjectRef;

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
   * ```js
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
    mapFormat: ScriptedMapFormat
  ): void;

  /**
   * Like {@link registerMapFormat}, but registers a custom tileset
   * format instead.
   */
  export function registerTilesetFormat(
    shortName: string,
    tilesetFormat: ScriptedTilesetFormat
  ): void;

  /**
   * A new asset has been created.
   */
  export const assetCreated: Signal<Asset>;

  /**
   * An asset has been opened.
   */
  export const assetOpened: Signal<Asset>;

  /**
   * An asset is about to be saved. Can be used to make last-minute
   * changes.
   */
  export const assetAboutToBeSaved: Signal<Asset>;

  /**
   * An asset has been saved.
   */
  export const assetSaved: Signal<Asset>;

  /**
   * An asset is about to be closed.
   */
  export const assetAboutToBeClosed: Signal<Asset>;

  /**
   * The currently active asset has changed.
   */
  export const activeAssetChanged: Signal<Asset>;
}

/**
 * The Process class allows you to start processes, track their output, and so on.
 *
 * @since 1.5
 */
declare class Process {
  /**
   * The directory the process will be started in. This only has an effect if set before the process is started.
   */
  workingDirectory : string

  /**
   * True if there is no more data to be read from the process output, otherwise false.
   */
  readonly atEnd : boolean

  /**
   * The exit code of the process. This is needed for retrieving the exit code from processes started via start(), rather than exec().
   */
  readonly exitCode : number

  /**
   * Sets the text codec to codec. The codec is used for reading and writing from and to the process, respectively. Common codecs are supported, for example: “UTF-8”, “UTF-16”, and “ISO 8859-1”.
   */
  codec: string

  /**
   * Allocates and returns a new Process object.
   */
  constructor()

  /**
   * Frees the resources associated with the process. It is recommended to always call this function as soon as you are finished with the process.
   */
  close() : void

  /**
   * Schedules the stdin channel of process to be closed. The channel will close once all data has been written to the process. After calling this function, any attempts to write to the process will do nothing.
   */
  closeWriteChannel() : void

  /**
   * Executes the program at filePath with the given argument list and blocks until the process is finished. If an error occurs (for example, there is no executable file at filePath) and throwOnError is true (the default), then a JavaScript exception will be thrown. Otherwise, -1 will be returned in case of an error. The normal return code is the exit code of the process.
   */
  exec(filePath : string, arguments : string[] , throwOnError? : boolean) : number

  /**
   * Returns the value of the variable varName in the process’ environment.
   */
  getEnv(name : string) : string

  /**
   * Kills the process, causing it to exit immediately.
   */
  kill() : void

  /**
   * Reads and returns one line of text from the process output, without the newline character(s).
   */
  readLine() : string

  /**
   * Reads and returns all data from the process’ standard error channel.
   */
  readStdErr() : string

  /**
   * Reads and returns all data from the process’ standard output channel.
   */
  readStdOut() : string

  /**
   * Sets the value of variable varName to varValue in the process environment. This only has an effect if called before the process is started.
   */
  setEnv(varName : string, varValue : string) : string

  /**
   * Starts the program at filePath with the given list of arguments. Returns true if the process could be started and false otherwise.
   *
   * Note: This call returns right after starting the process and should be used only if you need to interact with the process while it is running. Most of the time, you want to use exec() instead.
   */
  start(filePath : string, arguments : string[]) : boolean

  /**
   * Tries to terminate the process. This is not guaranteed to make the process exit immediately; if you need that, use kill().
   */
  terminate() : void

  /**
   * Blocks until the process has finished or timeout milliseconds have passed (default is 30000). Returns true if the process has finished and false if the operation has timed out. Calling this function only makes sense for processes started via start() (as opposed to exec()).
   */
  waitForFinished( msecs?:number ) : boolean

  /**
   * Writes text into the process’ input channel.
   */
  write(text : string) : void

  /**
   * Writes text, followed by a newline character, into the process’ input channel.
   */
  writeLine(text : string) : void
}

/// <reference types="@mapeditor/tiled-api" />

/**
 * Tiled MCP Bridge Extension
 *
 * This extension polls a commands.json file for commands from the MCP server
 * and executes them in real-time within Tiled.
 */

// Configuration - UPDATE THIS PATH to your mcp-server location
// Option 1: Hardcoded absolute path (most reliable)
// const COMMAND_FILE = "/path/to/tiled/mcp-server/commands.json";
// Option 2: Relative to extensions folder (if symlinked)
const COMMAND_FILE = tiled.extensionsPath + "/tiled-mcp-bridge/commands.json";
const POLL_INTERVAL = 100; // milliseconds

let lastModified = null;
let isProcessing = false;

// Log startup
tiled.log("MCP Bridge: Starting up...");
tiled.log("MCP Bridge: Command file: " + COMMAND_FILE);

/**
 * Check if the command file has been modified and process new commands.
 */
function checkForCommands() {
    if (isProcessing) return;

    try {
        if (!File.exists(COMMAND_FILE)) {
            return;
        }

        const modified = File.lastModified(COMMAND_FILE);
        const modifiedStr = modified ? modified.toString() : null;

        if (modifiedStr !== lastModified) {
            lastModified = modifiedStr;
            processCommands();
        }
    } catch (e) {
        tiled.error("MCP Bridge: Error checking commands: " + e);
    }
}

/**
 * Read and process all commands from the command file.
 */
function processCommands() {
    isProcessing = true;

    try {
        const file = new TextFile(COMMAND_FILE, TextFile.ReadOnly);
        const content = file.readAll();
        file.close();

        if (!content || content.trim() === "" || content.trim() === "[]") {
            isProcessing = false;
            return;
        }

        const commands = JSON.parse(content);

        if (!Array.isArray(commands) || commands.length === 0) {
            isProcessing = false;
            return;
        }

        tiled.log("MCP Bridge: Processing " + commands.length + " command(s)");

        for (const cmd of commands) {
            try {
                executeCommand(cmd);
            } catch (e) {
                tiled.error("MCP Bridge: Error executing command " + cmd.type + ": " + e);
            }
        }

        // Clear the command file after processing
        const clearFile = new TextFile(COMMAND_FILE, TextFile.WriteOnly);
        clearFile.write("[]");
        clearFile.close();

    } catch (e) {
        tiled.error("MCP Bridge: Error processing commands: " + e);
    }

    isProcessing = false;
}

/**
 * Execute a single command.
 */
function executeCommand(cmd) {
    const type = cmd.type;
    const params = cmd.params || {};

    tiled.log("MCP Bridge: Executing " + type);

    switch (type) {
        case "create_map":
            createMap(params);
            break;

        case "resize_map":
            resizeMap(params);
            break;

        case "add_tile_layer":
            addTileLayer(params);
            break;

        case "add_object_layer":
            addObjectLayer(params);
            break;

        case "remove_layer":
            removeLayer(params);
            break;

        case "set_current_layer":
            setCurrentLayer(params);
            break;

        case "set_tile":
            setTile(params);
            break;

        case "set_tiles":
            setTiles(params);
            break;

        case "fill_area":
            fillArea(params);
            break;

        case "add_object":
            addObject(params);
            break;

        case "add_tileset":
            addTileset(params);
            break;

        case "generate_tileset":
            generateTileset(params);
            break;

        case "reload_map":
            reloadMap(params);
            break;

        default:
            tiled.warn("MCP Bridge: Unknown command type: " + type);
    }
}

// =============================================================================
// Command Implementations
// =============================================================================

function getActiveMap() {
    const asset = tiled.activeAsset;
    if (asset && asset.isTileMap) {
        return asset;
    }
    return null;
}

function createMap(params) {
    const width = params.width || 20;
    const height = params.height || 15;
    const tileWidth = params.tile_width || 32;
    const tileHeight = params.tile_height || 32;

    // Create a new map
    const map = new TileMap();
    map.setSize(width, height);
    map.setTileSize(tileWidth, tileHeight);

    // Set orientation
    if (params.orientation === "isometric") {
        map.orientation = TileMap.Isometric;
    } else if (params.orientation === "staggered") {
        map.orientation = TileMap.Staggered;
    } else if (params.orientation === "hexagonal") {
        map.orientation = TileMap.Hexagonal;
    } else {
        map.orientation = TileMap.Orthogonal;
    }

    // Add a default ground layer
    const layer = new TileLayer("Ground");
    layer.width = width;
    layer.height = height;
    map.addLayer(layer);

    // Open the map in Tiled
    tiled.activeAsset = map;

    tiled.log("MCP Bridge: Created " + width + "x" + height + " map");
}

function resizeMap(params) {
    const map = getActiveMap();
    if (!map) {
        tiled.warn("MCP Bridge: No active map for resize");
        return;
    }

    map.macro("Resize Map", function() {
        map.setSize(params.width, params.height);
    });
}

function addTileLayer(params) {
    const map = getActiveMap();
    if (!map) {
        tiled.warn("MCP Bridge: No active map for add_tile_layer");
        return;
    }

    map.macro("Add Tile Layer", function() {
        const layer = new TileLayer(params.name);
        layer.width = map.width;
        layer.height = map.height;
        map.addLayer(layer);
    });

    tiled.log("MCP Bridge: Added tile layer '" + params.name + "'");
}

function addObjectLayer(params) {
    const map = getActiveMap();
    if (!map) {
        tiled.warn("MCP Bridge: No active map for add_object_layer");
        return;
    }

    map.macro("Add Object Layer", function() {
        const layer = new ObjectGroup(params.name);
        map.addLayer(layer);
    });

    tiled.log("MCP Bridge: Added object layer '" + params.name + "'");
}

function removeLayer(params) {
    const map = getActiveMap();
    if (!map) return;

    map.macro("Remove Layer", function() {
        for (let i = 0; i < map.layerCount; i++) {
            const layer = map.layerAt(i);
            if (layer.name === params.name) {
                map.removeLayerAt(i);
                tiled.log("MCP Bridge: Removed layer '" + params.name + "'");
                return;
            }
        }
    });
}

function setCurrentLayer(params) {
    const map = getActiveMap();
    if (!map) return;

    for (let i = 0; i < map.layerCount; i++) {
        const layer = map.layerAt(i);
        if (layer.name === params.name) {
            map.currentLayer = layer;
            tiled.log("MCP Bridge: Set current layer to '" + params.name + "'");
            return;
        }
    }
}

function findTileLayer(map, name) {
    for (let i = 0; i < map.layerCount; i++) {
        const layer = map.layerAt(i);
        if (layer.name === name && layer.isTileLayer) {
            return layer;
        }
    }
    return null;
}

function setTile(params) {
    const map = getActiveMap();
    if (!map) return;

    const layer = findTileLayer(map, params.layer);
    if (!layer) {
        tiled.warn("MCP Bridge: Layer not found: " + params.layer);
        return;
    }

    // Create edit for the layer
    const edit = layer.edit();
    edit.setTile(params.x, params.y, map.tileForGid(params.tile_id));
    edit.apply();
}

function setTiles(params) {
    const map = getActiveMap();
    if (!map) return;

    const layer = findTileLayer(map, params.layer);
    if (!layer) {
        tiled.warn("MCP Bridge: Layer not found: " + params.layer);
        return;
    }

    const tiles = params.tiles;
    if (!tiles || tiles.length === 0) return;

    // Batch edit for performance
    const edit = layer.edit();
    for (const t of tiles) {
        const tile = map.tileForGid(t.tile_id);
        edit.setTile(t.x, t.y, tile);
    }
    edit.apply();

    tiled.log("MCP Bridge: Set " + tiles.length + " tiles on layer '" + params.layer + "'");
}

function fillArea(params) {
    const map = getActiveMap();
    if (!map) return;

    const layer = findTileLayer(map, params.layer);
    if (!layer) {
        tiled.warn("MCP Bridge: Layer not found: " + params.layer);
        return;
    }

    const tile = map.tileForGid(params.tile_id);
    const edit = layer.edit();

    for (let y = params.y; y < params.y + params.height; y++) {
        for (let x = params.x; x < params.x + params.width; x++) {
            edit.setTile(x, y, tile);
        }
    }
    edit.apply();

    tiled.log("MCP Bridge: Filled " + params.width + "x" + params.height +
              " area at (" + params.x + ", " + params.y + ")");
}

function addObject(params) {
    const map = getActiveMap();
    if (!map) return;

    // Find the object layer
    let objLayer = null;
    for (let i = 0; i < map.layerCount; i++) {
        const layer = map.layerAt(i);
        if (layer.name === params.layer && layer.isObjectGroup) {
            objLayer = layer;
            break;
        }
    }

    if (!objLayer) {
        tiled.warn("MCP Bridge: Object layer not found: " + params.layer);
        return;
    }

    const obj = params.object;
    const mapObject = new MapObject(obj.name || "");
    mapObject.x = obj.x;
    mapObject.y = obj.y;
    mapObject.width = obj.width || 0;
    mapObject.height = obj.height || 0;

    objLayer.addObject(mapObject);
    tiled.log("MCP Bridge: Added object '" + obj.name + "' to layer '" + params.layer + "'");
}

function addTileset(params) {
    const map = getActiveMap();
    if (!map) return;

    const ts = params.tileset;

    try {
        // Try to load the tileset
        const tileset = tiled.open(ts.source);
        if (tileset && tileset.isTileset) {
            map.addTileset(tileset);
            tiled.log("MCP Bridge: Added tileset '" + ts.name + "' from " + ts.source);
        }
    } catch (e) {
        tiled.warn("MCP Bridge: Could not load tileset: " + ts.source + " - " + e);
    }
}

function generateTileset(params) {
    const map = getActiveMap();
    if (!map) return;

    const colors = params.colors;
    const tileSize = params.tile_size || 32;
    const name = params.name || "generated";
    const firstGid = params.first_gid || 1;

    // Create a tileset image with colored tiles
    const numTiles = colors.length;
    const imageWidth = numTiles * tileSize;
    const imageHeight = tileSize;

    // Create image
    const image = new Image(imageWidth, imageHeight);

    // Fill each tile with its color
    for (let i = 0; i < numTiles; i++) {
        const color = colors[i];
        // Parse hex color
        let r = 128, g = 128, b = 128;
        if (color && color.startsWith("#")) {
            const hex = color.substring(1);
            if (hex.length === 6) {
                r = parseInt(hex.substring(0, 2), 16);
                g = parseInt(hex.substring(2, 4), 16);
                b = parseInt(hex.substring(4, 6), 16);
            }
        }

        // Fill the tile rectangle
        for (let y = 0; y < tileSize; y++) {
            for (let x = 0; x < tileSize; x++) {
                image.setPixelColor(i * tileSize + x, y, Qt.rgba(r/255, g/255, b/255, 1));
            }
        }
    }

    // Save image to extensions directory
    const imagePath = tiled.extensionsPath + "/" + name + "_tileset.png";
    image.save(imagePath);

    // Create tileset from image
    const tileset = new Tileset(name);
    tileset.tileWidth = tileSize;
    tileset.tileHeight = tileSize;
    tileset.loadFromImage(image, imagePath);

    // Add to map
    map.addTileset(tileset);

    tiled.log("MCP Bridge: Generated tileset '" + name + "' with " + numTiles + " colored tiles");
}

function reloadMap(params) {
    if (params.path) {
        tiled.open(params.path);
        tiled.log("MCP Bridge: Reloaded map from " + params.path);
    }
}

// =============================================================================
// Start Polling
// =============================================================================

// Poll for commands every POLL_INTERVAL ms
tiled.log("MCP Bridge: Starting command polling (interval: " + POLL_INTERVAL + "ms)");

// Use setInterval equivalent - Tiled's timer
let timerId = null;

function startPolling() {
    // Tiled uses Qt timers, we'll use a recursive timeout
    const poll = function() {
        checkForCommands();
        timerId = tiled.setTimeout(poll, POLL_INTERVAL);
    };

    // Check if setTimeout exists, otherwise use action trigger
    if (typeof tiled.setTimeout === 'function') {
        timerId = tiled.setTimeout(poll, POLL_INTERVAL);
    } else {
        // Fallback: Register an action that polls on trigger
        tiled.log("MCP Bridge: Note - Using manual polling. Run 'MCP Poll' action to check for commands.");

        const pollAction = tiled.registerAction("MCPPoll", function() {
            checkForCommands();
        });
        pollAction.text = "MCP Poll";

        // Also poll on asset changes
        tiled.assetOpened.connect(checkForCommands);
        tiled.activeAssetChanged.connect(checkForCommands);
    }
}

startPolling();

tiled.log("MCP Bridge: Ready! Command file: " + COMMAND_FILE);

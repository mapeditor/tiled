# Tiled MCP Server

An MCP (Model Context Protocol) server that wraps Tiled map editor capabilities, enabling AI agents to create and modify 2D tile-based worlds with real-time visual feedback.

## Architecture

The server uses a file-based command queue for real-time communication with Tiled:

1. **MCP Server (Python)**: Exposes tools for map manipulation, writes commands to `commands.json`
2. **Tiled Extension (JavaScript)**: Polls `commands.json` every 100ms and executes commands in Tiled

## Setup

### Install dependencies

```bash
cd tiled/mcp-server
uv sync
```

### Install Tiled

Download and install from https://www.mapeditor.org/ or:
```bash
brew install tiled
```

### Install the Tiled extension

Symlink the extension folder into Tiled's extensions directory.

> **Note:** The extensions directory is created by Tiled on first launch. If you just installed Tiled (e.g. via `brew install tiled`), open Tiled once first so it creates its config directories, then close it.

```bash
cd tiled/mcp-server

# macOS
ln -s $(pwd)/extension ~/Library/Preferences/Tiled/extensions/tiled-mcp-bridge

# Linux
mkdir -p ~/.config/tiled/extensions
ln -s $(pwd)/extension ~/.config/tiled/extensions/tiled-mcp-bridge
```

This symlinks the entire extension folder — no copying needed. The extension reads `commands.json` relative to its own location.

### Run the MCP server

```bash
uv run tiled-mcp
```

## Verifying it works (without an LLM)

You can test the MCP server and Tiled extension independently before connecting to an LLM.

### Option 1: MCP Inspector

The `mcp dev` command opens an interactive web UI where you can browse and call tools:

```bash
cd tiled/mcp-server
uv run mcp dev src/tiled_mcp/server.py
```

This opens a browser at `http://localhost:6274`. From there you can call tools like `create_map`, `generate_colored_tileset`, `fill_area`, etc. and see the JSON responses. If Tiled is open with the extension installed, you'll see the map update in real-time.

### Option 2: Write commands directly

The Tiled extension polls `commands.json` for new commands. You can skip the MCP server entirely and write commands by hand to verify the extension works:

1. Open Tiled (with the extension symlinked)
2. Write a command to the file:

```bash
cat > mcp-server/extension/commands.json << 'EOF'
[{"type": "create_map", "params": {"width": 10, "height": 10, "tile_width": 32, "tile_height": 32}}]
EOF
```

You should see a new 10x10 map appear in Tiled within ~100ms. The extension clears the file after processing, so it will reset to `[]`.

3. Try generating a colored tileset and filling tiles:

```bash
cat > mcp-server/extension/commands.json << 'EOF'
[
  {"type": "generate_tileset", "params": {"name": "test", "colors": ["#228B22", "#8B4513", "#808080"], "tile_size": 32}},
  {"type": "fill_area", "params": {"layer": "Ground", "x": 0, "y": 0, "width": 10, "height": 10, "tile_id": 1}}
]
EOF
```

## Available Tools

### Map Management
- `create_map` - Create new map with dimensions and tile size
- `get_map_state` - Return current map as JSON
- `save_map` - Export map to file

### Layer Operations
- `add_tile_layer` - Add new tile layer
- `add_object_layer` - Add new object layer
- `remove_layer` - Remove layer by name

### Tile Operations
- `set_tile` - Place single tile
- `set_tiles` - Batch place tiles
- `fill_area` - Fill rectangle with tile
- `clear_area` - Clear rectangle

### Tileset Operations
- `add_tileset` - Reference existing tileset
- `generate_tileset` - Generate colored tiles programmatically

## Usage with Claude Code

Add to your Claude Code MCP configuration:

```json
{
  "mcpServers": {
    "tiled": {
      "command": "uv",
      "args": ["run", "--directory", "/path/to/tiled/mcp-server", "tiled-mcp"]
    }
  }
}
```

Then ask Claude to create worlds:

```
> Create a 20x15 dungeon with stone walls around the edges and a dirt floor
```

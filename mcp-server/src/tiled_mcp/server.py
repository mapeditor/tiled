"""MCP server for Tiled map editor."""

import json
from pathlib import Path
from typing import Any

from mcp.server.fastmcp import FastMCP

from .command_queue import Command, get_command_queue
from .map_state import (
    MapObject,
    MapState,
    TileLayer,
    TilesetRef,
    get_map_state,
    reset_map_state,
)

# Create the MCP server
mcp = FastMCP(
    name="Tiled Map Editor",
    instructions="Create and manipulate 2D tile-based maps in Tiled. Use create_map to start, add tilesets with add_tileset or generate_colored_tileset, then place tiles with set_tile, fill_area, etc.",
)


def push_command(command_type: str, **params: Any) -> None:
    """Push a command to the Tiled extension."""
    queue = get_command_queue()
    queue.push(Command(type=command_type, params=params))


# =============================================================================
# Map Management Tools
# =============================================================================


@mcp.tool()
def create_map(
    width: int = 20,
    height: int = 15,
    tile_width: int = 32,
    tile_height: int = 32,
    orientation: str = "orthogonal",
) -> str:
    """Create a new map with the specified dimensions.

    Args:
        width: Number of tiles horizontally (default: 20)
        height: Number of tiles vertically (default: 15)
        tile_width: Width of each tile in pixels (default: 32)
        tile_height: Height of each tile in pixels (default: 32)
        orientation: Map orientation - orthogonal, isometric, staggered, or hexagonal

    Returns:
        Confirmation message with map details
    """
    state = reset_map_state()
    state.width = width
    state.height = height
    state.tile_width = tile_width
    state.tile_height = tile_height
    state.orientation = orientation

    # Add a default tile layer
    state.add_tile_layer("Ground")

    # Push command to Tiled
    push_command(
        "create_map",
        width=width,
        height=height,
        tile_width=tile_width,
        tile_height=tile_height,
        orientation=orientation,
    )

    return f"Created {width}x{height} {orientation} map with {tile_width}x{tile_height} tiles. Default layer 'Ground' added."


@mcp.tool()
def resize_map(width: int, height: int) -> str:
    """Resize the current map.

    Args:
        width: New width in tiles
        height: New height in tiles

    Returns:
        Confirmation message
    """
    state = get_map_state()
    old_width, old_height = state.width, state.height
    state.width = width
    state.height = height

    # Resize tile layers
    for layer in state.tile_layers:
        new_data = [0] * (width * height)
        for y in range(min(old_height, height)):
            for x in range(min(old_width, width)):
                old_idx = y * old_width + x
                new_idx = y * width + x
                if old_idx < len(layer.data):
                    new_data[new_idx] = layer.data[old_idx]
        layer.data = new_data
        layer.width = width
        layer.height = height

    push_command("resize_map", width=width, height=height)
    return f"Resized map from {old_width}x{old_height} to {width}x{height}"


@mcp.tool()
def get_state() -> str:
    """Get the current map state as JSON.

    Returns:
        JSON representation of the current map
    """
    state = get_map_state()
    return json.dumps(state.to_tiled_json(), indent=2)


@mcp.tool()
def save_map(path: str, format: str = "json") -> str:
    """Save the current map to a file.

    Args:
        path: File path to save to (relative or absolute)
        format: File format - 'json' or 'tmx' (default: json)

    Returns:
        Confirmation message with saved path
    """
    state = get_map_state()
    file_path = Path(path)

    if format == "json" or file_path.suffix == ".json":
        with open(file_path, "w") as f:
            json.dump(state.to_tiled_json(), f, indent=2)
    else:
        # For TMX, we'll save as JSON and let Tiled handle conversion
        json_path = file_path.with_suffix(".json")
        with open(json_path, "w") as f:
            json.dump(state.to_tiled_json(), f, indent=2)
        push_command("save_as_tmx", json_path=str(json_path), tmx_path=str(file_path))
        return f"Saved map to {json_path}, Tiled will convert to TMX at {file_path}"

    push_command("reload_map", path=str(file_path.absolute()))
    return f"Saved map to {file_path.absolute()}"


# =============================================================================
# Layer Tools
# =============================================================================


@mcp.tool()
def add_tile_layer(name: str) -> str:
    """Add a new tile layer to the map.

    Args:
        name: Name for the new layer

    Returns:
        Confirmation message
    """
    state = get_map_state()

    if state.get_tile_layer(name):
        return f"Error: Layer '{name}' already exists"

    state.add_tile_layer(name)
    push_command("add_tile_layer", name=name)
    return f"Added tile layer '{name}'"


@mcp.tool()
def add_object_layer(name: str) -> str:
    """Add a new object layer to the map.

    Args:
        name: Name for the new layer

    Returns:
        Confirmation message
    """
    state = get_map_state()

    if state.get_object_layer(name):
        return f"Error: Object layer '{name}' already exists"

    state.add_object_layer(name)
    push_command("add_object_layer", name=name)
    return f"Added object layer '{name}'"


@mcp.tool()
def remove_layer(name: str) -> str:
    """Remove a layer from the map.

    Args:
        name: Name of the layer to remove

    Returns:
        Confirmation message
    """
    state = get_map_state()

    # Try tile layers first
    for i, layer in enumerate(state.tile_layers):
        if layer.name == name:
            state.tile_layers.pop(i)
            push_command("remove_layer", name=name)
            return f"Removed tile layer '{name}'"

    # Try object layers
    for i, layer in enumerate(state.object_layers):
        if layer.name == name:
            state.object_layers.pop(i)
            push_command("remove_layer", name=name)
            return f"Removed object layer '{name}'"

    return f"Error: Layer '{name}' not found"


@mcp.tool()
def set_current_layer(name: str) -> str:
    """Set the current working layer for tile operations.

    Args:
        name: Name of the layer to make current

    Returns:
        Confirmation message
    """
    state = get_map_state()

    if state.get_tile_layer(name):
        state.current_layer = name
        push_command("set_current_layer", name=name)
        return f"Set current layer to '{name}'"

    return f"Error: Tile layer '{name}' not found"


@mcp.tool()
def list_layers() -> str:
    """List all layers in the current map.

    Returns:
        List of layers with their types
    """
    state = get_map_state()
    layers = []

    for layer in state.tile_layers:
        current = " (current)" if layer.name == state.current_layer else ""
        layers.append(f"  - {layer.name} [tile]{current}")

    for layer in state.object_layers:
        layers.append(f"  - {layer.name} [object]")

    if not layers:
        return "No layers in the map"

    return "Layers:\n" + "\n".join(layers)


# =============================================================================
# Tile Tools
# =============================================================================


@mcp.tool()
def set_tile(x: int, y: int, tile_id: int, layer: str | None = None) -> str:
    """Place a single tile at the specified position.

    Args:
        x: X coordinate in tiles
        y: Y coordinate in tiles
        tile_id: Global tile ID to place (0 = empty/clear)
        layer: Layer name (uses current layer if not specified)

    Returns:
        Confirmation message
    """
    state = get_map_state()
    layer_name = layer or state.current_layer

    if not layer_name:
        return "Error: No layer specified and no current layer set"

    tile_layer = state.get_tile_layer(layer_name)
    if not tile_layer:
        return f"Error: Tile layer '{layer_name}' not found"

    if tile_layer.set_tile(x, y, tile_id):
        push_command("set_tile", x=x, y=y, tile_id=tile_id, layer=layer_name)
        return f"Set tile at ({x}, {y}) to {tile_id} on layer '{layer_name}'"
    else:
        return f"Error: Position ({x}, {y}) is out of bounds"


@mcp.tool()
def set_tiles(tiles: list[dict[str, int]], layer: str | None = None) -> str:
    """Place multiple tiles at once.

    Args:
        tiles: List of tile placements, each with {x, y, tile_id}
        layer: Layer name (uses current layer if not specified)

    Returns:
        Confirmation message
    """
    state = get_map_state()
    layer_name = layer or state.current_layer

    if not layer_name:
        return "Error: No layer specified and no current layer set"

    tile_layer = state.get_tile_layer(layer_name)
    if not tile_layer:
        return f"Error: Tile layer '{layer_name}' not found"

    count = 0
    for tile in tiles:
        x, y, tile_id = tile.get("x", 0), tile.get("y", 0), tile.get("tile_id", 0)
        if tile_layer.set_tile(x, y, tile_id):
            count += 1

    push_command("set_tiles", tiles=tiles, layer=layer_name)
    return f"Set {count} tiles on layer '{layer_name}'"


@mcp.tool()
def fill_area(
    x: int, y: int, width: int, height: int, tile_id: int, layer: str | None = None
) -> str:
    """Fill a rectangular area with a tile.

    Args:
        x: Starting X coordinate
        y: Starting Y coordinate
        width: Width of area to fill
        height: Height of area to fill
        tile_id: Tile ID to fill with
        layer: Layer name (uses current layer if not specified)

    Returns:
        Confirmation message
    """
    state = get_map_state()
    layer_name = layer or state.current_layer

    if not layer_name:
        return "Error: No layer specified and no current layer set"

    tile_layer = state.get_tile_layer(layer_name)
    if not tile_layer:
        return f"Error: Tile layer '{layer_name}' not found"

    count = 0
    for ty in range(y, y + height):
        for tx in range(x, x + width):
            if tile_layer.set_tile(tx, ty, tile_id):
                count += 1

    push_command(
        "fill_area",
        x=x,
        y=y,
        width=width,
        height=height,
        tile_id=tile_id,
        layer=layer_name,
    )
    return f"Filled {count} tiles in {width}x{height} area at ({x}, {y}) with tile {tile_id}"


@mcp.tool()
def clear_area(
    x: int, y: int, width: int, height: int, layer: str | None = None
) -> str:
    """Clear a rectangular area (set all tiles to 0).

    Args:
        x: Starting X coordinate
        y: Starting Y coordinate
        width: Width of area to clear
        height: Height of area to clear
        layer: Layer name (uses current layer if not specified)

    Returns:
        Confirmation message
    """
    return fill_area(x, y, width, height, 0, layer)


@mcp.tool()
def draw_rectangle_border(
    x: int,
    y: int,
    width: int,
    height: int,
    tile_id: int,
    layer: str | None = None,
) -> str:
    """Draw a rectangle border (outline only, not filled).

    Args:
        x: Starting X coordinate
        y: Starting Y coordinate
        width: Width of rectangle
        height: Height of rectangle
        tile_id: Tile ID for the border
        layer: Layer name (uses current layer if not specified)

    Returns:
        Confirmation message
    """
    state = get_map_state()
    layer_name = layer or state.current_layer

    if not layer_name:
        return "Error: No layer specified and no current layer set"

    tile_layer = state.get_tile_layer(layer_name)
    if not tile_layer:
        return f"Error: Tile layer '{layer_name}' not found"

    tiles = []
    # Top and bottom edges
    for tx in range(x, x + width):
        tiles.append({"x": tx, "y": y, "tile_id": tile_id})
        tiles.append({"x": tx, "y": y + height - 1, "tile_id": tile_id})

    # Left and right edges (excluding corners already done)
    for ty in range(y + 1, y + height - 1):
        tiles.append({"x": x, "y": ty, "tile_id": tile_id})
        tiles.append({"x": x + width - 1, "y": ty, "tile_id": tile_id})

    for tile in tiles:
        tile_layer.set_tile(tile["x"], tile["y"], tile["tile_id"])

    push_command("set_tiles", tiles=tiles, layer=layer_name)
    return f"Drew {width}x{height} rectangle border at ({x}, {y}) with tile {tile_id}"


# =============================================================================
# Object Tools
# =============================================================================


@mcp.tool()
def add_rectangle_object(
    x: float,
    y: float,
    width: float,
    height: float,
    name: str = "",
    layer: str | None = None,
    properties: dict[str, Any] | None = None,
) -> str:
    """Add a rectangle object to an object layer.

    Args:
        x: X position in pixels
        y: Y position in pixels
        width: Width in pixels
        height: Height in pixels
        name: Object name (optional)
        layer: Object layer name (uses first object layer if not specified)
        properties: Custom properties dict (optional)

    Returns:
        Confirmation message with object ID
    """
    state = get_map_state()

    obj_layer = None
    if layer:
        obj_layer = state.get_object_layer(layer)
    elif state.object_layers:
        obj_layer = state.object_layers[0]

    if not obj_layer:
        # Create a default object layer
        obj_layer = state.add_object_layer("Objects")
        push_command("add_object_layer", name="Objects")

    obj = MapObject(
        id=state._next_object_id,
        name=name,
        type="rectangle",
        x=x,
        y=y,
        width=width,
        height=height,
        properties=properties or {},
    )
    state._next_object_id += 1
    obj_layer.add_object(obj)

    push_command(
        "add_object",
        layer=obj_layer.name,
        object=obj.model_dump(),
    )
    return f"Added rectangle object '{name}' (id={obj.id}) at ({x}, {y}) size {width}x{height}"


@mcp.tool()
def add_point_object(
    x: float,
    y: float,
    name: str = "",
    layer: str | None = None,
    properties: dict[str, Any] | None = None,
) -> str:
    """Add a point object to an object layer.

    Args:
        x: X position in pixels
        y: Y position in pixels
        name: Object name (optional)
        layer: Object layer name
        properties: Custom properties dict (optional)

    Returns:
        Confirmation message with object ID
    """
    state = get_map_state()

    obj_layer = None
    if layer:
        obj_layer = state.get_object_layer(layer)
    elif state.object_layers:
        obj_layer = state.object_layers[0]

    if not obj_layer:
        obj_layer = state.add_object_layer("Objects")
        push_command("add_object_layer", name="Objects")

    obj = MapObject(
        id=state._next_object_id,
        name=name,
        type="point",
        x=x,
        y=y,
        properties=properties or {},
    )
    state._next_object_id += 1
    obj_layer.add_object(obj)

    push_command("add_object", layer=obj_layer.name, object=obj.model_dump())
    return f"Added point object '{name}' (id={obj.id}) at ({x}, {y})"


# =============================================================================
# Tileset Tools
# =============================================================================


@mcp.tool()
def add_tileset(
    source: str,
    name: str,
    tile_width: int = 32,
    tile_height: int = 32,
    tile_count: int | None = None,
    columns: int | None = None,
) -> str:
    """Add a tileset to the map.

    Args:
        source: Path to tileset image (PNG) or TSX file
        name: Name for the tileset
        tile_width: Width of each tile in pixels
        tile_height: Height of each tile in pixels
        tile_count: Total number of tiles (optional, calculated from image if not provided)
        columns: Number of columns in the tileset (optional)

    Returns:
        Confirmation message with first GID
    """
    state = get_map_state()

    tileset = TilesetRef(
        name=name,
        source=source,
        tile_width=tile_width,
        tile_height=tile_height,
        tile_count=tile_count,
        columns=columns,
    )
    state.add_tileset(tileset)

    push_command("add_tileset", tileset=tileset.model_dump())
    return f"Added tileset '{name}' from {source}, first GID = {tileset.first_gid}"


@mcp.tool()
def generate_colored_tileset(
    colors: list[str],
    name: str = "generated",
    tile_size: int = 32,
) -> str:
    """Generate a simple tileset with colored tiles.

    Args:
        colors: List of hex colors (e.g., ["#ff0000", "#00ff00", "#0000ff"])
        name: Name for the tileset
        tile_size: Size of each tile in pixels (square)

    Returns:
        Confirmation message with tile ID mapping
    """
    state = get_map_state()

    # Calculate first_gid
    first_gid = 1
    if state.tilesets:
        last = state.tilesets[-1]
        first_gid = last.first_gid + (last.tile_count or 256)

    # Store tileset info - the Tiled extension will generate the actual image
    tileset = TilesetRef(
        name=name,
        source=f"generated_{name}.png",
        tile_width=tile_size,
        tile_height=tile_size,
        tile_count=len(colors),
        columns=len(colors),
        first_gid=first_gid,
    )
    state.tilesets.append(tileset)

    push_command(
        "generate_tileset",
        name=name,
        colors=colors,
        tile_size=tile_size,
        first_gid=first_gid,
    )

    # Build tile ID mapping
    mapping = []
    for i, color in enumerate(colors):
        mapping.append(f"  {first_gid + i}: {color}")

    return f"Generated tileset '{name}' with {len(colors)} tiles:\n" + "\n".join(
        mapping
    )


@mcp.tool()
def list_tilesets() -> str:
    """List all tilesets in the current map.

    Returns:
        List of tilesets with their details
    """
    state = get_map_state()

    if not state.tilesets:
        return "No tilesets in the map. Use add_tileset() or generate_colored_tileset() to add one."

    lines = ["Tilesets:"]
    for ts in state.tilesets:
        lines.append(
            f"  - {ts.name}: {ts.source} (GID {ts.first_gid}-{ts.first_gid + (ts.tile_count or 256) - 1})"
        )

    return "\n".join(lines)


# =============================================================================
# Utility Tools
# =============================================================================


@mcp.tool()
def get_command_file_path() -> str:
    """Get the path to the commands.json file for Tiled extension setup.

    Returns:
        Absolute path to the commands.json file
    """
    queue = get_command_queue()
    return f"Command file path: {queue.get_file_path()}"


def main() -> None:
    """Run the MCP server."""
    mcp.run()


if __name__ == "__main__":
    main()

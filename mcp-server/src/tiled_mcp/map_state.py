"""In-memory map state management for the MCP server."""

from typing import Any

from pydantic import BaseModel, Field


class TilesetRef(BaseModel):
    """Reference to a tileset."""

    name: str
    source: str  # Path to tileset image or TSX file
    tile_width: int
    tile_height: int
    first_gid: int = 1  # Global tile ID offset
    tile_count: int | None = None
    columns: int | None = None


class TileData(BaseModel):
    """Data for a single tile placement."""

    x: int
    y: int
    tile_id: int  # Global tile ID (0 = empty)


class TileLayer(BaseModel):
    """A layer of tiles."""

    name: str
    width: int
    height: int
    data: list[int] = Field(default_factory=list)  # Flat array of tile IDs
    visible: bool = True
    opacity: float = 1.0

    def model_post_init(self, __context: Any) -> None:
        """Initialize data array if empty."""
        if not self.data:
            self.data = [0] * (self.width * self.height)

    def get_tile(self, x: int, y: int) -> int:
        """Get tile ID at position."""
        if 0 <= x < self.width and 0 <= y < self.height:
            return self.data[y * self.width + x]
        return 0

    def set_tile(self, x: int, y: int, tile_id: int) -> bool:
        """Set tile ID at position. Returns True if successful."""
        if 0 <= x < self.width and 0 <= y < self.height:
            self.data[y * self.width + x] = tile_id
            return True
        return False


class MapObject(BaseModel):
    """An object on an object layer."""

    id: int
    name: str = ""
    type: str = ""  # rectangle, point, polygon, ellipse
    x: float
    y: float
    width: float = 0
    height: float = 0
    rotation: float = 0
    visible: bool = True
    properties: dict[str, Any] = Field(default_factory=dict)
    polygon: list[dict[str, float]] | None = None  # For polygon objects


class ObjectLayer(BaseModel):
    """A layer of objects."""

    name: str
    objects: list[MapObject] = Field(default_factory=list)
    visible: bool = True
    opacity: float = 1.0

    _next_id: int = 1

    def add_object(self, obj: MapObject) -> MapObject:
        """Add an object to the layer."""
        if obj.id == 0:
            obj.id = self._next_id
            self._next_id += 1
        self.objects.append(obj)
        return obj

    def remove_object(self, obj_id: int) -> bool:
        """Remove an object by ID. Returns True if found and removed."""
        for i, obj in enumerate(self.objects):
            if obj.id == obj_id:
                self.objects.pop(i)
                return True
        return False


class MapState(BaseModel):
    """The complete state of a Tiled map."""

    width: int = 10
    height: int = 10
    tile_width: int = 32
    tile_height: int = 32
    orientation: str = "orthogonal"  # orthogonal, isometric, staggered, hexagonal
    render_order: str = "right-down"
    background_color: str | None = None

    tilesets: list[TilesetRef] = Field(default_factory=list)
    tile_layers: list[TileLayer] = Field(default_factory=list)
    object_layers: list[ObjectLayer] = Field(default_factory=list)

    current_layer: str | None = None
    _next_object_id: int = 1

    def get_tile_layer(self, name: str) -> TileLayer | None:
        """Get a tile layer by name."""
        for layer in self.tile_layers:
            if layer.name == name:
                return layer
        return None

    def get_object_layer(self, name: str) -> ObjectLayer | None:
        """Get an object layer by name."""
        for layer in self.object_layers:
            if layer.name == name:
                return layer
        return None

    def add_tile_layer(self, name: str) -> TileLayer:
        """Add a new tile layer."""
        layer = TileLayer(name=name, width=self.width, height=self.height)
        self.tile_layers.append(layer)
        if self.current_layer is None:
            self.current_layer = name
        return layer

    def add_object_layer(self, name: str) -> ObjectLayer:
        """Add a new object layer."""
        layer = ObjectLayer(name=name)
        self.object_layers.append(layer)
        return layer

    def add_tileset(self, tileset: TilesetRef) -> TilesetRef:
        """Add a tileset to the map."""
        # Calculate first_gid based on existing tilesets
        if self.tilesets:
            last = self.tilesets[-1]
            tileset.first_gid = last.first_gid + (last.tile_count or 256)
        else:
            tileset.first_gid = 1
        self.tilesets.append(tileset)
        return tileset

    def to_tiled_json(self) -> dict[str, Any]:
        """Export the map state to Tiled JSON format."""
        layers: list[dict[str, Any]] = []

        # Add tile layers
        for layer in self.tile_layers:
            layers.append(
                {
                    "type": "tilelayer",
                    "name": layer.name,
                    "width": layer.width,
                    "height": layer.height,
                    "x": 0,
                    "y": 0,
                    "data": layer.data,
                    "visible": layer.visible,
                    "opacity": layer.opacity,
                }
            )

        # Add object layers
        for layer in self.object_layers:
            objects = []
            for obj in layer.objects:
                obj_data: dict[str, Any] = {
                    "id": obj.id,
                    "name": obj.name,
                    "type": obj.type,
                    "x": obj.x,
                    "y": obj.y,
                    "width": obj.width,
                    "height": obj.height,
                    "rotation": obj.rotation,
                    "visible": obj.visible,
                }
                if obj.polygon:
                    obj_data["polygon"] = obj.polygon
                if obj.properties:
                    obj_data["properties"] = [
                        {"name": k, "type": "string", "value": str(v)}
                        for k, v in obj.properties.items()
                    ]
                objects.append(obj_data)

            layers.append(
                {
                    "type": "objectgroup",
                    "name": layer.name,
                    "objects": objects,
                    "visible": layer.visible,
                    "opacity": layer.opacity,
                }
            )

        # Build tilesets
        tilesets = []
        for ts in self.tilesets:
            ts_data: dict[str, Any] = {
                "name": ts.name,
                "firstgid": ts.first_gid,
                "tilewidth": ts.tile_width,
                "tileheight": ts.tile_height,
            }
            if ts.source.endswith(".tsx"):
                ts_data["source"] = ts.source
            else:
                ts_data["image"] = ts.source
                if ts.tile_count:
                    ts_data["tilecount"] = ts.tile_count
                if ts.columns:
                    ts_data["columns"] = ts.columns
            tilesets.append(ts_data)

        return {
            "type": "map",
            "version": "1.10",
            "tiledversion": "1.11.0",
            "orientation": self.orientation,
            "renderorder": self.render_order,
            "width": self.width,
            "height": self.height,
            "tilewidth": self.tile_width,
            "tileheight": self.tile_height,
            "infinite": False,
            "layers": layers,
            "tilesets": tilesets,
            "nextlayerid": len(layers) + 1,
            "nextobjectid": self._next_object_id,
        }


# Global map state
_state: MapState | None = None


def get_map_state() -> MapState:
    """Get the global map state instance."""
    global _state
    if _state is None:
        _state = MapState()
    return _state


def reset_map_state() -> MapState:
    """Reset the global map state to a new empty map."""
    global _state
    _state = MapState()
    return _state


def set_map_state(state: MapState) -> None:
    """Set the global map state."""
    global _state
    _state = state

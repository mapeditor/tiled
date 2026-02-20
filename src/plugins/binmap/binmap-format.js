/// <reference types="@mapeditor/tiled-api" />

/*
 * binmap-format.js
 *
 * This extension adds the 'binary map format' type to the Export As menu.
 * This is an alternative to the .so plugin, written in JavaScript.
 * To install, copy to `~/.config/tiled/extensions`
 *
 * Simplest format possible. Consist of a 16 bytes long header and then data.
 * See FORMAT.md for details.
 */

tiled.registerMapFormat("binmap", {
	name: "Binary map files",
	extension: "map",

	write: (map, fileName) => {
		let numLayers = 0, gid = 1;
		/* fix missing firstgid properties on tilesets */
		for (let i = 0; i < map.tilesets.length; ++i) {
			map.tilesets[i].firstgid = gid;
			gid += map.tilesets[i].tileCount;
		}
		/* count tile type layers */
		for (let i = 0; i < map.layerCount; ++i) {
			const layer = map.layerAt(i);
			if (layer.isTileLayer) numLayers++;
		}
		/* file and buffers */
		var file = new BinaryFile(fileName, BinaryFile.WriteOnly);
		let header = new ArrayBuffer(16);
		let item = new ArrayBuffer(2);
		let hu16 = new Uint16Array(header);
		let iu16 = new Uint16Array(item);
		/* write header */
		hu16[0] = 0x414D;
		hu16[1] = 0x0050;
		hu16[2] = map.width;
		hu16[3] = map.height;
		hu16[4] = numLayers;
		hu16[5] = map.tileWidth;
		hu16[6] = map.tileHeight;
		/* looks like the Javascript API can't handle hexagonal maps */
		hu16[7] = map.Isometric ? 1 : 0;
		file.write(header);
		/* write layer tiles */
		for (let i = 0; i < map.layerCount; ++i) {
			const layer = map.layerAt(i);
			if (!layer.isTileLayer) continue;

			for (let y = 0; y < map.height; ++y) {
				for (let x = 0; x < map.width; ++x) {
					const tile = layer.tileAt(x, y);
					/* convert local tile id to global tile id */
					iu16[0] = tile == null ? 0 : tile.id + (tile.tileset ? tile.tileset.firstgid : 1);
					file.write(item);
				}
			}
		}
		file.commit();
	},
});

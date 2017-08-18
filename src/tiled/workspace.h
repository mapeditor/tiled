#pragma once

#include "map.h"
#include "layer.h"
#include "tilelayer.h"

namespace Tiled {
namespace Internal {

class WorkSpace
{
public:
	WorkSpace(int width, int height, int tileWidth, int tileHeight)
		: mWidth(width)
		, mHeight(height)
		, mTileWidth(tileWidth)
		, mTileHeight(tileHeight)
	{}

	 int width() const { return mWidth; }
	 int height() const { return mHeight; }
	 int tileWidth() const { return mTileWidth; }
	 int tileHeight() const { return mTileHeight; }
}

}
}

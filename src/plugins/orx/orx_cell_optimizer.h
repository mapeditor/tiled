#ifndef ORXEXPORTER_CELL_OPTIMIZER_H
#define ORXEXPORTER_CELL_OPTIMIZER_H

#include "mapformat.h"
#include "map.h"
#include "savefile.h"
#include "tile.h"
#include "tilelayer.h"
#include "objectgroup.h"
#include "imagelayer.h"
#include "imagereference.h"
#include "mapobject.h"

#include "orx_objects.h"


namespace Orx
{
    enum class OptimizeMode
    {
        None,
        HorizontalVertical,
        VerticalHorizontal,
    };

    ///////////////////////////////////////////////////////////////////////////////
    class CellOptimizer
    {
        public:
            // optimizes cells of layer, first horizontally then vertically
            static void optimize_cells(OptimizeMode mode, int width, int height, Grid2D<OptimizedCell> & call_map, const Tiled::TileLayer * layer);

        private:
            // computes the number of times a cell is repeated horizontally starting from given coords
            static int get_h_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_x);
            // computes the number of times a cell is repeated vertically starting from given coords
            static int get_v_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_y);
            // build the cell_map unoptimized
            static void no_optimize(int width, int height, Grid2D<OptimizedCell> & call_map, const Tiled::TileLayer * layer);
            // optimizes cells of layer, first horizontally then vertically
            static void optimize_h_v(int width, int height, Grid2D<OptimizedCell> & call_map, const Tiled::TileLayer * layer);
            // optimizes cells of layer, first vertically then horizontally
            static void optimize_v_h(int width, int height, Grid2D<OptimizedCell> & call_map, const Tiled::TileLayer * layer);

    };

}


#endif //ORXEXPORTER_CELL_OPTIMIZER_H



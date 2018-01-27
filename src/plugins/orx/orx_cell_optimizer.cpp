#include "orx_cell_optimizer.h"

namespace Orx
{

    // optimizes cells of layer, first horizontally then vertically
    void CellOptimizer::optimize_cells(OptimizeMode mode, int width, int height, Grid2D<OptimizedCell> & cell_map, const Tiled::TileLayer * layer)
    {
        switch (mode)
        {
        case OptimizeMode::HorizontalVertical:
            optimize_h_v(width, height, cell_map, layer);
            break;

        case OptimizeMode::VerticalHorizontal:
            optimize_v_h(width, height, cell_map, layer);
            break;

        case OptimizeMode::None:
        default:
            no_optimize(width, height, cell_map, layer);
            break;
        }
    }



    ///////////////////////////////////////////////////////////////////////////////
    int CellOptimizer::get_h_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_x)
    {
        int count = 0;
        for (; x <= max_x; ++x)
            {
            const Tiled::Cell * cell = &layer->cellAt(x, y);
            if (*cell == *value)
                count++;
            else
                break;
            }

        return count;
    }

    ///////////////////////////////////////////////////////////////////////////////
    int CellOptimizer::get_v_repetitions(const Tiled::TileLayer * layer, int x, int y, const Tiled::Cell * value, int max_y)
    {
        int count = 0;
        for (; y <= max_y; ++y)
            {
            const Tiled::Cell * cell = &layer->cellAt(x, y);
            if (*cell == *value)
                count++;
            else
                break;
            }

        return count;
    }

    ///////////////////////////////////////////////////////////////////////////////
    void CellOptimizer::no_optimize(int width, int height, Grid2D<OptimizedCell> & cell_map, const Tiled::TileLayer * layer)
    {
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                OptimizedCell & ocell = cell_map.at(x, y);
                ocell.m_Valid = true;
                ocell.m_RepeatX = 1;
                ocell.m_RepeatY = 1;
                ocell.m_Cell = &layer->cellAt(x, y);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void CellOptimizer::optimize_h_v(int width, int height, Grid2D<OptimizedCell> & cell_map, const Tiled::TileLayer * layer)
    {
        // perform horizontal optimization
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; /*++x*/)
            {
                const Tiled::Cell * cell = &layer->cellAt(x, y);
                int rep = get_h_repetitions(layer, x, y, cell, width);
                OptimizedCell & ocell = cell_map.at(x, y);
                ocell.m_Valid = true;
                ocell.m_RepeatX = rep;
                ocell.m_RepeatY = 1;
                ocell.m_Cell = cell;
                x += rep;
            }
        }

        // perform vertical optimization
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; /*++y*/)
            {
                const Tiled::Cell * cell = &layer->cellAt(x, y);
                OptimizedCell & ocell = cell_map.at(x, y);
                if (ocell.m_Valid && (cell_map.at(x, y).m_RepeatX == 1))
                {
                    int rep = get_v_repetitions(layer, x, y, cell, height);
                    ocell.m_Valid = true;
                    ocell.m_RepeatY = rep;
                    ocell.m_Cell = cell;
                    y += rep;
                }
                else
                    y++;
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    void CellOptimizer::optimize_v_h(int width, int height, Grid2D<OptimizedCell> & cell_map, const Tiled::TileLayer * layer)
    {
        // perform vertical optimization
        for (int x = 0; x < width; ++x)
        {
            for (int y = 0; y < height; /*++y*/)
            {
                const Tiled::Cell * cell = &layer->cellAt(x, y);
                int rep = get_v_repetitions(layer, x, y, cell, height);
                OptimizedCell & ocell = cell_map.at(x, y);
                ocell.m_Valid = true;
                ocell.m_RepeatX = 1;
                ocell.m_RepeatY = rep;
                ocell.m_Cell = cell;
                y += rep;
            }
        }

        // perform horizontal optimization
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; /*++x*/)
            {
                const Tiled::Cell * cell = &layer->cellAt(x, y);
                OptimizedCell & ocell = cell_map.at(x, y);
                if (ocell.m_Valid && (cell_map.at(x, y).m_RepeatY == 1))
                {
                    int rep = get_h_repetitions(layer, x, y, cell, width);
                    ocell.m_Valid = true;
                    ocell.m_RepeatX = rep;
                    ocell.m_RepeatY = 1;
                    ocell.m_Cell = cell;
                    x += rep;
                }
                else
                    x++;
            }
        }
    }

}

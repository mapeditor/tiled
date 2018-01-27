#ifndef ORXEXPORTER_UTILITY_H
#define ORXEXPORTER_UTILITY_H

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

#include <QFileInfo>
#include <QDir>


namespace Orx
{
    struct Utility
    {
        ///////////////////////////////////////////////////////////////////////////////
        // copies an image into destination folder
        static bool copy_image(ImagePtr img, const QString & folder, QFileInfo & dst_finfo)
        {
            bool ret = false;

            if (img->m_UseCount)
            {
                QFileInfo src_finfo(img->m_Texture);
                QString fn = src_finfo.fileName();
                if (folder.isEmpty())
                {
                    QString dst_path = dst_finfo.dir().absolutePath() + QDir::separator() + fn;
                    QFile::copy(img->m_Texture, dst_path);
                    img->m_Texture = fn;
                }
                else
                {
                    QString dst_dir = dst_finfo.dir().absolutePath() + QDir::separator() + folder;
                    QDir dir(dst_dir);
                    if (!dir.exists())
                        dir.mkpath(".");
                    QString dst_path = dst_finfo.dir().absolutePath() + QDir::separator() + folder + QDir::separator() + fn;
                    QFile::copy(img->m_Texture, dst_path);
                    img->m_Texture = folder + QDir::separator() + fn;
                }

                ret = true;
            }

            return ret;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // converts the given absolute filename to a filename relative to destination save directory
        static QString get_relative_filename(const QString & base_filename, const QString & filename)
        {
            QFileInfo fi(base_filename);
            QDir dir(fi.dir());
            QString s = dir.relativeFilePath(filename);
            return s;
        }

        ///////////////////////////////////////////////////////////////////////////////
        // returns the name of the given tile
        static QString get_tile_name(const Tiled::Tile * tile)
        {
            QString tile_filename;
            if (tile->imageSource().isEmpty() == false)
                tile_filename = tile->imageSource().toString();
            else
                tile_filename = tile->tileset()->imageSource().toString();

            QFileInfo fi(tile_filename);
            tile_filename = fi.baseName();
            tile_filename = OrxObject::normalize_name(tile_filename);
            return tile_filename;
        }

    };

}


#endif //ORXEXPORTER_UTILITY_H



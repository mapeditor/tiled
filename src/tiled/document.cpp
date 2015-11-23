/*
 * document.cpp
 * Copyright 2015, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "document.h"

#include <QUndoStack>

namespace Tiled {
namespace Internal {

Document::Document(const QString &fileName,
                   QObject *parent)
    : QObject(parent)
    , mFileName(fileName)
    , mUndoStack(new QUndoStack(this))
{
    connect(mUndoStack, &QUndoStack::cleanChanged,
            this, &Document::modifiedChanged);
}

void Document::setFileName(const QString &fileName)
{
    if (mFileName == fileName)
        return;

    QString oldFileName = mFileName;
    mFileName = fileName;
    emit fileNameChanged(fileName, oldFileName);
}

/**
 * Returns whether the document has unsaved changes.
 */
bool Document::isModified() const
{
    return !mUndoStack->isClean();
}

} // namespace Internal
} // namespace Tiled

/*
 * editableproject.h
 * Copyright 2023, Chris Boehm AKA dogboydog
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include "project.h"
#include "editableasset.h"
#include <QObject>

namespace Tiled {

class EditableProject : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(QString extensionsPath READ extensionsPath)
    Q_PROPERTY(QString automappingRulesFile READ automappingRulesFile)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QStringList folders READ folders)

public:
    EditableProject(Project *project, QObject *parent = nullptr);
    bool isReadOnly() const override;
    QString extensionsPath() const;
    QString automappingRulesFile() const;
    QString fileName() const;
    QStringList folders() const;
    QSharedPointer<Document> createDocument() override;
private:
    Project *mProject;
};

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::EditableProject*)

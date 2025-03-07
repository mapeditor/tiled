/*
 * editableproject.h
 * Copyright 2023, dogboydog
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

#include "editableasset.h"
#include "project.h"
#include "scriptpropertytype.h"

#include <QObject>

namespace Tiled {

class ProjectDocument;

class EditableProject final : public EditableAsset
{
    Q_OBJECT

    Q_PROPERTY(QString extensionsPath READ extensionsPath)
    Q_PROPERTY(QString automappingRulesFile READ automappingRulesFile)
    Q_PROPERTY(QString fileName READ fileName)
    Q_PROPERTY(QStringList folders READ folders)
    Q_PROPERTY(QVector<ScriptPropertyType *> propertyTypes READ propertyTypes)

public:
    EditableProject(ProjectDocument *projectDocument, QObject *parent = nullptr);

    bool isReadOnly() const override;
    AssetType::Value assetType() const override { return AssetType::Project; }

    QString extensionsPath() const;
    QString automappingRulesFile() const;
    QString fileName() const;
    QStringList folders() const;
    QVector<ScriptPropertyType*> propertyTypes() const;

    Project *project() const;

    QSharedPointer<Document> createDocument() override;

    Q_INVOKABLE bool removeTypeByName(const QString &name);
    Q_INVOKABLE ScriptPropertyType *findTypeByName(const QString &name);
    Q_INVOKABLE ScriptPropertyType *addClassType(const QString &name);
    Q_INVOKABLE ScriptPropertyType *addEnumType(const QString &name);

private:
    ScriptPropertyType *toScriptType(const SharedPropertyType &type) const;
    ScriptPropertyType *addPropertyType(const SharedPropertyType &type);
    void applyPropertyChanges();
};

inline Project *EditableProject::project() const
{
    return static_cast<Project*>(object());
}

} // namespace Tiled

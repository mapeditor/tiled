/*
 * varianteditorfactory.h
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef VARIANTEDITORFACTORY_H
#define VARIANTEDITORFACTORY_H

#include <QtVariantEditorFactory>

namespace Tiled {
namespace Internal {

class FileEdit;

/**
 * Extension of the QtVariantEditorFactory that adds support for a FileEdit,
 * used for editing file references.
 *
 * It also adds support for a "suggestions" attribute for string values.
 */
class VariantEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT

public:
    explicit VariantEditorFactory(QObject *parent = 0)
        : QtVariantEditorFactory(parent)
    {}

    ~VariantEditorFactory();

protected:
    void connectPropertyManager(QtVariantPropertyManager *manager);
    QWidget *createEditor(QtVariantPropertyManager *manager,
                          QtProperty *property,
                          QWidget *parent);
    void disconnectPropertyManager(QtVariantPropertyManager *manager);

private slots:
    void slotPropertyChanged(QtProperty *property, const QVariant &value);
    void slotPropertyAttributeChanged(QtProperty *property,
                                      const QString &attribute,
                                      const QVariant &value);
    void slotSetValue(const QString &value);
    void slotEditorDestroyed(QObject *object);

private:
    QMap<QtProperty *, QList<FileEdit *> > mCreatedEditors;
    QMap<FileEdit *, QtProperty *> mEditorToProperty;
};

} // namespace Internal
} // namespace Tiled

#endif // VARIANTEDITORFACTORY_H

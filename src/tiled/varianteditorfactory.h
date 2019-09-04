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

#pragma once

#include <QtVariantEditorFactory>

class QComboBox;

namespace Tiled {

class FileEdit;
class TextPropertyEdit;
class TilesetParametersEdit;

/**
 * Extension of the QtVariantEditorFactory that adds support for a FileEdit,
 * used for editing file references.
 *
 * It also adds support for "suggestions" and "multiline" attributes for string
 * values.
 */
class VariantEditorFactory : public QtVariantEditorFactory
{
    Q_OBJECT

public:
    explicit VariantEditorFactory(QObject *parent = nullptr)
        : QtVariantEditorFactory(parent)
    {}

    ~VariantEditorFactory();

signals:
    void resetProperty(QtProperty *property);

protected:
    void connectPropertyManager(QtVariantPropertyManager *manager) override;
    QWidget *createEditor(QtVariantPropertyManager *manager,
                          QtProperty *property,
                          QWidget *parent) override;
    void disconnectPropertyManager(QtVariantPropertyManager *manager) override;

private:
    void slotPropertyChanged(QtProperty *property, const QVariant &value);
    void slotPropertyAttributeChanged(QtProperty *property,
                                      const QString &attribute,
                                      const QVariant &value);
    void fileEditFileUrlChanged(const QUrl &value);
    void textPropertyEditTextChanged(const QString &value);
    void comboBoxPropertyEditTextChanged(const QString &value);
    void slotEditorDestroyed(QObject *object);

    QMap<QtProperty *, QList<FileEdit *> > mCreatedFileEdits;
    QMap<FileEdit *, QtProperty *> mFileEditToProperty;

    QMap<QtProperty *, QList<TilesetParametersEdit *> > mCreatedTilesetEdits;
    QMap<TilesetParametersEdit *, QtProperty *> mTilesetEditToProperty;

    QMap<QtProperty *, QList<TextPropertyEdit *> > mCreatedTextPropertyEdits;
    QMap<TextPropertyEdit *, QtProperty *> mTextPropertyEditToProperty;

    QMap<QtProperty *, QList<QComboBox *> > mCreatedComboBoxes;
    QMap<QComboBox *, QtProperty *> mComboBoxToProperty;
};

} // namespace Tiled

/*
 * varianteditorfactory.cpp
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

#include "varianteditorfactory.h"

#include "fileedit.h"
#include "textpropertyedit.h"
#include "tilesetparametersedit.h"
#include "utils.h"
#include "variantpropertymanager.h"

#include <QCompleter>
#include <QHBoxLayout>
#include <QToolButton>

namespace Tiled {
namespace Internal {

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent = nullptr);

signals:
    void resetProperty(QtProperty *property);

private slots:
    void buttonClicked();

private:
    QtProperty *mProperty;
};

ResetWidget::ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent)
    : QWidget(parent)
    , mProperty(property)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    QToolButton *resetButton = new QToolButton(this);
    resetButton->setIcon(QIcon(QLatin1String(":/images/16x16/edit-clear.png")));
    resetButton->setIconSize(QSize(16, 16));
    resetButton->setAutoRaise(true);
    Utils::setThemeIcon(resetButton, "edit-clear");

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(editor);
    layout->addWidget(resetButton);

    connect(resetButton, &QToolButton::clicked, this, &ResetWidget::buttonClicked);
}

void ResetWidget::buttonClicked()
{
    emit resetProperty(mProperty);
}


VariantEditorFactory::~VariantEditorFactory()
{
    qDeleteAll(mFileEditToProperty.keys());
    qDeleteAll(mTilesetEditToProperty.keys());
    qDeleteAll(mTextPropertyEditToProperty.keys());
}

void VariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)),
            this, SLOT(slotPropertyChanged(QtProperty*,QVariant)));
    connect(manager, SIGNAL(attributeChanged(QtProperty*,QString,QVariant)),
            this, SLOT(slotPropertyAttributeChanged(QtProperty*,QString,QVariant)));
    QtVariantEditorFactory::connectPropertyManager(manager);
}

QWidget *VariantEditorFactory::createEditor(QtVariantPropertyManager *manager,
                                            QtProperty *property,
                                            QWidget *parent)
{
    const int type = manager->propertyType(property);

    if (type == filePathTypeId()) {
        FileEdit *editor = new FileEdit(parent);
        FilePath filePath = manager->value(property).value<FilePath>();
        editor->setFilePath(filePath.absolutePath);
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        mCreatedFileEdits[property].append(editor);
        mFileEditToProperty[editor] = property;

        connect(editor, &FileEdit::filePathChanged,
                this, &VariantEditorFactory::fileEditFilePathChanged);
        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));

        return editor;
    }

    if (type == VariantPropertyManager::tilesetParametersTypeId()) {
        auto editor = new TilesetParametersEdit(parent);
        editor->setTileset(manager->value(property).value<EmbeddedTileset>());
        mCreatedTilesetEdits[property].append(editor);
        mTilesetEditToProperty[editor] = property;

        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));

        return editor;
    }

    if (type == QVariant::String) {
        bool multiline = manager->attributeValue(property, QLatin1String("multiline")).toBool();
        if (multiline) {
            auto editor = new TextPropertyEdit(parent);
            editor->setText(manager->value(property).toString());
            mCreatedTextPropertyEdits[property].append(editor);
            mTextPropertyEditToProperty[editor] = property;

            connect(editor, &TextPropertyEdit::textChanged,
                    this, &VariantEditorFactory::textPropertyEditTextChanged);
            connect(editor, SIGNAL(destroyed(QObject *)),
                    this, SLOT(slotEditorDestroyed(QObject *)));

            return editor;
        }
    }

    QWidget *editor = QtVariantEditorFactory::createEditor(manager, property, parent);

    if (type == QVariant::String) {
        // Add support for "suggestions" attribute that adds a QCompleter to the QLineEdit
        QVariant suggestions = manager->attributeValue(property, QLatin1String("suggestions"));
        if (!suggestions.toStringList().isEmpty()) {
            if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(editor)) {
                QCompleter *completer = new QCompleter(suggestions.toStringList(), lineEdit);
                completer->setCaseSensitivity(Qt::CaseInsensitive);
                lineEdit->setCompleter(completer);
            }
        }
    }

    if (type == QVariant::Color) {
        // Allow resetting a color property to the invalid color
        ResetWidget *resetWidget = new ResetWidget(property, editor, parent);
        connect(resetWidget, &ResetWidget::resetProperty,
                this, &VariantEditorFactory::resetProperty);
        editor = resetWidget;
    }

    return editor;
}

void VariantEditorFactory::disconnectPropertyManager(QtVariantPropertyManager *manager)
{
    disconnect(manager, SIGNAL(valueChanged(QtProperty*,QVariant)),
               this, SLOT(slotPropertyChanged(QtProperty*,QVariant)));
    disconnect(manager, SIGNAL(attributeChanged(QtProperty*,QString,QVariant)),
               this, SLOT(slotPropertyAttributeChanged(QtProperty*,QString,QVariant)));
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantEditorFactory::slotPropertyChanged(QtProperty *property,
                                               const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        for (FileEdit *edit : mCreatedFileEdits[property]) {
            FilePath filePath = value.value<FilePath>();
            edit->setFilePath(filePath.absolutePath);
        }
    }
    else if (mCreatedTilesetEdits.contains(property)) {
        for (TilesetParametersEdit *edit : mCreatedTilesetEdits[property])
            edit->setTileset(value.value<EmbeddedTileset>());
    }
    else if (mCreatedTextPropertyEdits.contains(property)) {
        for (TextPropertyEdit *edit : mCreatedTextPropertyEdits[property])
            edit->setText(value.toString());
    }
}

void VariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property,
                                                        const QString &attribute,
                                                        const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            for (FileEdit *edit : mCreatedFileEdits[property])
                edit->setFilter(value.toString());
        }
    }
    // changing of "multiline" attribute currently not supported
}

void VariantEditorFactory::fileEditFilePathChanged(const QString &value)
{
    FileEdit *fileEdit = qobject_cast<FileEdit*>(sender());
    Q_ASSERT(fileEdit);

    if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, QVariant::fromValue(FilePath { value }));
    }
}

void VariantEditorFactory::textPropertyEditTextChanged(const QString &value)
{
    auto textPropertyEdit = qobject_cast<TextPropertyEdit*>(sender());
    Q_ASSERT(textPropertyEdit);

    if (QtProperty *property = mTextPropertyEditToProperty.value(textPropertyEdit)) {
        QtVariantPropertyManager *manager = propertyManager(property);
        if (!manager)
            return;
        manager->setValue(property, value);
    }
}

void VariantEditorFactory::slotEditorDestroyed(QObject *object)
{
    // Check if it was a FileEdit
    {
        FileEdit *fileEdit = static_cast<FileEdit*>(object);

        if (QtProperty *property = mFileEditToProperty.value(fileEdit)) {
            mFileEditToProperty.remove(fileEdit);
            mCreatedFileEdits[property].removeAll(fileEdit);
            if (mCreatedFileEdits[property].isEmpty())
                mCreatedFileEdits.remove(property);
            return;
        }
    }

    // Check if it was a TilesetParametersEdit
    {
        TilesetParametersEdit *tilesetEdit = static_cast<TilesetParametersEdit*>(object);

        if (QtProperty *property = mTilesetEditToProperty.value(tilesetEdit)) {
            mTilesetEditToProperty.remove(tilesetEdit);
            mCreatedTilesetEdits[property].removeAll(tilesetEdit);
            if (mCreatedTilesetEdits[property].isEmpty())
                mCreatedTilesetEdits.remove(property);
            return;
        }
    }

    // Check if it was a TextPropertyEdit
    {
        TextPropertyEdit *textPropertyEdit = static_cast<TextPropertyEdit*>(object);

        if (QtProperty *property = mTextPropertyEditToProperty.value(textPropertyEdit)) {
            mTextPropertyEditToProperty.remove(textPropertyEdit);
            mCreatedTextPropertyEdits[property].removeAll(textPropertyEdit);
            if (mCreatedTextPropertyEdits[property].isEmpty())
                mCreatedTextPropertyEdits.remove(property);
            return;
        }
    }
}

} // namespace Internal
} // namespace Tiled

#include "varianteditorfactory.moc"

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
#include "tilesetdocument.h"
#include "tilesetparametersedit.h"
#include "utils.h"
#include "variantpropertymanager.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QToolButton>

#include "qtcompat_p.h"

namespace Tiled {

class ResetWidget : public QWidget
{
    Q_OBJECT

public:
    ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent = nullptr);

signals:
    void resetProperty(QtProperty *property);

private:
    void buttonClicked();

    QtProperty *mProperty;
};

ResetWidget::ResetWidget(QtProperty *property, QWidget *editor, QWidget *parent)
    : QWidget(parent)
    , mProperty(property)
{
    QHBoxLayout *layout = new QHBoxLayout(this);

    QToolButton *resetButton = new QToolButton(this);
    resetButton->setIcon(QIcon(QLatin1String(":/images/16/edit-clear.png")));
    resetButton->setIconSize(Utils::smallIconSize());
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
    qDeleteAll(mFileEditToProperty.keyBegin(), mFileEditToProperty.keyEnd());
    qDeleteAll(mTilesetEditToProperty.keyBegin(), mTilesetEditToProperty.keyEnd());
    qDeleteAll(mTextPropertyEditToProperty.keyBegin(), mTextPropertyEditToProperty.keyEnd());
    qDeleteAll(mComboBoxToProperty.keyBegin(), mComboBoxToProperty.keyEnd());
}

void VariantEditorFactory::connectPropertyManager(QtVariantPropertyManager *manager)
{
    connect(manager, &QtVariantPropertyManager::valueChanged,
            this, &VariantEditorFactory::slotPropertyChanged);
    connect(manager, &QtVariantPropertyManager::attributeChanged,
            this, &VariantEditorFactory::slotPropertyAttributeChanged);
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
        editor->setFileUrl(filePath.url);
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        mCreatedFileEdits[property].append(editor);
        mFileEditToProperty[editor] = property;

        connect(editor, &FileEdit::fileUrlChanged,
                this, &VariantEditorFactory::fileEditFileUrlChanged);
        connect(editor, &QObject::destroyed,
                this, &VariantEditorFactory::slotEditorDestroyed);

        return editor;
    }

    if (type == VariantPropertyManager::tilesetParametersTypeId()) {
        auto editor = new TilesetParametersEdit(parent);
        editor->setTilesetDocument(manager->value(property).value<TilesetDocument*>());
        mCreatedTilesetEdits[property].append(editor);
        mTilesetEditToProperty[editor] = property;

        connect(editor, &QObject::destroyed,
                this, &VariantEditorFactory::slotEditorDestroyed);

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
            connect(editor, &QObject::destroyed,
                    this, &VariantEditorFactory::slotEditorDestroyed);

            return editor;
        }

        QStringList suggestions = manager->attributeValue(property, QLatin1String("suggestions")).toStringList();
        if (!suggestions.isEmpty()) {
            auto editor = new QComboBox(parent);
            editor->setEditable(true);
            editor->addItems(suggestions);
            editor->setCurrentText(manager->value(property).toString());
            mCreatedComboBoxes[property].append(editor);
            mComboBoxToProperty[editor] = property;

            connect(editor, &QComboBox::currentTextChanged,
                    this, &VariantEditorFactory::comboBoxPropertyEditTextChanged);
            connect(editor, &QObject::destroyed,
                    this, &VariantEditorFactory::slotEditorDestroyed);

            return editor;
        }
    }

    QWidget *editor = QtVariantEditorFactory::createEditor(manager, property, parent);

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
    disconnect(manager, &QtVariantPropertyManager::valueChanged,
               this, &VariantEditorFactory::slotPropertyChanged);
    disconnect(manager, &QtVariantPropertyManager::attributeChanged,
               this, &VariantEditorFactory::slotPropertyAttributeChanged);
    QtVariantEditorFactory::disconnectPropertyManager(manager);
}

void VariantEditorFactory::slotPropertyChanged(QtProperty *property,
                                               const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        for (FileEdit *edit : qAsConst(mCreatedFileEdits)[property]) {
            FilePath filePath = value.value<FilePath>();
            edit->setFileUrl(filePath.url);
        }
    }
    else if (mCreatedTilesetEdits.contains(property)) {
        for (TilesetParametersEdit *edit : qAsConst(mCreatedTilesetEdits)[property])
            edit->setTilesetDocument(value.value<TilesetDocument*>());
    }
    else if (mCreatedTextPropertyEdits.contains(property)) {
        for (TextPropertyEdit *edit : qAsConst(mCreatedTextPropertyEdits)[property])
            edit->setText(value.toString());
    }
    else if (mCreatedComboBoxes.contains(property)) {
        for (QComboBox *comboBox: qAsConst(mCreatedComboBoxes)[property])
            comboBox->setCurrentText(value.toString());
    }
}

void VariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property,
                                                        const QString &attribute,
                                                        const QVariant &value)
{
    if (mCreatedFileEdits.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            for (FileEdit *edit : qAsConst(mCreatedFileEdits)[property])
                edit->setFilter(value.toString());
        }
    }
    else if (mCreatedComboBoxes.contains(property)) {
        if (attribute == QLatin1String("suggestions")) {
            for (QComboBox *comboBox: qAsConst(mCreatedComboBoxes)[property]) {
                comboBox->clear();
                comboBox->addItems(value.toStringList());
            }
        }
    }
    // changing of "multiline" attribute currently not supported
}

void VariantEditorFactory::fileEditFileUrlChanged(const QUrl &value)
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

void VariantEditorFactory::comboBoxPropertyEditTextChanged(const QString &value)
{
    auto comboBox = qobject_cast<QComboBox*>(sender());
    Q_ASSERT(comboBox);

    if (QtProperty *property = mComboBoxToProperty.value(comboBox)) {
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

    // Check if it was a QComboBox
    {
        QComboBox *comboBox = static_cast<QComboBox*>(object);

        if (QtProperty *property = mComboBoxToProperty.value(comboBox)) {
            mComboBoxToProperty.remove(comboBox);
            mCreatedComboBoxes[property].removeAll(comboBox);
            if (mCreatedComboBoxes[property].isEmpty())
                mCreatedComboBoxes.remove(property);
            return;
        }
    }
}

} // namespace Tiled

#include "varianteditorfactory.moc"

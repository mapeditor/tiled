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

#include "variantpropertymanager.h"
#include "fileedit.h"
#include "utils.h"

#include <QCompleter>
#include <QHBoxLayout>
#include <QToolButton>
#include <QCoreApplication>

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
    qDeleteAll(mEditorToProperty.keys());
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

    if (type == VariantPropertyManager::filePathTypeId()) {
        FileEdit *editor = new FileEdit(parent);
        editor->setFilePath(manager->value(property).toString());
        editor->setFilter(manager->attributeValue(property, QLatin1String("filter")).toString());
        editor->setLineEditEnabled(false);
        mCreatedEditors[property].append(editor);
        mEditorToProperty[editor] = property;

        connect(editor, SIGNAL(filePathChanged(const QString &)),
                this, SLOT(slotSetValue(const QString &)));
        connect(editor, SIGNAL(destroyed(QObject *)),
                this, SLOT(slotEditorDestroyed(QObject *)));
        return editor;
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

    // Not resetting the child props of color property
    if(editor && property->isResettable())
    {
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
    if (!mCreatedEditors.contains(property))
        return;

    QList<FileEdit *> editors = mCreatedEditors[property];
    QListIterator<FileEdit *> itEditor(editors);
    while (itEditor.hasNext())
        itEditor.next()->setFilePath(value.toString());
}

void VariantEditorFactory::slotPropertyAttributeChanged(QtProperty *property,
                                                        const QString &attribute,
                                                        const QVariant &value)
{
    if (!mCreatedEditors.contains(property))
        return;

    if (attribute != QLatin1String("filter"))
        return;

    QList<FileEdit *> editors = mCreatedEditors[property];
    QListIterator<FileEdit *> itEditor(editors);
    while (itEditor.hasNext())
        itEditor.next()->setFilter(value.toString());
}

void VariantEditorFactory::slotSetValue(const QString &value)
{
    QObject *object = sender();
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor = mEditorToProperty.constBegin();
    while (itEditor != mEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            QtProperty *property = itEditor.value();
            QtVariantPropertyManager *manager = propertyManager(property);
            if (!manager)
                return;
            manager->setValue(property, value);
            return;
        }
        itEditor++;
    }
}

void VariantEditorFactory::slotEditorDestroyed(QObject *object)
{
    QMap<FileEdit *, QtProperty *>::ConstIterator itEditor = mEditorToProperty.constBegin();
    while (itEditor != mEditorToProperty.constEnd()) {
        if (itEditor.key() == object) {
            FileEdit *editor = itEditor.key();
            QtProperty *property = itEditor.value();
            mEditorToProperty.remove(editor);
            mCreatedEditors[property].removeAll(editor);
            if (mCreatedEditors[property].isEmpty())
                mCreatedEditors.remove(property);
            return;
        }
        itEditor++;
    }
}

} // namespace Internal
} // namespace Tiled

#include "varianteditorfactory.moc"

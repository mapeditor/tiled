/*
 * addpropertydialog.cpp
 * Copyright 2015, CaptainFrog <jwilliam.perreault@gmail.com>
 * Copyright 2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addpropertydialog.h"
#include "ui_addpropertydialog.h"

#include "preferences.h"
#include "properties.h"
#include "utils.h"

#include <QPushButton>
#include <QSettings>

using namespace Tiled;

static const char * const TYPE_KEY = "AddPropertyDialog/PropertyType";

AddPropertyDialog::AddPropertyDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new Ui::AddPropertyDialog)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    mUi->setupUi(this);
    resize(Utils::dpiScaled(size()));

    QString stringType = typeToName(QVariant::String);

    // Add possible types from QVariant
    mUi->typeBox->addItem(typeToName(QVariant::Bool),   false);
    mUi->typeBox->addItem(typeToName(QVariant::Color),  QColor());
    mUi->typeBox->addItem(typeToName(QVariant::Double), 0.0);
    mUi->typeBox->addItem(typeToName(filePathTypeId()), QVariant::fromValue(FilePath()));
    mUi->typeBox->addItem(typeToName(QVariant::Int),    0);
    mUi->typeBox->addItem(stringType,                   QString());

    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    // Restore previously used type
    Preferences *prefs = Preferences::instance();
    QSettings *s = prefs->settings();
    QString lastType = s->value(QLatin1String(TYPE_KEY), stringType).toString();

    mUi->typeBox->setCurrentText(lastType);

    connect(mUi->name, &QLineEdit::textChanged,
            this, &AddPropertyDialog::nameChanged);
    connect(mUi->typeBox, &QComboBox::currentTextChanged,
            this, &AddPropertyDialog::typeChanged);
}

AddPropertyDialog::~AddPropertyDialog()
{
    delete mUi;
}

QString AddPropertyDialog::propertyName() const
{
    return mUi->name->text();
}

QVariant AddPropertyDialog::propertyValue() const
{
    return mUi->typeBox->currentData();
}

void AddPropertyDialog::nameChanged(const QString &text)
{
    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!text.isEmpty());
}

void AddPropertyDialog::typeChanged(const QString &text)
{
    Preferences *prefs = Preferences::instance();
    QSettings *s = prefs->settings();
    s->setValue(QLatin1String(TYPE_KEY), text);
}

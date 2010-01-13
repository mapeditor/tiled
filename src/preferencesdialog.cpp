/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "languagemanager.h"
#include "preferences.h"

using namespace Tiled::Internal;

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::PreferencesDialog),
    mLanguages(LanguageManager::instance()->availableLanguages())
{
    mUi->setupUi(this);

    QStringList items;
    items.append(tr("System default"));

    foreach (const QString &name, mLanguages) {
        QLocale locale(name);
        QString string = QString(QLatin1String("%1 (%2)"))
            .arg(QLocale::languageToString(locale.language()))
            .arg(QLocale::countryToString(locale.country()));
        items.append(string);
    }

    mUi->languageCombo->addItems(items);

    fromPreferences();

    connect(mUi->languageCombo, SIGNAL(currentIndexChanged(int)),
            SLOT(languageSelected(int)));
}

PreferencesDialog::~PreferencesDialog()
{
    toPreferences();
    delete mUi;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange: {
            const int formatIndex = mUi->layerDataCombo->currentIndex();
            mUi->retranslateUi(this);
            mUi->layerDataCombo->setCurrentIndex(formatIndex);
            mUi->languageCombo->setItemText(0, tr("System default"));
        }
        break;
    default:
        break;
    }
}

void PreferencesDialog::languageSelected(int index)
{
    const int languageIndex = index - 1;
    const QString language =
            (languageIndex < 0) ? QString() : mLanguages.at(languageIndex);

    Preferences *prefs = Preferences::instance();
    prefs->setLanguage(language);
}

void PreferencesDialog::fromPreferences()
{
    const Preferences *prefs = Preferences::instance();
    mUi->reloadTilesetImages->setChecked(prefs->reloadTilesetsOnChange());
    mUi->enableDtd->setChecked(prefs->dtdEnabled());

    int formatIndex = 0;
    switch (prefs->layerDataFormat()) {
    case TmxMapWriter::XML:
        formatIndex = 0;
        break;
    case TmxMapWriter::Base64:
        formatIndex = 1;
        break;
    case TmxMapWriter::Base64Zlib:
        formatIndex = 3;
        break;
    default:
        formatIndex = 2;
        break;
    }
    mUi->layerDataCombo->setCurrentIndex(formatIndex);

    // Not found (-1) ends up at index 0, system default
    const int languageIndex = mLanguages.indexOf(prefs->language()) + 1;
    mUi->languageCombo->setCurrentIndex(languageIndex);
}

void PreferencesDialog::toPreferences()
{
    Preferences *prefs = Preferences::instance();

    prefs->setReloadTilesetsOnChanged(mUi->reloadTilesetImages->isChecked());
    prefs->setDtdEnabled(mUi->enableDtd->isChecked());
    prefs->setLayerDataFormat(layerDataFormat());
}

TmxMapWriter::LayerDataFormat PreferencesDialog::layerDataFormat() const
{
    switch (mUi->layerDataCombo->currentIndex()) {
    case 0:
        return TmxMapWriter::XML;
    case 1:
        return TmxMapWriter::Base64;
    case 3:
        return TmxMapWriter::Base64Zlib;
    default:
        return TmxMapWriter::Base64Gzip;
    }
}

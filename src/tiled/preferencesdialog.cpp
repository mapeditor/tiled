/*
 * preferencesdialog.cpp
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include "abstractobjecttool.h"
#include "languagemanager.h"
#include "mapobjectitem.h"
#include "mapview.h"
#include "pluginlistmodel.h"
#include "preferences.h"
#include "scriptmanager.h"
#include "sentryhelper.h"

#include <QDesktopServices>
#include <QSortFilterProxyModel>

using namespace Tiled;

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , mUi(new ::Ui::PreferencesDialog)
    , mLanguages(LanguageManager::instance()->availableLanguages())
{
    mUi->setupUi(this);

#if defined(QT_NO_OPENGL)
    mUi->openGL->setEnabled(false);
#else
    mUi->openGL->setEnabled(true);
#endif

    for (const QString &name : qAsConst(mLanguages)) {
        QLocale locale(name);
        QString string = QStringLiteral("%1 (%2)")
            .arg(QLocale::languageToString(locale.language()),
                 QLocale::countryToString(locale.country()));
        mUi->languageCombo->addItem(string, name);
    }

    mUi->languageCombo->model()->sort(0);
    mUi->languageCombo->insertItem(0, tr("System default"));

    mUi->styleCombo->addItems({ QApplication::translate("PreferencesDialog", "Native"),
                                QApplication::translate("PreferencesDialog", "Tiled Fusion") });

    mUi->styleCombo->setItemData(0, Preferences::SystemDefaultStyle);
    mUi->styleCombo->setItemData(1, Preferences::TiledStyle);

    mUi->objectSelectionBehaviorCombo->addItems({ tr("Select From Any Layer"),
                                                  tr("Prefer Selected Layers"),
                                                  tr("Selected Layers Only") });

    PluginListModel *pluginListModel = new PluginListModel(this);
    QSortFilterProxyModel *pluginProxyModel = new QSortFilterProxyModel(this);
    pluginProxyModel->setSortLocaleAware(true);
    pluginProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    pluginProxyModel->setSourceModel(pluginListModel);
    pluginProxyModel->sort(0);

    mUi->pluginList->setModel(pluginProxyModel);

    fromPreferences();

    auto *preferences = Preferences::instance();

    connect(mUi->reloadTilesetImages, &QCheckBox::toggled,
            preferences, &Preferences::setReloadTilesetsOnChanged);
    connect(mUi->restoreSession, &QCheckBox::toggled,
            preferences, &Preferences::setRestoreSessionOnStartup);
    connect(mUi->safeSaving, &QCheckBox::toggled,
            preferences, &Preferences::setSafeSavingEnabled);
    connect(mUi->exportOnSave, &QCheckBox::toggled,
            preferences, &Preferences::setExportOnSave);

    connect(mUi->embedTilesets, &QCheckBox::toggled, preferences, [preferences] (bool value) {
        preferences->setExportOption(Preferences::EmbedTilesets, value);
    });
    connect(mUi->detachTemplateInstances, &QCheckBox::toggled, preferences, [preferences] (bool value) {
        preferences->setExportOption(Preferences::DetachTemplateInstances, value);
    });
    connect(mUi->resolveObjectTypesAndProperties, &QCheckBox::toggled, preferences, [preferences] (bool value) {
        preferences->setExportOption(Preferences::ResolveObjectTypesAndProperties, value);
    });
    connect(mUi->minimizeOutput, &QCheckBox::toggled, preferences, [preferences] (bool value) {
        preferences->setExportOption(Preferences::ExportMinimized, value);
    });

#ifdef TILED_SENTRY
    connect(mUi->sendCrashReports, &QCheckBox::toggled, [] (bool value) {
        Sentry::instance()->setUserConsent(value ? Sentry::ConsentGiven : Sentry::ConsentRevoked);
    });
    mUi->crashReportingLabel->setOpenExternalLinks(true);
#else
    mUi->crashReporting->setVisible(false);
#endif

    connect(mUi->languageCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::languageSelected);
    connect(mUi->gridColor, &ColorButton::colorChanged,
            preferences, &Preferences::setGridColor);
    connect(mUi->backgroundFadeColor, &ColorButton::colorChanged,
            preferences, &Preferences::setBackgroundFadeColor);
    connect(mUi->gridFine, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            preferences, &Preferences::setGridFine);
    connect(mUi->gridMajorX, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            preferences, &Preferences::setGridMajorX);
    connect(mUi->gridMajorY, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            preferences, &Preferences::setGridMajorY);
    connect(mUi->objectLineWidth, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            preferences, &Preferences::setObjectLineWidth);
    connect(mUi->preciseTileObjectSelection, &QCheckBox::toggled,
            this, [] (bool checked) { MapObjectItem::preciseTileObjectSelection = checked; });
    connect(mUi->openGL, &QCheckBox::toggled,
            preferences, &Preferences::setUseOpenGL);
    connect(mUi->wheelZoomsByDefault, &QCheckBox::toggled,
            preferences, &Preferences::setWheelZoomsByDefault);
    connect(mUi->autoScrolling, &QCheckBox::toggled,
            this, [] (bool checked) { MapView::ourAutoScrollingEnabled = checked; });
    connect(mUi->smoothScrolling, &QCheckBox::toggled,
            this, [] (bool checked) { MapView::ourSmoothScrollingEnabled = checked; });

    connect(mUi->styleCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PreferencesDialog::styleComboChanged);
    connect(mUi->objectSelectionBehaviorCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [] (int index) { AbstractObjectTool::ourSelectionBehavior = static_cast<AbstractObjectTool::SelectionBehavior>(index); });
    connect(mUi->baseColor, &ColorButton::colorChanged,
            preferences, &Preferences::setBaseColor);
    connect(mUi->selectionColor, &ColorButton::colorChanged,
            preferences, &Preferences::setSelectionColor);

    connect(mUi->displayNewsCheckBox, &QCheckBox::toggled,
            preferences, &Preferences::setDisplayNews);
    connect(mUi->displayNewVersionCheckBox, &QCheckBox::toggled,
            preferences, &Preferences::setCheckForUpdates);

    connect(pluginListModel, &PluginListModel::setPluginEnabled,
            preferences, &Preferences::setPluginEnabled);

    const QString &extensionsPath = ScriptManager::instance().extensionsPath();
    mUi->extensionsPathEdit->setText(extensionsPath);
    mUi->openExtensionsPathButton->setEnabled(!extensionsPath.isEmpty());
    connect(mUi->openExtensionsPathButton, &QPushButton::clicked, this, [&] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(extensionsPath));
    });

    resize(sizeHint());
}

PreferencesDialog::~PreferencesDialog()
{
    delete mUi;
}

void PreferencesDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange: {
            mUi->retranslateUi(this);
            retranslateUi();
        }
        break;
    default:
        break;
    }
}

void PreferencesDialog::languageSelected(int index)
{
    const QString language = mUi->languageCombo->itemData(index).toString();
    Preferences *prefs = Preferences::instance();
    prefs->setLanguage(language);
}

void PreferencesDialog::fromPreferences()
{
    const Preferences *prefs = Preferences::instance();

    // General
    mUi->reloadTilesetImages->setChecked(prefs->reloadTilesetsOnChange());
    mUi->restoreSession->setChecked(prefs->restoreSessionOnStartup());
    mUi->safeSaving->setChecked(prefs->safeSavingEnabled());
    mUi->exportOnSave->setChecked(prefs->exportOnSave());

    mUi->embedTilesets->setChecked(prefs->exportOption(Preferences::EmbedTilesets));
    mUi->detachTemplateInstances->setChecked(prefs->exportOption(Preferences::DetachTemplateInstances));
    mUi->resolveObjectTypesAndProperties->setChecked(prefs->exportOption(Preferences::ResolveObjectTypesAndProperties));
    mUi->minimizeOutput->setChecked(prefs->exportOption(Preferences::ExportMinimized));

#ifdef TILED_SENTRY
    mUi->sendCrashReports->setChecked(Sentry::instance()->userConsent() == Sentry::ConsentGiven);
#endif

    // Interface
    mUi->preciseTileObjectSelection->setChecked(MapObjectItem::preciseTileObjectSelection);
    if (mUi->openGL->isEnabled())
        mUi->openGL->setChecked(prefs->useOpenGL());
    mUi->wheelZoomsByDefault->setChecked(prefs->wheelZoomsByDefault());
    mUi->autoScrolling->setChecked(MapView::ourAutoScrollingEnabled);
    mUi->smoothScrolling->setChecked(MapView::ourSmoothScrollingEnabled);

    // Not found (-1) ends up at index 0, system default
    int languageIndex = mUi->languageCombo->findData(prefs->language());
    if (languageIndex == -1)
        languageIndex = 0;
    mUi->languageCombo->setCurrentIndex(languageIndex);
    mUi->gridColor->setColor(prefs->gridColor());
    mUi->backgroundFadeColor->setColor(prefs->backgroundFadeColor());
    mUi->gridFine->setValue(prefs->gridFine());
    mUi->gridMajorX->setValue(prefs->gridMajor().width());
    mUi->gridMajorY->setValue(prefs->gridMajor().height());
    mUi->objectLineWidth->setValue(prefs->objectLineWidth());

    // Updates
    mUi->displayNewsCheckBox->setChecked(prefs->displayNews());
    mUi->displayNewVersionCheckBox->setChecked(prefs->checkForUpdates());

    // Theme
    int styleComboIndex = mUi->styleCombo->findData(prefs->applicationStyle());
    if (styleComboIndex == -1)
        styleComboIndex = 1;

    mUi->styleCombo->setCurrentIndex(styleComboIndex);
    mUi->objectSelectionBehaviorCombo->setCurrentIndex(AbstractObjectTool::ourSelectionBehavior);
    mUi->baseColor->setColor(prefs->baseColor());
    mUi->selectionColor->setColor(prefs->selectionColor());
    bool systemStyle = prefs->applicationStyle() == Preferences::SystemDefaultStyle;
    mUi->baseColor->setEnabled(!systemStyle);
    mUi->baseColorLabel->setEnabled(!systemStyle);
    mUi->selectionColor->setEnabled(!systemStyle);
    mUi->selectionColorLabel->setEnabled(!systemStyle);
}

void PreferencesDialog::retranslateUi()
{
    mUi->languageCombo->setItemText(0, tr("System default"));

    mUi->styleCombo->setItemText(0, QApplication::translate("PreferencesDialog", "Native"));
    mUi->styleCombo->setItemText(1, QApplication::translate("PreferencesDialog", "Tiled Fusion"));

    mUi->objectSelectionBehaviorCombo->setItemText(0, tr("Select From Any Layer"));
    mUi->objectSelectionBehaviorCombo->setItemText(1, tr("Prefer Selected Layers"));
    mUi->objectSelectionBehaviorCombo->setItemText(3, tr("Selected Layers Only"));
}

void PreferencesDialog::styleComboChanged()
{
    Preferences *prefs = Preferences::instance();
    int style = mUi->styleCombo->currentData().toInt();

    prefs->setApplicationStyle(static_cast<Preferences::ApplicationStyle>(style));

    bool systemStyle = prefs->applicationStyle() == Preferences::SystemDefaultStyle;
    mUi->baseColor->setEnabled(!systemStyle);
    mUi->baseColorLabel->setEnabled(!systemStyle);
    mUi->selectionColor->setEnabled(!systemStyle);
    mUi->selectionColorLabel->setEnabled(!systemStyle);
}

#include "moc_preferencesdialog.cpp"

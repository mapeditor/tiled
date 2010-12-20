/*
 * languagemanager.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "languagemanager.h"

#include "preferences.h"

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>

using namespace Tiled::Internal;

LanguageManager *LanguageManager::mInstance = 0;

LanguageManager *LanguageManager::instance()
{
    if (!mInstance)
        mInstance = new LanguageManager;
    return mInstance;
}

void LanguageManager::deleteInstance()
{
    delete mInstance;
    mInstance = 0;
}

LanguageManager::LanguageManager()
    : mQtTranslator(0)
    , mAppTranslator(0)
{
    mTranslationsDir = QCoreApplication::applicationDirPath();
#ifdef Q_OS_WIN32
    mTranslationsDir += QLatin1String("/translations");
#elif defined(Q_OS_MAC)
    mTranslationsDir += QLatin1String("/../Translations");
#else
    mTranslationsDir += QLatin1String("/../share/tiled/translations");
#endif
}

LanguageManager::~LanguageManager()
{
    delete mQtTranslator;
    delete mAppTranslator;
}

void LanguageManager::installTranslators()
{
    // Delete previous translators
    delete mQtTranslator;
    delete mAppTranslator;

    mQtTranslator = new QTranslator;
    mAppTranslator = new QTranslator;

    QString language = Preferences::instance()->language();
    if (language.isEmpty())
        language = QLocale::system().name();

    const QString qtTranslationsDir =
            QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    if (mQtTranslator->load(QLatin1String("qt_") + language,
                            qtTranslationsDir)) {
        QCoreApplication::installTranslator(mQtTranslator);
    } else {
        delete mQtTranslator;
        mQtTranslator = 0;
    }

    if (mAppTranslator->load(QLatin1String("tiled_") + language,
                             mTranslationsDir)) {
        QCoreApplication::installTranslator(mAppTranslator);
    } else {
        delete mAppTranslator;
        mAppTranslator = 0;
    }
}

QStringList LanguageManager::availableLanguages()
{
    if (mLanguages.isEmpty())
        loadAvailableLanguages();
    return mLanguages;
}

void LanguageManager::loadAvailableLanguages()
{
    mLanguages.clear();

    QStringList nameFilters;
    nameFilters.append(QLatin1String("tiled_*.qm"));

    QDirIterator iterator(mTranslationsDir, nameFilters,
                          QDir::Files | QDir::Readable);

    while (iterator.hasNext()) {
        iterator.next();
        const QString baseName = iterator.fileInfo().completeBaseName();
        // Cut off "tiled_" from the start
        mLanguages.append(baseName.mid(6));
    }
}

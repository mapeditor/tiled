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

using namespace Tiled;

LanguageManager *LanguageManager::instance()
{
    static LanguageManager instance;
    return &instance;
}

LanguageManager::LanguageManager()
    : mQtTranslator(nullptr)
    , mAppTranslator(nullptr)
{
    mTranslationsDir = QCoreApplication::applicationDirPath();
#if defined(TILED_WINDOWS_LAYOUT)
    mTranslationsDir += QStringLiteral("/translations");
#elif defined(Q_OS_MAC)
    mTranslationsDir += QStringLiteral("/../Translations");
#else
    mTranslationsDir += QStringLiteral("/../share/tiled/translations");
#endif
}

LanguageManager::~LanguageManager() = default;

void LanguageManager::installTranslators()
{
    mQtTranslator = std::make_unique<QTranslator>();
    mAppTranslator = std::make_unique<QTranslator>();

    const QString language = Preferences::instance()->language();
    const QLocale locale = language.isEmpty() ? QLocale()
                                              : QLocale(language);

    const QString qtTranslationsDir =
            QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    if (mQtTranslator->load(locale,
                            QStringLiteral("qt"),
                            QStringLiteral("_"),
                            qtTranslationsDir)) {
        QCoreApplication::installTranslator(mQtTranslator.get());
    }

    if (mAppTranslator->load(locale,
                             QStringLiteral("tiled"),
                             QStringLiteral("_"),
                             mTranslationsDir)) {
        QCoreApplication::installTranslator(mAppTranslator.get());
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
    nameFilters.append(QStringLiteral("tiled_*.qm"));

    QDirIterator iterator(mTranslationsDir, nameFilters,
                          QDir::Files | QDir::Readable);

    while (iterator.hasNext()) {
        iterator.next();
        const QString baseName = iterator.fileInfo().completeBaseName();
        // Cut off "tiled_" from the start
        mLanguages.append(baseName.mid(6));
    }
}

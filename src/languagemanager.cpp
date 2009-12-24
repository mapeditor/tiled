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
    mTranslationsDir += QLatin1String("/../translations");
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
    mLanguages.append(QLatin1String("en_US"));

    QDirIterator iterator(mTranslationsDir);
    while (iterator.hasNext()) {
        const QString fileName = iterator.next();
        if (!fileName.endsWith(QLatin1String(".qm")))
            continue;

        const QString baseName = iterator.fileInfo().completeBaseName();
        if (!baseName.startsWith(QLatin1String("tiled_")))
            continue;

        // Cut off "tiled_" from the start
        mLanguages.append(baseName.mid(6));
    }
}

/*
 * commandlineparser.cpp
 * Copyright 2011, Ben Longbons <b.r.longbons@gmail.com>
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "commandlineparser.h"

#include <QDebug>
#include <QFileInfo>

using namespace Tiled;
using namespace Tiled::Internal;

CommandLineParser::CommandLineParser()
    : mLongestArgument(0)
    , mShowHelp(false)
{
}

void CommandLineParser::registerOption(Callback callback,
                                       void *data,
                                       QChar shortName,
                                       const QString &longName,
                                       const QString &help)
{
    mOptions.append(Option(callback, data, shortName, longName, help));

    const int length = longName.length();
    if (mLongestArgument < length)
        mLongestArgument = length;
}

bool CommandLineParser::parse(const QStringList &arguments)
{
    mFilesToOpen.clear();
    mShowHelp = false;

    QStringList todo = arguments;
    mCurrentProgramName = QFileInfo(todo.takeFirst()).fileName();

    int index = 0;
    bool noMoreArguments = false;

    while (!todo.isEmpty()) {
        index++;
        const QString arg = todo.takeFirst();

        if (arg.isEmpty())
            continue;

        if (noMoreArguments || arg.at(0) != QLatin1Char('-')) {
            mFilesToOpen.append(arg);
            continue;
        }

        if (arg.length() == 1) {
            // Traditionally a single hyphen means read file from stdin,
            // write file to stdout. This isn't supported right now.
            qWarning().nospace() << "Bad argument " << index
                                 << ": lonely hyphen";
            showHelp();
            return false;
        }

        // Long options
        if (arg.at(1) == QLatin1Char('-')) {
            // Double hypen "--" means no more options will follow
            if (arg.length() == 2) {
                noMoreArguments = true;
                continue;
            }

            if (!handleLongOption(arg)) {
                qWarning().nospace() << "Unknown long argument " << index
                                     << ": " << arg;
                mShowHelp = true;
                break;
            }

            continue;
        }

        // Short options
        for (int i = 1; i < arg.length(); ++i) {
            const QChar c = arg.at(i);
            if (!handleShortOption(c)) {
                qWarning().nospace() << "Unknown short argument " << index
                                     << '.' << i << ": " << c;
                mShowHelp = true;
                break;
            }
        }
    }

    if (mShowHelp) {
        showHelp();
        return false;
    }

    return true;
}

void CommandLineParser::showHelp()
{
    // TODO: Make translatable
    qWarning().nospace() << "Usage:\n"
                         << "  " << qPrintable(mCurrentProgramName)
                         << " [options] [files...]\n\n"
                         << "Options:";

    qWarning("  -h %-*s : Display this help", mLongestArgument, "--help");

    foreach (const Option &option, mOptions) {
        if (!option.shortName.isNull()) {
            qWarning("  -%c %-*s : %s",
                     option.shortName.toLatin1(),
                     mLongestArgument, qPrintable(option.longName),
                     qPrintable(option.help));
        } else {
            qWarning("     %-*s : %s",
                     mLongestArgument, qPrintable(option.longName),
                     qPrintable(option.help));

        }
    }

    qWarning();
}

bool CommandLineParser::handleLongOption(const QString &longName)
{
    if (longName == QLatin1String("--help")) {
        mShowHelp = true;
        return true;
    }

    foreach (const Option &option, mOptions) {
        if (longName == option.longName) {
            option.callback(option.data);
            return true;
        }
    }

    return false;
}

bool CommandLineParser::handleShortOption(QChar c)
{
    if (c == QLatin1Char('h')) {
        mShowHelp = true;
        return true;
    }

    foreach (const Option &option, mOptions) {
        if (c == option.shortName) {
            option.callback(option.data);
            return true;
        }
    }

    return false;
}

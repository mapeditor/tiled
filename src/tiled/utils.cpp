/*
 * utils.cpp
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

#include "utils.h"

#include "mapformat.h"
#include "preferences.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QJsonDocument>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QRegExp>
#include <QScreen>

#include "qtcompat_p.h"

static QString toImageFileFilter(const QList<QByteArray> &formats)
{
    QString filter(QCoreApplication::translate("Utils", "Image files"));
    filter += QLatin1String(" (");
    bool first = true;
    for (const QByteArray &format : formats) {
        if (!first)
            filter += QLatin1Char(' ');
        first = false;
        filter += QLatin1String("*.");
        filter += QString::fromLatin1(format.toLower());
    }
    filter += QLatin1Char(')');
    return filter;
}

namespace Tiled {
namespace Utils {

/**
 * Returns a file dialog filter that matches all readable image formats.
 *
 * This includes all supported map formats, which are rendered to an image when
 * used in this context.
 */
QString readableImageFormatsFilter()
{
    auto imageFilter = toImageFileFilter(QImageReader::supportedImageFormats());

    FormatHelper<MapFormat> helper(FileFormat::Read, imageFilter);
    return helper.filter();
}

/**
 * Returns a file dialog filter that matches all writable image formats.
 */
QString writableImageFormatsFilter()
{
    return toImageFileFilter(QImageWriter::supportedImageFormats());
}

// Makes a list of filters from a normal filter string "Image Files (*.png *.jpg)"
//
// Copied from qplatformdialoghelper.cpp in Qt, used under the terms of the GPL
// version 2.0.
QStringList cleanFilterList(const QString &filter)
{
    const char filterRegExp[] =
    "^(.*)\\(([a-zA-Z0-9_.,*? +;#\\-\\[\\]@\\{\\}/!<>\\$%&=^~:\\|]*)\\)$";

    QRegExp regexp(QString::fromLatin1(filterRegExp));
    Q_ASSERT(regexp.isValid());
    QString f = filter;
    int i = regexp.indexIn(f);
    if (i >= 0)
        f = regexp.cap(2);
    return f.split(QLatin1Char(' '), QString::SkipEmptyParts);
}

/**
 * Returns whether the \a fileName has an extension that is matched by
 * the \a nameFilter.
 */
bool fileNameMatchesNameFilter(const QString &fileName,
                               const QString &nameFilter)
{
    QRegExp rx;
    rx.setCaseSensitivity(Qt::CaseInsensitive);
    rx.setPatternSyntax(QRegExp::Wildcard);

    const QStringList filterList = cleanFilterList(nameFilter);
    for (const QString &filter : filterList) {
        rx.setPattern(filter);
        if (rx.exactMatch(fileName))
            return true;
    }
    return false;
}

QString firstExtension(const QString &nameFilter)
{
    QString extension;

    const auto filterList = cleanFilterList(nameFilter);
    if (!filterList.isEmpty()) {
        extension = filterList.first();
        extension.remove(QLatin1Char('*'));
    }

    return extension;
}

struct Match {
    int wordIndex;
    int stringIndex;
};

/**
 * Matches the given \a word against the \a string. The match is a fuzzy one,
 * being case-insensitive and allowing any characters to appear between the
 * characters of the given word.
 *
 * Attempts to make matching indexes sequential.
 */
static bool matchingIndexes(const QString &word, QStringRef string, QVarLengthArray<Match, 16> &matchingIndexes)
{
    int index = 0;

    for (int i = 0; i < word.size(); ++i) {
        const QChar c = word.at(i);

        int newIndex = string.indexOf(c, index, Qt::CaseInsensitive);
        if (newIndex == -1)
            return false;

        // If the new match is not sequential, check if we can make it
        // sequential by moving a previous match forward
        if (newIndex != index) {
            for (int offset = 1; matchingIndexes.size() >= offset; ++offset) {
                int backTrackIndex = newIndex - offset;
                Match &match = matchingIndexes[matchingIndexes.size() - offset];

                const int previousIndex = string.lastIndexOf(string.at(match.stringIndex), backTrackIndex, Qt::CaseInsensitive);

                if (previousIndex == backTrackIndex)
                    match.stringIndex = previousIndex;
                else
                    break;
            }
        }

        matchingIndexes.append({ i, newIndex });
        index = newIndex + 1;
    }

    return true;
}

/**
 * Rates the match between \a word and \a string with a score indicating the
 * strength of the match, for sorting purposes.
 *
 * A score of 0 indicates there is no match.
 */
static int matchingScore(const QString &word, QStringRef string)
{
    QVarLengthArray<Match, 16> indexes;
    if (!matchingIndexes(word, string, indexes))
        return 0;

    int score = 1;  // empty word matches
    int previousIndex = -1;

    for (const Match &match : qAsConst(indexes)) {
        const int start = match.stringIndex == 0;
        const int sequential = match.stringIndex == previousIndex + 1;

        const auto c = word.at(match.wordIndex);
        const int caseMatch = c.isUpper() && string.at(match.stringIndex) == c;

        score += 1 + start + sequential + caseMatch;
        previousIndex = match.stringIndex;
    }

    return score;
}

static bool matchingRanges(const QString &word, QStringRef string, int offset, RangeSet<int> &result)
{
    QVarLengthArray<Match, 16> indexes;
    if (!matchingIndexes(word, string, indexes))
        return false;

    for (const Match &match : qAsConst(indexes))
        result.insert(match.stringIndex + offset);

    return true;
}

int matchingScore(const QStringList &words, QStringRef string)
{
    const QStringRef fileName = string.mid(string.lastIndexOf(QLatin1Char('/')) + 1);

    int totalScore = 1;     // no words matches everything

    for (const QString &word : words) {
        if (int score = Utils::matchingScore(word, fileName)) {
            // Higher score if file name matches
            totalScore += score * 2;
        } else if ((score = Utils::matchingScore(word, string))) {
            totalScore += score;
        } else {
            totalScore = 0;
            break;
        }
    }

    return totalScore;
}

RangeSet<int> matchingRanges(const QStringList &words, QStringRef string)
{
    const int startOfFileName = string.lastIndexOf(QLatin1Char('/')) + 1;
    const QStringRef fileName = string.mid(startOfFileName);

    RangeSet<int> result;

    for (const QString &word : words) {
        if (!matchingRanges(word, fileName, startOfFileName, result))
            matchingRanges(word, string, 0, result);
    }

    return result;
}


/**
 * Restores a widget's geometry.
 * Requires the widget to have its object name set.
 */
void restoreGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    const auto preferences = Preferences::instance();

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    widget->restoreGeometry(preferences->value(key).toByteArray());

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget)) {
        const QString stateKey = widget->objectName() + QLatin1String("/State");
        mainWindow->restoreState(preferences->value(stateKey).toByteArray());
    }
}

/**
 * Saves a widget's geometry.
 * Requires the widget to have its object name set.
 */
void saveGeometry(QWidget *widget)
{
    Q_ASSERT(!widget->objectName().isEmpty());

    auto preferences = Preferences::instance();

    const QString key = widget->objectName() + QLatin1String("/Geometry");
    preferences->setValue(key, widget->saveGeometry());

    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(widget)) {
        const QString stateKey = widget->objectName() + QLatin1String("/State");
        preferences->setValue(stateKey, mainWindow->saveState());
    }
}

int defaultDpi()
{
    static int dpi = []{
        if (const QScreen *screen = QGuiApplication::primaryScreen())
            return static_cast<int>(screen->logicalDotsPerInchX());
#ifdef Q_OS_MAC
        return 72;
#else
        return 96;
#endif
    }();
    return dpi;
}

qreal defaultDpiScale()
{
    static qreal scale = []{
        if (const QScreen *screen = QGuiApplication::primaryScreen())
            return screen->logicalDotsPerInchX() / 96.0;
        return 1.0;
    }();
    return scale;
}

qreal dpiScaled(qreal value)
{
#ifdef Q_OS_MAC
    // On mac the DPI is always 72 so we should not scale it
    return value;
#else
    static const qreal scale = defaultDpiScale();
    return value * scale;
#endif
}

int dpiScaled(int value)
{
    return qRound(dpiScaled(qreal(value)));
}

QSize dpiScaled(QSize value)
{
    return QSize(dpiScaled(value.width()),
                 dpiScaled(value.height()));
}

QPoint dpiScaled(QPoint value)
{
    return QPoint(dpiScaled(value.x()),
                  dpiScaled(value.y()));
}

QRectF dpiScaled(QRectF value)
{
    return QRectF(dpiScaled(value.x()),
                  dpiScaled(value.y()),
                  dpiScaled(value.width()),
                  dpiScaled(value.height()));
}

QSize smallIconSize()
{
    static QSize size = dpiScaled(QSize(16, 16));
    return size;
}

bool isZoomInShortcut(QKeyEvent *event)
{
    if (event->matches(QKeySequence::ZoomIn))
        return true;
    if (event->key() == Qt::Key_Plus)
        return true;
    if (event->key() == Qt::Key_Equal)
        return true;

    return false;
}

bool isZoomOutShortcut(QKeyEvent *event)
{
    if (event->matches(QKeySequence::ZoomOut))
        return true;
    if (event->key() == Qt::Key_Minus)
        return true;
    if (event->key() == Qt::Key_Underscore)
        return true;

    return false;
}

bool isResetZoomShortcut(QKeyEvent *event)
{
    if (event->key() == Qt::Key_0 && event->modifiers() & Qt::ControlModifier)
        return true;

    return false;
}

/*
 * Code based on FileUtils::showInGraphicalShell from Qt Creator
 * Copyright (C) 2016 The Qt Company Ltd.
 * Used under the terms of the GNU General Public License version 3
 */
static void showInFileManager(const QString &fileName)
{
    // Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
    QStringList param;
    if (!QFileInfo(fileName).isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileName);
    QProcess::startDetached(QLatin1String("explorer.exe"), param);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(fileName);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
    // We cannot select a file here, because xdg-open would open the file
    // instead of the file browser...
    QProcess::startDetached(QString(QLatin1String("xdg-open")),
                            QStringList(QFileInfo(fileName).absolutePath()));
#endif
}

void addFileManagerActions(QMenu &menu, const QString &fileName)
{
    if (fileName.isEmpty())
        return;

    menu.addAction(QCoreApplication::translate("Utils", "Copy File Path"), [fileName] {
        QApplication::clipboard()->setText(QDir::toNativeSeparators(fileName));
    });

    addOpenContainingFolderAction(menu, fileName);
}

void addOpenContainingFolderAction(QMenu &menu, const QString &fileName)
{
    menu.addAction(QCoreApplication::translate("Utils", "Open Containing Folder..."), [fileName] {
        showInFileManager(fileName);
    });
}

void addOpenWithSystemEditorAction(QMenu &menu, const QString &fileName)
{
    menu.addAction(QCoreApplication::translate("Utils", "Open with System Editor"), [=] {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
    });
}

static bool readJsonFile(QIODevice &device, QSettings::SettingsMap &map)
{
    QJsonParseError error;
    map = QJsonDocument::fromJson(device.readAll(), &error).toVariant().toMap();
    return error.error == QJsonParseError::NoError;
}

static bool writeJsonFile(QIODevice &device, const QSettings::SettingsMap &map)
{
    const auto json = QJsonDocument { QJsonObject::fromVariantMap(map) }.toJson();
    return device.write(json) == json.size();
}

QSettings::Format jsonSettingsFormat()
{
    static const auto format = QSettings::registerFormat(QStringLiteral("json"),
                                                         readJsonFile,
                                                         writeJsonFile);
    return format;
}

std::unique_ptr<QSettings> jsonSettings(const QString &fileName)
{
    return std::make_unique<QSettings>(fileName, jsonSettingsFormat());
}

} // namespace Utils
} // namespace Tiled

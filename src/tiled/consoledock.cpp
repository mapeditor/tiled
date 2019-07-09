/*
 * consoledialog.cpp
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
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

#include "consoledock.h"

#include "logginginterface.h"
#include "pluginmanager.h"
#include "preferences.h"
#include "scriptmanager.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSettings>
#include <QShortcut>
#include <QVBoxLayout>

namespace Tiled {

ConsoleDock::ConsoleDock(QWidget *parent)
    : QDockWidget(parent)
    , mPlainTextEdit(new QPlainTextEdit)
    , mLineEdit(new QLineEdit)
{
    setObjectName(QLatin1String("ConsoleDock"));
    setWindowTitle(tr("Console"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(0);

    mPlainTextEdit->setReadOnly(true);

    QPalette p = mPlainTextEdit->palette();
    p.setColor(QPalette::Base, Qt::black);
    p.setColor(QPalette::Text, Qt::lightGray);
    mPlainTextEdit->setPalette(p);

    mLineEdit->setPlaceholderText(tr("Execute script"));
    mLineEdit->setClearButtonEnabled(true);
    connect(mLineEdit, &QLineEdit::returnPressed,
            this, &ConsoleDock::executeScript);

    auto previousShortcut = new QShortcut(Qt::Key_Up, mLineEdit, nullptr, nullptr, Qt::WidgetShortcut);
    connect(previousShortcut, &QShortcut::activated, [this] { moveHistory(-1); });

    auto nextShortcut = new QShortcut(Qt::Key_Down, mLineEdit, nullptr, nullptr, Qt::WidgetShortcut);
    connect(nextShortcut, &QShortcut::activated, [this] { moveHistory(1); });

    layout->addWidget(mPlainTextEdit);
    layout->addWidget(mLineEdit);

    const auto outputs = PluginManager::objects<LoggingInterface>();
    for (LoggingInterface *output : outputs)
        registerOutput(output);

    connect(PluginManager::instance(), &PluginManager::objectAdded,
            this, &ConsoleDock::onObjectAdded);

    setWidget(widget);

    QSettings *settings = Preferences::instance()->settings();
    mHistory = settings->value(QStringLiteral("Console/History")).toStringList();
    mHistoryPosition = mHistory.size();
}

ConsoleDock::~ConsoleDock()
{
}

void ConsoleDock::appendInfo(const QString &str)
{
    mPlainTextEdit->appendHtml(QLatin1String("<pre>") + str.toHtmlEscaped() +
                               QLatin1String("</pre>"));
}

void ConsoleDock::appendError(const QString &str)
{
    mPlainTextEdit->appendHtml(QLatin1String("<pre style='color:red'>") + str.toHtmlEscaped() +
                               QLatin1String("</pre>"));
}

void ConsoleDock::appendScript(const QString &str)
{
    mPlainTextEdit->appendHtml(QLatin1String("<pre style='color:lightgreen'>&gt; ") + str.toHtmlEscaped() +
                               QLatin1String("</pre>"));
}

void ConsoleDock::onObjectAdded(QObject *object)
{
    if (LoggingInterface *output = qobject_cast<LoggingInterface*>(object))
        registerOutput(output);
}

void ConsoleDock::executeScript()
{
    const QString script = mLineEdit->text();
    if (script.isEmpty())
        return;

    appendScript(script);

    const QJSValue result = ScriptManager::instance().evaluate(script);
    if (!result.isError() && !result.isUndefined())
        appendInfo(result.toString());

    mLineEdit->clear();

    mHistory.append(script);
    mHistoryPosition = mHistory.size();

    // Remember the last few script lines
    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QStringLiteral("Console/History"),
                       QStringList(mHistory.mid(mHistory.size() - 10)));
}

void ConsoleDock::moveHistory(int direction)
{
    int newPosition = qBound(0, mHistoryPosition + direction, mHistory.size());
    if (newPosition == mHistoryPosition)
        return;

    if (newPosition < mHistory.size())
        mLineEdit->setText(mHistory.at(newPosition));
    else
        mLineEdit->clear();

    mHistoryPosition = newPosition;
}

void ConsoleDock::registerOutput(LoggingInterface *output)
{
    connect(output, &LoggingInterface::info, this, &ConsoleDock::appendInfo);
    connect(output, &LoggingInterface::error, this, &ConsoleDock::appendError);
}

} // namespace Tiled

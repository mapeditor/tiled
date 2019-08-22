/*
 * consoledock.h
 * Copyright 2013, Samuli Tuomola <samuli.tuomola@gmail.com>
 * Copyright 2018-2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include <QDockWidget>

class QLineEdit;
class QPlainTextEdit;

namespace Tiled {

class ConsoleDock : public QDockWidget
{
    Q_OBJECT

public:
    explicit ConsoleDock(QWidget *parent = nullptr);
    ~ConsoleDock();

protected:
    void changeEvent(QEvent *e) override;

private:
    void appendInfo(const QString &str);
    void appendWarning(const QString &str);
    void appendError(const QString &str);
    void appendScript(const QString &str);

    void executeScript();

    void moveHistory(int direction);

    void retranslateUi();

    QPlainTextEdit *mPlainTextEdit;
    QLineEdit *mLineEdit;
    QStringList mHistory;
    int mHistoryPosition = 0;
};

} // namespace Tiled

/*
 * rtbtutorialdock.h
 * Copyright 2016, David Stammer
 *
 * This file is part of Road to Ballhalla Editor.
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

#ifndef RTBTUTORIALDOCK_H
#define RTBTUTORIALDOCK_H

#include <QDockWidget>

class QTextEdit;
class QPushButton;

namespace Tiled {

namespace Internal {

class RTBTutorialDock : public QDockWidget
{
    Q_OBJECT

public:
    RTBTutorialDock(QWidget *parent = 0);

    ~RTBTutorialDock();

    void setText(QString text);

    bool isNextButtonEnabled();
    bool isPrevButtonEnabled();

    void setNextButtonEnabled(bool enable);
    void setPrevButtonEnabled(bool enable);
    void setStartButtonText(int type);

    enum ButtonType {
        Start,
        Continue
    };

signals:
    void nextButtonPressed();
    void prevButtonPressed();
    void pauseButtonPressed(bool isRunning);

private slots:
    void emitNextButtonPressed();
    void emitPrevButtonPressed();
    void emitPauseButtonPressed();

protected:
    void closeEvent(QCloseEvent *event);

private:
    void retranslateUi();

    QTextEdit *mTextField;
    QPushButton *mPauseButton;
    QPushButton *mPrevButton;
    QPushButton *mNextButton;
    bool isTutorialRunning;
    QString mStartButtonText;

};


} // namespace Internal
} // namespace Tiled

#endif // RTBTUTORIALDOCK_H

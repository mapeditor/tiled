/*
 * rtbtutorialdock.cpp
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

#include "rtbtutorialdock.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>

static const QString BUTTON_START = QLatin1String("Start");
static const QString BUTTON_CONTINUE = QLatin1String("Continue");
static const QString BUTTON_PAUSE = QLatin1String("Pause");
static const QString BUTTON_PREVIOUS = QLatin1String("Previous");
static const QString BUTTON_NEXT = QLatin1String("Next");

using namespace Tiled;
using namespace Tiled::Internal;


RTBTutorialDock::RTBTutorialDock(QWidget *parent)
    : QDockWidget(parent)
    , isTutorialRunning(false)
    , mStartButtonText(BUTTON_START)
{
    setObjectName(QLatin1String("TutorialDock"));

    QWidget *w = new QWidget(this);

    QVBoxLayout *vertical = new QVBoxLayout(w);
    vertical->setSpacing(0);
    vertical->setMargin(2);

    mTextField = new QTextEdit(this);
    mTextField->setReadOnly(true);
    vertical->addWidget(mTextField);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    vertical->addLayout(buttonLayout);

    mPrevButton = new QPushButton(BUTTON_PREVIOUS, this);
    mPrevButton->setEnabled(false);
    connect(mPrevButton, SIGNAL(clicked(bool)), this, SLOT(emitPrevButtonPressed()));
    buttonLayout->addWidget(mPrevButton);

    mPauseButton = new QPushButton(mStartButtonText, this);
    connect(mPauseButton, SIGNAL(clicked(bool)), this, SLOT(emitPauseButtonPressed()));
    buttonLayout->addWidget(mPauseButton);

    mNextButton = new QPushButton(BUTTON_NEXT, this);
    mNextButton->setEnabled(false);
    connect(mNextButton, SIGNAL(clicked(bool)), this, SLOT(emitNextButtonPressed()));
    buttonLayout->addWidget(mNextButton);

    setWidget(w);
    retranslateUi();

}

RTBTutorialDock::~RTBTutorialDock()
{

}

void RTBTutorialDock::retranslateUi()
{
    setWindowTitle(tr("Tutorial"));
    mPrevButton->setText(tr(BUTTON_PREVIOUS.toStdString().c_str()));
    mNextButton->setText(tr(BUTTON_NEXT.toStdString().c_str()));
    mPauseButton->setText(tr(mPauseButton->text().toStdString().c_str()));
}

void RTBTutorialDock::setText(QString text)
{
    mTextField->setHtml(text);
}

void RTBTutorialDock::emitNextButtonPressed()
{
    emit nextButtonPressed();
}

void RTBTutorialDock::emitPrevButtonPressed()
{
    emit prevButtonPressed();
}

void RTBTutorialDock::emitPauseButtonPressed()
{
    if(isTutorialRunning)
    {
        isTutorialRunning = false;
        mPauseButton->setText(mStartButtonText);

        mPrevButton->setEnabled(false);
        mNextButton->setEnabled(false);
    }
    else
    {
        isTutorialRunning = true;
        mPauseButton->setText(BUTTON_PAUSE);

        mPrevButton->setEnabled(true);
        mNextButton->setEnabled(true);
    }

    emit pauseButtonPressed(isTutorialRunning);
}

bool RTBTutorialDock::isNextButtonEnabled()
{
    return mNextButton->isEnabled();
}

bool RTBTutorialDock::isPrevButtonEnabled()
{
    return mPrevButton->isEnabled();
}

void RTBTutorialDock::setNextButtonEnabled(bool enable)
{
    mNextButton->setEnabled(enable);
}

void RTBTutorialDock::setPrevButtonEnabled(bool enable)
{
    mPrevButton->setEnabled(enable);
}

void RTBTutorialDock::setStartButtonText(int type)
{
    if(type == Start){
        mStartButtonText = BUTTON_START;
    } else if(type == Continue) {
        mStartButtonText = BUTTON_CONTINUE;
    }

    if(!isTutorialRunning){
        mPauseButton->setText(mStartButtonText);
    }
}

void RTBTutorialDock::closeEvent(QCloseEvent *event)
{
    isTutorialRunning = false;
    mPauseButton->setText(mStartButtonText);

    mPrevButton->setEnabled(false);
    mNextButton->setEnabled(false);

    emit pauseButtonPressed(false);

    QDockWidget::closeEvent(event);
}

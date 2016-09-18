/*
 * rtbtutorial.h
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

#ifndef RTBTUTORIAL_H
#define RTBTUTORIAL_H

#include <QObject>
#include <QAbstractButton>
#include <QMessageBox>

class HighlightSectionItem;

namespace Tiled {
namespace Internal {

class RTBTutorialDock;

class RTBTutorial : public QObject
{
    Q_OBJECT

public:
    RTBTutorial(RTBTutorialDock *tutorialDock, QWidget *parentWidget = 0);

    ~RTBTutorial();

    void retranslate();

    enum Chapter {
            Start,
            CreateMap,
            Map,
            MapProperties,
            Toolbar,
            Layer,
            Objects,
            ObjectProperties,
            Play,
            Validator,
            End
    };

    enum Section {
            None,
            MainToolBar,
            ToolsToolBar,
            LayerDock,
            PropertiesDock,
            ValidatorDock,
            MapView
    };

signals:
    void highlightSection(int);

public slots:
    void changeChapter();

    void next();
    void previous();
    void pause(bool isRunning);

private:
    QString tileImage(int id);
    void setText(int id);
    void activateHighlight();

    QWidget *mParentWidget;
    int mCurrentChapter;
    bool mIsRunning;

    RTBTutorialDock *mTutorialDock;
};

#endif // RTBTUTORIAL_H

}
}

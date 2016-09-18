/*
 * rtbtutorial.cpp
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

#include "rtbtutorial.h"

#include "preferences.h"
#include "rtbtutorialdock.h"
#include "rtbmapsettings.h"

#include <QBuffer>

using namespace Tiled;
using namespace Internal;

static const char* CHAPTER_START = "<b>Welcome to the Road to Ballhalla Editor Tutorial!</b>"
                                    "<br><br>This tutorial will guide you through the steps of creating a simple map for Road to Ballhalla."
                                    "<br><br>If you're done with the current page of the tutorial, just click on \"Next\" at the bottom of this text area to continue.";
static const char* CHAPTER_CREATE_MAP = "<b>Map Toolbar</b>"
                                        "<br><br>In the toolbar at the top, among others you'll find buttons to create, modify and play maps."
                                        "<br><br><b>Create a new map, step by step:</b>"
                                        "<br>1. Click on \"New\". <img src=\":images/24x24/document-new.png\" height=\"16\" width=\"16\">"
                                        "<br>2. Enter a level name (at least 5 characters)."
                                        "<br>3. Click \"OK\"."
                                        "<br><br><i>Hints:</i>"
                                        "<br>- Don't worry - all settings in this dialog can be changed afterwards.";
static const char* CHAPTER_MAP = "<b>Map</b>"
                                 "<br><br>The marked area displays the actual map you're creating. Here you can add, edit, move or remove tiles and objects."
                                 "<br><br>You can load multiple maps at once and switch between them using the tabs above the map area."
                                 "<br><br>As \"Add Starter Content\" was (hopefully) selected when you created the new map, both a Start Location <img src=\"data:image/png;base64, %1 height=\"16\" width=\"16\"> and a Finish Hole <img src=\"data:image/png;base64, %2 height=\"16\" width=\"16\"> are already placed on the map's floor.";
static const char* CHAPTER_MAP_PROP = "<b>Properties (Map)</b>"
                                        "<br><br>This area displays the properties of the map or the currently selected layer or object."
                                        "<br><br>Properties are explained in tooltips that appear when you hover over the property name. You can reset properties to their default value at any time by clicking on the brush icon <img src=\":images/24x24/edit-clear.png\" height=\"16\" width=\"16\"> next to it."
                                        "<br><br>You can open the map properties at any time by clicking on the \"Show Map Properties\" (P) <img src=\":images/24x24/document-properties.png\" height=\"16\" width=\"16\"> button. Map properties include general settings as well as settings for the colors, camera, music and more.";
static const char* CHAPTER_TOOLBAR = "<b>Creation Toolbar (Floor Layer)</b>"
                                        "<br><br>This area provides the tools for you to create and modify the actual content of your map."
                                        "<br><br><b>Draw Floor, step by step:</b>"
                                        "<br>1. Click on the \"Floor\" <img src=\"data:image/png;base64, %1 height=\"16\" width=\"16\"> button, which will select the \"Floor\" brush."
                                        "<br>2. Try to extend the grey floor as seen in the screenshot below. If you messed up, use the \"Eraser\" <img src=\":images/22x22/stock-tool-eraser.png\" height=\"16\" width=\"16\"> to remove existing floor tiles. To continue drawing, use the \"Stamp Brush\" <img src=\":images/22x22/stock-tool-clone.png\" height=\"16\" width=\"16\">."
                                        "<br><br><i>Hints:</i>"
                                        "<br>- You can quickly change the brush by right-clicking on an existing tile of that brush in the map (instead of selecting its brush in the toolbar)."
                                        "<br><br><img src=\"://rtb_resources/tutorial/tutorialFloor.png\" >";
static const char* CHAPTER_LAYER = "<b>Layers</b>"
                                    "<br><br>A map consists of 3 layers: Floor, Objects and Orbs. You can switch between them in the layers area, or by pressing the number keys \"1\", \"2\" or \"3\". Each layer has its own set of tools to modify the map."
                                    "<br><br>- <b>Floor:</b> On this layer you can create the floor of the map, which includes normal (grey) floor, red floor, speedpads and more."
                                    "<br>- <b>Objects:</b> On this layer you can add gameplay objects like buttons, laser beams and teleporters."
                                    "<br>- <b>Orbs:</b> On this layer you can place orbs that the player can collect."
                                    "<br><br><b>Switch to the \"Objects\" layer by clicking on \"Objects (2)\" in the layers area.</b>"
                                    "<br><br><i>Hints:</i>"
                                    "<br>- Normally, your actions only apply to the currently active layer. However, you can select and move, copy or delete parts of your map on all layers at once using the \"Select Area\" <img src=\":images/22x22/stock-tool-rect-select.png\" height=\"16\" width=\"16\"> tool.";
static const char* CHAPTER_OBJECTS = "<b>Creation Toolbar (Objects Layer)</b>"
                                        "<br><br>With the objects layer selected, you can see all actions that apply to it in the toolbar."
                                        "<br><br><b>Add Gameplay Objects, step by step:</b>"
                                        "<br>1. Select the object \"Teleporter\" <img src=\"data:image/png;base64, %3 height=\"16\" width=\"16\">."
                                        "<br>2. Place it at the top-right corner of the grey floor."
                                        "<br>3. Select and place the other objects as seen in the screenshot below: a \"Button\" <img src=\"data:image/png;base64, %2 height=\"16\" width=\"16\">, two \"Laser Beam Left\" <img src=\"data:image/png;base64, %1 height=\"16\" width=\"16\">, and a \"Target\" <img src=\"data:image/png;base64, %4 height=\"16\" width=\"16\">."
                                        "<br><br><i>Note:</i> If you are saving your map file now, there will be a validation error and a warning - you can ignore both for now."
                                        "<br><br><img src=\"://rtb_resources/tutorial/tutorialObjects.png\" >";
static const char* CHAPTER_OBJECT_PROP = "<b>Properties (Objects)</b>"
                                            "<br><br>An object can have one or multiple properties you can set."
                                            "<br><br><b>Let the Button deactivate the Laser Beams, step by step:</b>"
                                            "<br>1. In the map (i.e. not in the toolbar), click on the button <img src=\"data:image/png;base64, %1 height=\"16\" width=\"16\"> you just created."
                                            "<br>2. Select the IDs of the two laser beams you just created in the \"Target ID 1\" and \"Target ID 2\" properties. You can find out the ID of an object in the map by clicking on it and looking for \"ID\" in the property area."
                                            "<br><br><i>Note:</i> If you are saving your map file now, there will still be a validation error, but the warning should be gone."
                                            "<br><br><i>Hints:</i>"
                                            "<br>- You can see that two objects are connected if there is a dashed line between them."
                                            "<br>- The button \"Show/Hide Property Visualization\" (V) <img src=\"://rtb_resources/icons/action_visualhelper.png\" height=\"16\" width=\"16\"> in the toolbar enables these visual hints (lines, outlines, ...) for all objects in the map. If deactivated, only hints for selected objects are displayed.";
static const char* CHAPTER_PLAY = "<b>Play Map</b>"
                                    "<br><br>You can test your map directly from the editor."
                                    "<br><br><b>Test your map, step for step:</b>"
                                    "<br>1. Save <img src=\":images/24x24/document-save.png\" height=\"16\" width=\"16\"> your map file on your PC."
                                    "<br>2. Click the \"Play Map\" (F5) <img src=\"://rtb_resources/icons/action_playlevel.png\" height=\"16\" width=\"16\"> button."
                                    "<br>3. An error dialog will show. Close it with \"OK\". The next tutorial page will explain how to fix this error.";
static const char* CHAPTER_VALIDATOR = "<b>Validator & Map Testing</b>"
                                            "<br><br>If there is an error in the map, it can't be played. In the \"Validator\" area, you can see a list of all current errors or warnings in your map."
                                            "<br><br><b>Solve the error and test your map, step for step:</b>"
                                            "<br>1. Click on the validator entry \"Teleporters must have a Target.\". This means that your teleporter currently has no target location to teleport the ball to."
                                            "<br>2. In the properties of your teleporter, select the ID of your \"Target\" object in the \"Target ID\" property (which is marked red because of the error)."
                                            "<br>3. Save <img src=\":images/24x24/document-save.png\" height=\"16\" width=\"16\"> your map again. Notice that the validation error is gone."
                                            "<br>4. Again, click the \"Play Level\" (F5) <img src=\"://rtb_resources/icons/action_playlevel.png\" height=\"16\" width=\"16\"> button. The game will now start and load your map."
                                            "<br>5. To upload your map, roll into the finish hole and follow the steps that are displayed."
                                            "<br><br><i>Hints:</i>"
                                            "<br>- You can keep the game running in the background while creating your map. If you make changes to your map, save them, switch to the game and press F5 to reload the map."
                                            "<br>- The validator is always run when saving your map."
                                            "<br>- If your map has validation warnings, it can still be played. It is adviced though that you fix all warnings before uploading your map.";
static const char* CHAPTER_END = "<b>We're done!</b>"
                                    "<br><br>Congratulations, you have created your first map for Road to Ballhalla. Though this tutorial covered the major features of the editor, there's still lots of stuff to discover."
                                    "<br><br><b>Cheat Sheet:</b>"
                                    "<table cellspacing=\"5\">"
                                    "<tr><td>1, 2, 3</td><td>Jump to Layer</td></tr>"
                                    "<tr><td>TAB</td><td>Next Layer</td></tr>"
                                    "<tr><td>SHIFT + TAB</td><td>Previous Layer</td></tr>"
                                    "<tr><td>V</td><td>Show/Hide Property Visualization</td></tr>"
                                    "<tr><td>P</td><td>Open Map Properties</td></tr>"
                                    "<tr><td>F5</td><td>Play/Test Level</td></tr>"
                                    "<tr><td>A</td><td>Area Selection Tool</td></tr>"
                                    "<tr><td>ALT + 1-4</td><td>Set Interval Speed property for selected object(s)</td></tr>"
                                    "<tr><td>CTRL + 1-8</td><td>Set Interval Offset property for selected object(s)</td></tr>"
                                    "</table>";

RTBTutorial::RTBTutorial(RTBTutorialDock *tutorialDock, QWidget *parentWidget)
    : mParentWidget(parentWidget)
    , mTutorialDock(tutorialDock)
    , mIsRunning(false)
{
    connect(mTutorialDock, SIGNAL(nextButtonPressed()), this, SLOT(next()));
    connect(mTutorialDock, SIGNAL(prevButtonPressed()), this, SLOT(previous()));
    connect(mTutorialDock, SIGNAL(pauseButtonPressed(bool)), this, SLOT(pause(bool)));

    mCurrentChapter = Preferences::instance()->tutorialChapter();
    changeChapter();
}

RTBTutorial::~RTBTutorial()
{
    Preferences::instance()->setTutorialChapter(mCurrentChapter);
}

void RTBTutorial::retranslate()
{
    setText(mCurrentChapter);
}

void RTBTutorial::changeChapter()
{
    if(mIsRunning)
    {
        if(mCurrentChapter != Start && !mTutorialDock->isPrevButtonEnabled()){
            mTutorialDock->setPrevButtonEnabled(true);
        }
        else if(mCurrentChapter != End && !mTutorialDock->isNextButtonEnabled())
            mTutorialDock->setNextButtonEnabled(true);
    }

    if(mCurrentChapter == Start){
        mTutorialDock->setStartButtonText(RTBTutorialDock::Start);
    } else {
        mTutorialDock->setStartButtonText(RTBTutorialDock::Continue);
    }

    setText(mCurrentChapter);
    activateHighlight();
}

void RTBTutorial::activateHighlight()
{
    switch (mCurrentChapter) {
    case Start:
        mTutorialDock->setPrevButtonEnabled(false);
        emit highlightSection(None);
        break;
    case CreateMap:
        emit highlightSection(MainToolBar);
        break;
    case Map:
        emit highlightSection(MapView);
        break;
    case MapProperties:
        emit highlightSection(PropertiesDock);
        break;
    case Toolbar:
        emit highlightSection(ToolsToolBar);
        break;
    case Layer:
        emit highlightSection(LayerDock);
        break;
    case Objects:
        emit highlightSection(ToolsToolBar);
        break;
    case ObjectProperties:
        emit highlightSection(PropertiesDock);
        break;
    case Play:
        emit highlightSection(MainToolBar);
        break;
    case Validator:
        emit highlightSection(ValidatorDock);
        break;
    case End:
        mTutorialDock->setNextButtonEnabled(false);
        emit highlightSection(None);
        break;
    default:
        break;
    }
}

void RTBTutorial::next()
{
    if(mCurrentChapter == End)
        return;

    mCurrentChapter++;
    changeChapter();
}

void RTBTutorial::previous()
{
    if(mCurrentChapter == Start)
        return;

    mCurrentChapter--;
    changeChapter();
}

void RTBTutorial::pause(bool isRunning)
{
    if(!isRunning)
        emit highlightSection(None);
    else
        changeChapter();

    mIsRunning = isRunning;
}

QString RTBTutorial::tileImage(int id)
{
    QPixmap pixmap = RTBMapSettings::tileImageSource(id);
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap.save(&buffer, "PNG");

    return QLatin1String(byteArray.toBase64());
}

void RTBTutorial::setText(int id)
{
    switch (id) {
    case Start:
        mTutorialDock->setText(tr(CHAPTER_START));
        break;
    case CreateMap:
        mTutorialDock->setText(tr(CHAPTER_CREATE_MAP));
        break;
    case Map:
        mTutorialDock->setText(tr(CHAPTER_MAP).arg(tileImage(RTBMapObject::StartLocation), tileImage(RTBMapObject::FinishHole)));
        break;
    case MapProperties:
        mTutorialDock->setText(tr(CHAPTER_MAP_PROP));
        break;
    case Toolbar:
        mTutorialDock->setText(tr(CHAPTER_TOOLBAR).arg(tileImage(RTBMapSettings::Floor)));
        break;
    case Layer:
        mTutorialDock->setText(tr(CHAPTER_LAYER));
        break;
    case Objects:
        mTutorialDock->setText(tr(CHAPTER_OBJECTS).arg(tileImage(RTBMapObject::LaserBeamLeft)
                                                            , tileImage(RTBMapObject::Button)
                                                            , tileImage(RTBMapObject::Teleporter)
                                                            , tileImage(RTBMapObject::Target)));
        break;

    case ObjectProperties:
        mTutorialDock->setText(tr(CHAPTER_OBJECT_PROP).arg(tileImage(RTBMapObject::Button)));
        break;
    case Play:
        mTutorialDock->setText(tr(CHAPTER_PLAY));
        break;
    case Validator:
        mTutorialDock->setText(tr(CHAPTER_VALIDATOR));
        break;
    case End:
        mTutorialDock->setText(tr(CHAPTER_END));
        break;
    default:
        mTutorialDock->setText(tr(""));
        break;
    }
}

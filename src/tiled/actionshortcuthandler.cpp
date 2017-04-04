/* This class contains 3 Vector containers:
 * mActionList contains a list of QActions. mDockWidgetList contains a list of DockWidgets
 * mTooltipList constains a list of tooltips that are indexed to correspond with its Parent
 * QAction in mActionList. EX: mToolTipList[0] is the original tooltip for mActionList[0].
 *
 * This singleton class can be accessed from anywhere by including its header file and using
 * ActionShorcutHandler::getInstance() */

#include "actionshortcuthandler.h"

#include <QAction>
#include <QDockWidget>
#include <QToolBar>
#include <QMenu>
#include <QString>

ActionShortcutHandler& ActionShortcutHandler::getInstance() {
    static ActionShortcutHandler instance;
    return instance;
}
ActionShortcutHandler::ActionShortcutHandler() {}

/* Add all of our QActions to a vector */
void ActionShortcutHandler::addAction(QAction *_newAction)
{
    /* Do not add duplicate actions */
    if (!mActionList.contains(_newAction))
    {
        mActionList.append(_newAction);
        QString str = _newAction->toolTip();

        /* This takes off the existing shortcut text in tooltip if it is instantiated
         * before being added to this list, this class takes care of the rest  */
        if (str.contains(QLatin1String("(")) && str.contains(QLatin1String(")")))
            mTooltipList.append(str.left(str.indexOf(QLatin1String("(")) - 1));
        else
            mTooltipList.append(str);
    }
}

/* Add all our QActions to a vector, actions are added under these conditions:
 * 1: the QAction toolTip is not empty || 2: the QAction is not a seperator */
void ActionShortcutHandler::addActionList(const QVector<QAction*>& listToAdd)
{
    foreach(QAction* _action, listToAdd)
    {
        // Add each action to our ActionList only if it doesnt already exist
        if (!mActionList.contains(_action))
        {
            if (!_action->toolTip().isEmpty() && !_action->isSeparator())
            {
                mActionList.append(_action);
                QString str = _action->toolTip();
                /* This takes off the existing shortcut text in tooltip if it is instantiated
                 * before being added to this list, this class takes care of the rest  */
                if (str.contains(QLatin1String("(")) && str.contains(QLatin1String(")")))
                    mTooltipList.append(str.left(str.indexOf(QLatin1String("(")) - 1));
                else
                    mTooltipList.append(str);
            }
        }
    }
}

/* Add all our QWidgets to a vector */
void ActionShortcutHandler::addDockWidget(QDockWidget* _newWidget)
{
    if (!mWidgetList.contains(_newWidget))
        mWidgetList.append(_newWidget);
}

/* Add an array of QWidgets to our current List */
void ActionShortcutHandler::addDockWidgetList(const QVector<QDockWidget*>& listToAdd)
{
    foreach(QDockWidget *_widget, listToAdd)
    {
        // Add each widget to our action list only if it doesnt already exist
        if (!mWidgetList.contains(_widget))
            mWidgetList.append(_widget);
    }
}

/* Run through each DockWidget in our widget list, for each DockWidget we need to
 * grab its toolbar. Then from each toolbar we grab its set of actions. Finally,
 * we add oll of that toolbars actions to our actionList */
void ActionShortcutHandler::populateAListFromWList()
{
    foreach (QDockWidget* _widget, mWidgetList)
    {
        // Find all the childred of type QToolBar of this specific QDockWidget
        QList<QToolBar*> _toolBars = _widget->findChildren<QToolBar*>();

        // Add the actions of each toolBar in our newly create list of QToolbars
        foreach (QToolBar* _aToolBar, _toolBars) {
            addActionList(_aToolBar->actions().toVector());
        }
    }
}

/* Does the specified action already exist in our list? */
bool ActionShortcutHandler::actionFound(QAction *_action) {
    if (mActionList.contains(_action))
        return true;
    else
        return false;
}

/* Return the list of QActions contained in mActionList */
QVector<QAction*> ActionShortcutHandler::getActionList() {
    return mActionList;
}

/* This method updates the shortcut text for the actions in our list based on
 * their shortcuts, erases shortcut text if a shortcut no longer exists */
void ActionShortcutHandler::updateAllActionShortcutText()
{
    int i = 0;
    foreach(QAction *_mAction, mActionList)
    {
        i = mActionList.indexOf(_mAction);
        // If the QAction has a shortcut we add it onto the end of the tooltip
        if (!_mAction->shortcut().isEmpty())
            _mAction->setToolTip(mTooltipList[i] + QLatin1String (" (") + _mAction->shortcut().toString()+ QLatin1String(")"));
        else
            _mAction->setToolTip(mTooltipList[i]);
    }
}

/* Get the tooltip as set in our toolTip list, this is the QActions default
 * tooltip text without the shortcut added */
QString ActionShortcutHandler::getToolip(QAction *_mAction) {
    return mTooltipList[mActionList.indexOf(_mAction)];
}

/* Update only one specific QAction toolTip */
void ActionShortcutHandler::updateActionShortcutText(QAction* _mAction)
{
    int i = mActionList.indexOf(_mAction);
    if (!_mAction->shortcut().isEmpty())
        _mAction->setToolTip(mTooltipList[i] + QLatin1String (" (") + _mAction->shortcut().toString()+ QLatin1String(")"));
    else
        _mAction->setToolTip(mTooltipList[i]);
}

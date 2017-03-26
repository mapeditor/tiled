#include <QAction>
#include <QVector>
#include <QDockWidget>
#include <QToolBar>
#include <QKeySequence>


#ifndef ACTIONSHORTCUTDISPLAY_H
#define ACTIONSHORTCUTDISPLAY_H

class ActionShortcutHandler
{
    public:
        static ActionShortcutHandler& getInstance();

        void addAction(QAction *_newAction); // Add a QActions to our vector
        void addActionList(const QVector<QAction*>& listToAdd); // Add list of QActions to vector
        void updateAllActionShortcutText(); // Reset shortcut text for entire list
        void updateActionShortcutText(QAction *compareObj); // Update shortcut text for specific action
        QVector<QAction*> getActionList();
        void addDockWidgetList(const QVector<QDockWidget*>& listToAdd); // Add list of QDockWidgets
        void addDockWidget(QDockWidget* _newWidget); // Add single QDockWidget
        void populateAListFromWList(); // Populate Action list from Dock list
        bool actionFound(QAction *_action); // Check for a specific action in the action list
        QString getToolip(QAction *_mAction); // Return the original tool tip text of this action

        /* Do not want these methods for Singleton class, ensure only
           one instance ever exists */
        ActionShortcutHandler(ActionShortcutHandler const&) = delete;
        void operator=(ActionShortcutHandler const&) = delete;

    private:
        // This vector houses all of the created actions in our program
        QVector<QAction*> mActionList;
        QVector<QDockWidget*> mWidgetList;
        QVector<QString> mTooltipList;
        ActionShortcutHandler();
};

#endif // ACTIONSHORTCUTDISPLAY_H

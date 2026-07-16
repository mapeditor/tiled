/*
 * qmldock.cpp
 * Copyright 2026, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "qmldock.h"

#include "documentmanager.h"
#include "editor.h"
#include "logginginterface.h"
#include "mainwindow.h"
#include "scriptmanager.h"

#include <QCoreApplication>
#include <QDockWidget>
#include <QMainWindow>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWidget>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include <QQuickWindow>
#endif

namespace Tiled {

QmlDock::QmlDock(QObject *parent)
    : QObject(parent)
{
}

QmlDock::~QmlDock()
{
    delete mDockWidget;
}

void QmlDock::setTitle(const QString &title)
{
    if (mTitle == title)
        return;

    mTitle = title;

    if (mDockWidget)
        mDockWidget->setWindowTitle(title);

    emit titleChanged();
}

static QMainWindow *targetWindow(QmlDock::Window window)
{
    switch (window) {
    case QmlDock::MainWindow:
        return Tiled::MainWindow::maybeInstance();
    case QmlDock::MapEditor:
    case QmlDock::TilesetEditor: {
        const auto documentType = window == QmlDock::MapEditor ? Document::MapDocumentType
                                                               : Document::TilesetDocumentType;
        if (auto documentManager = DocumentManager::maybeInstance())
            if (auto editor = documentManager->editor(documentType))
                return qobject_cast<QMainWindow*>(editor->editorWidget());
        break;
    }
    }
    return nullptr;
}

void QmlDock::componentComplete()
{
    // Docks are not supported when running without GUI
    auto mainWindow = targetWindow(mWindow);
    if (!mainWindow)
        return;

    if (mName.isEmpty()) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Dock without name"));
        return;
    }

    if (!mContentItem) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Dock '%1' without content item").arg(mName));
        return;
    }

    // The object name identifies the dock when saving/restoring the window
    // layout, so duplicates would corrupt each other's placement.
    const QString objectName = QLatin1String("qml.") + mName;
    if (mainWindow->findChild<QDockWidget*>(objectName)) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Duplicate dock name: '%1'").arg(mName));
        return;
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
    // Before Qt 6.4, QQuickWidget only works with the OpenGL backend
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
#endif

    std::unique_ptr<QObject> content { mContentItem->create() };
    if (!content) {
        for (const QQmlError &error : mContentItem->errors())
            Tiled::ERROR(error.toString());
        return;
    }

    if (!qobject_cast<QQuickItem*>(content.get())) {
        Tiled::ERROR(QCoreApplication::translate("Script Errors", "Dock '%1' content is not an Item").arg(mName));
        return;
    }

    auto dock = new QDockWidget(mTitle.isEmpty() ? mName : mTitle, mainWindow);
    dock->setObjectName(objectName);

    auto quickWidget = new QQuickWidget(ScriptManager::instance().engine(), dock);
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    quickWidget->setContent(mContentItem->url(), mContentItem, content.release());

    dock->setWidget(quickWidget);
    mDockWidget = dock;

    // Apply any previously saved placement, relevant when the dock is
    // created after the main window layout was restored (which is the common
    // case, and always the case after a script engine reset). Restoring
    // before adding also avoids a crash with Qt 6.10.0 to 6.10.2 (see
    // QTBUG-143708).
    if (!mainWindow->restoreDockWidget(dock))
        mainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(mArea), dock);
}

} // namespace Tiled

#include "moc_qmldock.cpp"

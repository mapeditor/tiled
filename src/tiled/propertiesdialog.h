/*
 * propertiesdialog.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include "object.h"

#include <QDialog>
#include <QString>

class QUndoStack;

namespace Ui {
class PropertiesDialog;
}

namespace Tiled {

class Layer;

namespace Internal {

class PropertiesModel;
class MapDocument;

class PropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param kind       the kind of properties (Map, Layer, Object, etc.)
     * @param object     the object of which to edit the properties.
     * @param undoStack  the undo stack to push changes onto.
     */
    PropertiesDialog(const QString &kind,
                     Object *object,
                     QUndoStack *undoStack,
                     QWidget *parent = 0);

    /**
     * Destructor.
     */
    ~PropertiesDialog();

    /**
     * Applies the edited properties. It does this by constructing a
     * ChangeProperties command and adding it to the undo stack.
     */
    void accept();

    /**
     * Shows the appropriate properties dialog for the given layer.
     */
    static void showDialogFor(Layer *layer,
                              MapDocument *mapDocument,
                              QWidget *parent);

private slots:
    void deleteSelectedProperties();

private:
    Ui::PropertiesDialog *mUi;
    PropertiesModel *mModel;
    QUndoStack *mUndoStack;
    Object *mObject;
    QString mKind;
};

} // namespace Internal
} // namespace Tiled

#endif // PROPERTIESDIALOG_H

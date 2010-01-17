/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>
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
     * @param properties the properties edited by this dialog.
     * @param undoStack  the undo stack to push changes onto.
     */
    PropertiesDialog(const QString &kind,
                     QMap<QString, QString> *properties,
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
    QMap<QString, QString> *mProperties;
    QString mKind;
};

} // namespace Internal
} // namespace Tiled

#endif // PROPERTIESDIALOG_H

/*
 * preferencesdialog.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QDialog>
#include <QVector>
#include <QKeySequence>
#include <QMap>

class QModelIndex;

namespace Ui {
class PreferencesDialog;
}

namespace Tiled {
namespace Internal {

/**
 * The preferences dialog. Allows the user to configure some general behaviour
 * settings of Tiled and choose the language.
 */
class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void languageSelected(int index);

    void on_resetButton_clicked();
    void on_keySequenceEdit_editingFinished();
    void on_scWidget_itemSelectionChanged();

private:
    void fromPreferences();

    void retranslateUi();

    void styleComboChanged(int index);
    void addItem(QString name, QString key);

    void autoUpdateToggled(bool checked);
    void checkForUpdates();

    Ui::PreferencesDialog *mUi;
    QStringList mLanguages;

    // Keyboard Shortcut Handlers
    QVector<QKeySequence> originalKeySequences;
    QMap<QString ,QAction*> itemIndex; // Stores QAction pointers by keys of their text.
};


} // namespace Internal
} // namespace Tiled

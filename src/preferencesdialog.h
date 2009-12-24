/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
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

#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include "tmxmapwriter.h"

#include <QDialog>

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
    PreferencesDialog(QWidget *parent = 0);
    ~PreferencesDialog();

protected:
    void changeEvent(QEvent *e);

private slots:
    void languageSelected(int index);

private:
    void fromPreferences();
    void toPreferences();

    TmxMapWriter::LayerDataFormat layerDataFormat() const;

    Ui::PreferencesDialog *mUi;
    QStringList mLanguages;
};


} // namespace Internal
} // namespace Tiled

#endif // PREFERENCESDIALOG_H

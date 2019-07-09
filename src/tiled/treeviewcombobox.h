/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include <QComboBox>
#include <QTreeView>

namespace Tiled {

class TreeViewComboBoxView : public QTreeView
{
    Q_OBJECT

public:
    TreeViewComboBoxView(QWidget *parent = nullptr);
    void adjustWidth(int width);
};


class TreeViewComboBox : public QComboBox
{
    Q_OBJECT

public:
    TreeViewComboBox(QWidget *parent = nullptr);

    QModelIndex currentModelIndex() const;
    void setCurrentModelIndex(const QModelIndex &index);

    TreeViewComboBoxView *view() const;

protected:
    void wheelEvent(QWheelEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    bool eventFilter(QObject* object, QEvent* event) override;
    void showPopup() override;
    void hidePopup() override;

private:
    QModelIndex indexBelow(QModelIndex index) const;
    QModelIndex indexAbove(QModelIndex index) const;
    QModelIndex lastIndex(const QModelIndex &index) const;

    TreeViewComboBoxView *m_view;
    bool m_skipNextHide;
};


inline QModelIndex TreeViewComboBox::currentModelIndex() const
{
    return m_view->currentIndex();
}

} // namespace Tiled

/*
 * shortcutsettingspage.cpp
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "shortcutsettingspage.h"
#include "ui_shortcutsettingspage.h"

#include "actionmanager.h"
#include "preferences.h"
#include "utils.h"

#include <QAbstractListModel>
#include <QAction>
#include <QItemEditorFactory>
#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QToolButton>

#include <memory>

namespace Tiled {

/**
 * The ActionsModel makes the list of actions and their shortcuts available
 * to the view.
 */
class ActionsModel : public QAbstractListModel
{
public:
    explicit ActionsModel(QObject *parent = nullptr);

    void resetAllCustomShortcuts();

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
     QList<Id> mActions;
     bool mDirty = false;
};

ActionsModel::ActionsModel(QObject *parent)
    : QAbstractListModel(parent)
    , mActions(ActionManager::actions())
{
    connect(ActionManager::instance(), &ActionManager::actionAdded,
            this, [this] { mDirty = true; });
}

void ActionsModel::resetAllCustomShortcuts()
{
    ActionManager::instance()->resetAllCustomShortcuts();

    emit dataChanged(index(0, 0),
                     index(mActions.size() - 1, 2),
                     QVector<int> { Qt::DisplayRole, Qt::EditRole, Qt::FontRole });
}

int ActionsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mActions.size();
}

int ActionsModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 3;
}

static QString strippedText(QString s)
{
    s.remove(QLatin1String("..."));
    for (int i = 0; i < s.size(); ++i) {
        if (s.at(i) == QLatin1Char('&'))
            s.remove(i, 1);
    }
    return s.trimmed();
}

QVariant ActionsModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole: {
        const Id actionId = mActions.at(index.row());

        switch (index.column()) {
        case 0:
            return actionId.name();
        case 1:
            return strippedText(ActionManager::action(actionId)->text());
        case 2:
            return ActionManager::action(actionId)->shortcut();
        }

        break;
    }
    case Qt::FontRole: {
        const Id actionId = mActions.at(index.row());

        if (ActionManager::instance()->hasCustomShortcut(actionId)) {
            QFont font = QApplication::font();
            font.setBold(true);
            return font;
        }
        break;
    }
    }

    return QVariant();
}

bool ActionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.column() == 2 && role == Qt::EditRole) {
        const Id actionId = mActions.at(index.row());
        const auto action = ActionManager::findAction(actionId);
        const auto keySequence = value.value<QKeySequence>();

        if (action && action->shortcut() != keySequence) {
            ActionManager::instance()->setCustomShortcut(actionId, keySequence);
            emit dataChanged(this->index(index.row(), 0),
                             this->index(index.row(), 2),
                             QVector<int> { Qt::DisplayRole, Qt::EditRole, Qt::FontRole });
            return true;
        }
    }

    return false;
}

Qt::ItemFlags ActionsModel::flags(const QModelIndex &index) const
{
    auto f = QAbstractListModel::flags(index);

    if (index.column() == 2)
        f |= Qt::ItemIsEditable;

    return f;
}

QVariant ActionsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return tr("Action");
        case 1:
            return tr("Text");
        case 2:
            return tr("Shortcut");
        }
    }
    return QVariant();
}


/**
 * ShortcutEditor is mostly needed to add a "Clear" button to the
 * QKeySequenceEdit, which doesn't feature one as of Qt 5.13.
 */
class ShortcutEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY keySequenceChanged USER true)

public:
    ShortcutEditor(QWidget *parent = nullptr);

    QKeySequence keySequence() const;

public slots:
    void setKeySequence(QKeySequence keySequence);

signals:
    void editingFinished();
    void keySequenceChanged(QKeySequence keySequence);

private:
    QKeySequenceEdit *mKeySequenceEdit;
};

ShortcutEditor::ShortcutEditor(QWidget *parent)
    : QWidget(parent)
    , mKeySequenceEdit(new QKeySequenceEdit)
{
    auto clearButton = new QToolButton(this);
    clearButton->setToolTip(tr("Remove shortcut"));

    Utils::setThemeIcon(clearButton, "edit-clear");

    auto layout = new QHBoxLayout(this);
    layout->addWidget(mKeySequenceEdit);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(clearButton);

    setFocusProxy(mKeySequenceEdit);

    connect(clearButton, &QToolButton::clicked,
            mKeySequenceEdit, &QKeySequenceEdit::clear);

    connect(mKeySequenceEdit, &QKeySequenceEdit::editingFinished,
            this, &ShortcutEditor::editingFinished);
    connect(mKeySequenceEdit, &QKeySequenceEdit::keySequenceChanged,
            this, &ShortcutEditor::keySequenceChanged);
}

QKeySequence ShortcutEditor::keySequence() const
{
    return mKeySequenceEdit->keySequence();
}

void ShortcutEditor::setKeySequence(QKeySequence keySequence)
{
    mKeySequenceEdit->setKeySequence(keySequence);
}


/**
 * ShortcutDelegate subclass needed to make the editor close when it emits the
 * "editingFinished" signal.
 */
class ShortcutDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ShortcutDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

private:
    std::unique_ptr<QItemEditorFactory> mItemEditorFactory;

};

ShortcutDelegate::ShortcutDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , mItemEditorFactory(new QItemEditorFactory)
{
    mItemEditorFactory->registerEditor(QVariant::KeySequence,
                                       new QStandardItemEditorCreator<ShortcutEditor>);

    setItemEditorFactory(mItemEditorFactory.get());
}

QWidget *ShortcutDelegate::createEditor(QWidget *parent,
                                        const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    auto editor = QStyledItemDelegate::createEditor(parent, option, index);

    if (auto shortcutEditor = qobject_cast<ShortcutEditor*>(editor)) {
        connect(shortcutEditor, &ShortcutEditor::editingFinished, this, [=] {
            const_cast<ShortcutDelegate*>(this)->commitData(editor);
            const_cast<ShortcutDelegate*>(this)->closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
        });
    }

    return editor;
}


/**
 * The actual settings page for editing keyboard shortcuts.
 */
ShortcutSettingsPage::ShortcutSettingsPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ShortcutSettingsPage)
    , mProxyModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    auto actionsModel = new ActionsModel(this);

    mProxyModel->setSourceModel(actionsModel);
    mProxyModel->setSortLocaleAware(true);
    mProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
    mProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    ui->shortcutsView->setModel(mProxyModel);
    ui->shortcutsView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->shortcutsView->header()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->shortcutsView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->shortcutsView->sortByColumn(0, Qt::AscendingOrder);
    ui->shortcutsView->setItemDelegateForColumn(2, new ShortcutDelegate);

    connect(ui->filterEdit, &QLineEdit::textChanged,
            mProxyModel, &QSortFilterProxyModel::setFilterFixedString);

    connect(ui->resetButton, &QPushButton::clicked,
            actionsModel, &ActionsModel::resetAllCustomShortcuts);
}

ShortcutSettingsPage::~ShortcutSettingsPage()
{
    delete ui;
}

} // namespace Tiled

#include "shortcutsettingspage.moc"

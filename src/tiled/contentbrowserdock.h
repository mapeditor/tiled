#ifndef CONTENTBROWSERDOCK_H
#define CONTENTBROWSERDOCK_H

#include <QDockWidget>

class QFileSystemModel;
class QTreeView;
class QLineEdit;
class QSortFilterProxyModel;

namespace Tiled {

class ContentBrowserDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit ContentBrowserDock(QWidget *parent = nullptr);

private slots:
    void onFilterChanged(const QString &text);

private:
    QFileSystemModel       *mFileModel;
    QSortFilterProxyModel  *mProxyModel;
    QTreeView              *mTreeView;
    QLineEdit              *mFilterEdit;
};

}

#endif

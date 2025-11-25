#ifndef CONTENTBROWSERDOCK_H
#define CONTENTBROWSERDOCK_H

#include <QDockWidget>

class QFileSystemModel;
class QSortFilterProxyModel;
class QTreeView;
class QListView;
class QLineEdit;
class QSplitter;

namespace Tiled {

class ContentBrowserDock : public QDockWidget
{
    Q_OBJECT
public:
    explicit ContentBrowserDock(QWidget *parent = nullptr);
    ~ContentBrowserDock() override = default;
private slots:
    void navigateToFolder(const QString &path);

private slots:
    void onTreeClicked(const QModelIndex &proxyIndex);
    void onDetailsDoubleClicked(const QModelIndex &proxyIndex);
    void onFilterChanged(const QString &text);

private:
    QFileSystemModel       *mFileModel;
    QSortFilterProxyModel  *mTreeProxy;
    QSortFilterProxyModel   *mDetailsProxy;

    QTreeView              *mTreeView;
    QListView              *mDetailsView;
    QLineEdit              *mFilterEdit;
    QSplitter              *mSplitter;

    QString                mCurrentFolder;
};

}

#endif

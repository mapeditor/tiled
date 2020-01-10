#pragma once

#include <reversingproxymodel.h>

#include <QObject>

namespace Tiled {

class ObjectsFilterModel : public ReversingProxyModel
{
    Q_OBJECT

public:
    ObjectsFilterModel(QObject *parent = nullptr)
        : ReversingProxyModel(parent)
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
        setRecursiveFilteringEnabled(true);
#endif
    }

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        return filterRecursiveAcceptsRow(sourceRow, sourceParent);
    }

private:
    bool filterRecursiveAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (ReversingProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return true;

        const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
        const int count = sourceModel()->rowCount(index);

        for (int i = 0; i < count; ++i)
            if (filterRecursiveAcceptsRow(i, index))
                return true;

        return false;
    }
#endif
};

}

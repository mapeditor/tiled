#ifndef TILEDQUICKPLUGIN_H
#define TILEDQUICKPLUGIN_H

#include <QQuickItem>

class TiledQuickPlugin : public QQuickItem
{
    Q_OBJECT
    Q_DISABLE_COPY(TiledQuickPlugin)

public:
    TiledQuickPlugin(QQuickItem *parent = 0);
    ~TiledQuickPlugin();
};

#endif // TILEDQUICKPLUGIN_H


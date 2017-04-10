#include <algorithm>
#include <string>
#include <iostream>

// Qt
#include <QString>
#include <QObject>
#include <QVariant>
#include <QEvent>
#include <QVector>
#include <QMap>
#include <QtPlugin>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include <QImage>
#include <QPixmap>
#include <QPalette>
#include <QTextDocument>

#include <QWidget>
#include <QApplication>
#include <QUndoCommand>

#include <QBoxLayout>
#include <QFrame>

#include <QMenu>
#include <QPinchGesture>
#include <QTreeView>
#include <QPushButton>
#include <QCheckBox>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>

#include <QPlainTextEdit>
#include <QFileDialog>

#include <qtvariantproperty.h>

// libtiled
#include <layer.h>
#include <mapdocument.h>
#include <mapobject.h>
#include <objectgroup.h>
#include <terrain.h>
#include <tile.h>
#include <tileset.h>
#include <tilesetmanager.h>

// tiled
#include "abstractobjecttool.h"
#include "preferences.h"

#include "documentmanager.h"
#include "mapscene.h"
#include "tmxmapformat.h"

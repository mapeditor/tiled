#include <algorithm>
#include <string>
#include <iostream>

// Qt
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QRegularExpression>
#include <QtCore/QEvent>
#include <QtCore/QVector>
#include <QtCore/QMap>
#include <QtCore/QtPlugin>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtGui/QPalette>
#include <QtGui/QTextDocument>

#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>
#include <QtWidgets/QUndoCommand>

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QFrame>

#include <QtWidgets/QMenu>
#include <QtWidgets/QPinchGesture>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>

#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>

#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QFileDialog>

#include <qtvariantproperty.h>

// libtiled
#include <tile.h>
#include <tileset.h>
#include <layer.h>
#include <terrain.h>
#include <mapdocument.h>
#include <mapobject.h>
#include <objectgroup.h>

// tiled
#include "abstractobjecttool.h"
#include "preferences.h"

#include "documentmanager.h"
#include "mapscene.h"
#include "tilesetmanager.h"
#include "tmxmapwriter.h"

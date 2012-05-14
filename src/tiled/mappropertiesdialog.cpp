#include "mappropertiesdialog.h"

#include "changemapproperties.h"
#include "colorbutton.h"
#include "map.h"
#include "mapdocument.h"

#include <QLabel>
#include <QLineEdit>
#include <QUndoStack>
#include <QGridLayout>
#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

MapPropertiesDialog::MapPropertiesDialog(MapDocument *mapDocument, QWidget *parent)

    : PropertiesDialog(tr("Map"), mapDocument->map(), mapDocument->undoStack(), parent)
    , mMapDocument(mapDocument)
    , mColorButton(new ColorButton)
{
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Background Color:")), 0, 0);
    grid->addWidget(mColorButton, 0, 1);

    QColor bgColor = mapDocument->map()->backgroundColor();
    mColorButton->setColor(bgColor.isValid() ? bgColor : Qt::gray);

    qobject_cast<QBoxLayout*>(layout())->insertLayout(0, grid);
}

void MapPropertiesDialog::accept()
{
    QUndoStack *undoStack = mMapDocument->undoStack();

    const QColor newColor = mColorButton->color() != Qt::gray ? mColorButton->color() : QColor();

    const bool localChanges = newColor != mMapDocument->map()->backgroundColor();

    if (localChanges) {
        undoStack->beginMacro(QCoreApplication::translate(
            "Undo Commands",
            "Change Map Properties"));

        undoStack->push(new ChangeMapProperties(mMapDocument, mColorButton->color()));
    }

    PropertiesDialog::accept();

    if (localChanges)
        undoStack->endMacro();
}

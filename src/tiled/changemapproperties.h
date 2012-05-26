#ifndef CHANGEMAPPROPERTIES_H
#define CHANGEMAPPROPERTIES_H

#include <QColor>
#include <QUndoCommand>

namespace Tiled {

namespace Internal {

class MapDocument;

class ChangeMapProperties : public QUndoCommand
{
public:
    /**
     * Constructs a new 'Change Map Properties' command.
     *
     * @param mapDocument       the map document of the map
     * @param bgColor           the new color to apply for the background
     */
    ChangeMapProperties(MapDocument *mapDocument, const QColor &bgColor);

    void undo();
    void redo();

private:

    MapDocument *mMapDocument;
    const QColor mUndoColor;
    const QColor mRedoColor;
};

} // namespace Internal
} // namespace Tiled

#endif // CHANGEMAPPROPERTIES_H

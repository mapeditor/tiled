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

//#include <QColor>
//#include <QString>
//#include <QUndoCommand>

//namespace Tiled {

//class ObjectGroup;

//namespace Internal {

//class MapDocument;

//class ChangeObjectGroupProperties : public QUndoCommand
//{
//public:
//    /**
//     * Constructs a new 'Change Object Layer Properties' command.
//     *
//     * @param mapDocument     the map document of the object group's map
//     * @param objectGroup     the object group in to modify
//     * @param newColor        the new color to apply
//     */
//    ChangeObjectGroupProperties(MapDocument *mapDocument,
//                                ObjectGroup *objectGroup,
//                                const QColor &newColor);

//    void undo();
//    void redo();

//private:

//    MapDocument *mMapDocument;
//    ObjectGroup *mObjectGroup;
//    const QColor mUndoColor;
//    const QColor mRedoColor;
//};

//} // namespace Internal
//} // namespace Tiled

//#endif // CHANGEOBJECTGROUPPROPERTIES_H

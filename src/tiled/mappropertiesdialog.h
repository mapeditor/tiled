#ifndef MAPPROPERTIESDIALOG_H
#define MAPPROPERTIESDIALOG_H

#include "propertiesdialog.h"

namespace Tiled {

namespace Internal {

class ColorButton;

class MapPropertiesDialog : public PropertiesDialog
{
    Q_OBJECT

public:
    MapPropertiesDialog(MapDocument *mapDocument, QWidget *parent = 0);

    void accept();

private:
    MapDocument *mMapDocument;
    ColorButton *mColorButton;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPPROPERTIESDIALOG_H

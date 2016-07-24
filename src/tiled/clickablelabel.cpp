#include "clickablelabel.h"

using namespace Tiled;
using namespace Tiled::Internal;

ClickableLabel::ClickableLabel(QWidget *parent) :
QLabel(parent)
{

}

ClickableLabel::~ClickableLabel()
{

}

void ClickableLabel::mouseMoveEvent(QMouseEvent *event)
{
    emit mouseMoved(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    emit mousePressed(event);
}

void ClickableLabel::mouseReleaseEvent(QMouseEvent *event)
{
    emit mouseReleased(event);
}

#include "scriptdialogwidget.h"
namespace Tiled {
ScriptDialogWidget::ScriptDialogWidget(QLabel *label, QWidget *mainWidget)
{
    this->label = label;
    this->mainWidget = mainWidget;
}
QString ScriptDialogWidget::getToolTip()
{
    return mainWidget->toolTip();
}
void ScriptDialogWidget::setToolTip(const QString &labelText)
{
    mainWidget->setToolTip(labelText);
    label->setToolTip(labelText);
}
void ScriptDialogWidget::setLabelText(const QString &labelText)
{
    label->setText(labelText);
}
} //namespace Tiled
#include "moc_scriptdialogwidget.cpp"

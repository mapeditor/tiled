#include "scriptdialogwidget.h"
namespace Tiled {
ScriptDialogWidget::ScriptDialogWidget(QLabel *label, QWidget *mainWidget)
{
    this->label = label;
    this->m_mainWidget = mainWidget;
}
QWidget *ScriptDialogWidget::mainWidget() const
{
    return m_mainWidget;
}
QString ScriptDialogWidget::getToolTip()
{
    return mainWidget()->toolTip();
}
void ScriptDialogWidget::setToolTip(const QString &labelText)
{
    mainWidget()->setToolTip(labelText);
    label->setToolTip(labelText);
}
void ScriptDialogWidget::setLabelText(const QString &labelText)
{
    label->setText(labelText);
}
} //namespace Tiled
#include "moc_scriptdialogwidget.cpp"

#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "map.h"

OptionsDialog::OptionsDialog(const Tiled::Map * map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    m_ImagesFolder = "";
    m_Optimize = true;
    m_OptimizeHV = true;

    ui->txtImageName->setText(m_ImagesFolder);
    ui->ckEnableCellOptimization->setChecked(m_Optimize);
    ui->rdCellOptHV->setChecked(m_OptimizeHV);
    ui->rdCellOptVH->setChecked(!m_OptimizeHV);

    // Set single selection mode
    ui->lstLayers->setSelectionMode(QAbstractItemView::SingleSelection);

    // Create list view item model
    m_Model = new QStandardItemModel(ui->lstLayers);

    int cnt = 0;
    for (Tiled::Layer * layer : map->layers())
    {
        QStandardItem * poListItem = new QStandardItem;
        poListItem->setCheckable( true );
        poListItem->setData(Qt::Checked, Qt::CheckStateRole);
        poListItem->setData(QVariant::fromValue((void *)layer), Qt::UserRole + 10);
        poListItem->setText(layer->name());
        m_Model->setItem(cnt++, poListItem);
    }

    ui->lstLayers->setModel(m_Model);

/*    // Register model item changed signal
    connect(m_Model, SIGNAL(itemChanged(QStandardItem*)), this,  SLOT(SlotItemChanged(QStandardItem*)));

    // Resister view item acticated
    connect( m_poListView , SIGNAL(activated(const QModelIndex & )), this, SLOT(SlotListItemActivated(const QModelIndex & )))
*/
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_btSelectAllLayers_clicked()
{
    for (int a = 0; a < m_Model->rowCount(); a++)
    {
        QStandardItem * poListItem = m_Model->item(a);
        poListItem->setCheckState(Qt::CheckState::Checked);
    }
}

void OptionsDialog::on_btUnselectAllLayers_clicked()
{
    for (int a = 0; a < m_Model->rowCount(); a++)
    {
        QStandardItem * poListItem = m_Model->item(a);
        poListItem->setCheckState(Qt::CheckState::Unchecked);
    }
}

void OptionsDialog::on_ckEnableCellOptimization_toggled(bool checked)
{
    m_Optimize = checked;
    ui->rdCellOptHV->setChecked(m_OptimizeHV);
    ui->rdCellOptHV->setEnabled(checked);

    ui->rdCellOptVH->setChecked(!m_OptimizeHV);
    ui->rdCellOptVH->setEnabled(checked);
}

void OptionsDialog::on_buttonBox_accepted()
{
    for (int a = 0; a < m_Model->rowCount(); a++)
    {
        QStandardItem * poListItem = m_Model->item(a);
        if (poListItem->checkState() == Qt::CheckState::Checked)
        {
            QVariant vv = poListItem->data(Qt::UserRole + 10);
            Tiled::Layer * layer = (Tiled::Layer *)vv.value<void *>();
            m_SelectedLayers.push_back(layer);
        }
    }

    m_ImagesFolder = ui->txtImageName->text();
    m_OptimizeHV = ui->rdCellOptHV->isChecked();
}

void OptionsDialog::on_buttonBox_rejected()
{

}

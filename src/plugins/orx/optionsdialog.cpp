#include "optionsdialog.h"
#include <QMessageBox>

#include "map.h"

OptionsDialog::OptionsDialog(const Tiled::Map * map, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    m_ChangingCellState(false)
{
    ui->setupUi(this);

    m_ImagesFolder = "";
    m_Optimize = true;
    m_OptimizeHV = true;

    ui->txtImageFolderName->setText(m_ImagesFolder);
    ui->ckEnableCellOptimization->setChecked(m_Optimize);
    ui->rdCellOptHV->setChecked(m_OptimizeHV);
    ui->rdCellOptVH->setChecked(!m_OptimizeHV);

    fill_layers(map);

    QString tooltip;
    tooltip += "Accepted values:\n";
    tooltip += " %c = tile column value\n";
    tooltip += " %i = tile index value\n";
    tooltip += " %n = base name of the tileset\n";
    tooltip += " %r = tile row value\n";
    tooltip += "\n";
    tooltip += "Example:\n";
    tooltip += " %n_%i = tileset1_537\n";

    ui->txtNameExpression->setToolTip(tooltip);
    ui->txtNameExpression->setText("%n%i");
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::fill_layers(const Tiled::Map * map)
{
    int cnt = 0;
    ui->lstLayers->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    for (Tiled::Layer * layer : map->layers())
    {
        int row = ui->lstLayers->rowCount();
        ui->lstLayers->insertRow(row);

        m_ChangingCellState = true;

        QTableWidgetItem * expItem = new QTableWidgetItem;
        expItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        expItem->setCheckState(Qt::Checked);
        ui->lstLayers->setItem(row, 0, expItem);

        QTableWidgetItem * normalItem = new QTableWidgetItem;
        normalItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        normalItem->setCheckState(Qt::Checked);
        ui->lstLayers->setItem(row, 1, normalItem);

        QTableWidgetItem * shaderItem = new QTableWidgetItem;
        shaderItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        shaderItem->setCheckState(Qt::Unchecked);
        ui->lstLayers->setItem(row, 2, shaderItem);

        QTableWidgetItem * nameItem = new QTableWidgetItem;
        nameItem->setText(layer->name());
        nameItem->setData(Qt::UserRole + 10, QVariant::fromValue((void *)layer));
        ui->lstLayers->setItem(row, 3, nameItem);

        m_ChangingCellState = false;
    }

}

void OptionsDialog::on_lstLayers_itemChanged(QTableWidgetItem *item)
{
    if (m_ChangingCellState)
        return;

    m_ChangingCellState = true;

    QTableWidgetItem * other_item = nullptr;
    int col = item->column();

    if (col = 1)
        other_item = ui->lstLayers->item(item->row(), 2);
    else if (col == 2)
        other_item = ui->lstLayers->item(item->row(), 1);

    if (other_item != nullptr)
    {
        if ((item->checkState() == Qt::CheckState::Checked) && (other_item->checkState() == Qt::CheckState::Checked))
            other_item->setCheckState(Qt::CheckState::Unchecked);

        if ((item->checkState() == Qt::CheckState::Unchecked) && (other_item->checkState() == Qt::CheckState::Unchecked))
            other_item->setCheckState(Qt::CheckState::Checked);
    }
    m_ChangingCellState = false;
}


void OptionsDialog::on_btSelectAllLayers_clicked()
{
    for (int a = 0; a < ui->lstLayers->rowCount(); a++)
    {
        QTableWidgetItem * expItem = ui->lstLayers->item(a, 0);
        expItem->setCheckState(Qt::CheckState::Checked);
    }
}

void OptionsDialog::on_btUnselectAllLayers_clicked()
{
    for (int a = 0; a < ui->lstLayers->rowCount(); a++)
    {
        QTableWidgetItem * expItem = ui->lstLayers->item(a, 0);
        expItem->setCheckState(Qt::CheckState::Unchecked);
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
    for (int a = 0; a < ui->lstLayers->rowCount(); a++)
    {
        QTableWidgetItem * exportItem = ui->lstLayers->item(a, 0);
        QTableWidgetItem * normalItem = ui->lstLayers->item(a, 1);
//        QTableWidgetItem * shaderItem = ui->lstLayers->item(a, 2);
        QTableWidgetItem * layerItem = ui->lstLayers->item(a, 3);
        if (exportItem->checkState() == Qt::CheckState::Checked)
        {
            QVariant vv = layerItem->data(Qt::UserRole + 10);
            SelectedLayer ss;
            ss.m_layer = (Tiled::Layer *)vv.value<void *>();
            ss.m_normalExport = (normalItem->checkState() == Qt::CheckState::Checked);
            m_SelectedLayers.push_back(ss);
        }
    }

    m_ImagesFolder = ui->txtImageFolderName->text();
    m_OptimizeHV = ui->rdCellOptHV->isChecked();
    m_SectionNameExp = ui->txtNameExpression->text();
}

void OptionsDialog::on_buttonBox_rejected()
{

}

void OptionsDialog::on_rdUseWnidowsSeparator_clicked()
{
    ui->rdUseUnixSeparator->setChecked(!ui->rdUseWnidowsSeparator->isChecked());
}

void OptionsDialog::on_rdUseUnixSeparator_clicked()
{
    ui->rdUseWnidowsSeparator->setChecked(!ui->rdUseUnixSeparator->isChecked());
}

void OptionsDialog::on_txtNameExpression_textChanged(const QString &arg1)
{
    ui->lblNamePreview->setText(Orx::NameGenerator::Generate(arg1, "Preview", 43, 16, 8));
}



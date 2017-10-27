#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QStandardItemModel>
#include <QDialog>
#include "ui_optionsdialog.h"


#include "tilelayer.h"
#include "name_generator.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(const Tiled::Map * map, QWidget *parent = 0);
    ~OptionsDialog();

public:
    struct SelectedLayer
    {
        Tiled::Layer *  m_layer;
        bool            m_normalExport;
    };

    QString                 m_ImagesFolder;
    QString                 m_SectionNameExp;
    QVector<SelectedLayer>  m_SelectedLayers;
    bool                    m_Optimize;
    bool                    m_OptimizeHV;
    bool                    m_WindowsFolderSeparator;
    bool                    m_ChangingCellState;


private slots:
    void on_btSelectAllLayers_clicked();

    void on_btUnselectAllLayers_clicked();

    void on_ckEnableCellOptimization_toggled(bool checked);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_rdUseWnidowsSeparator_clicked();

    void on_rdUseUnixSeparator_clicked();

    void on_txtNameExpression_textChanged(const QString &arg1);

    void on_lstLayers_itemChanged(QTableWidgetItem *item);

private:
    void fill_layers(const Tiled::Map * map);

private:
    Ui::OptionsDialog *ui;

};

#endif // OPTIONSDIALOG_H

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QStandardItemModel>
#include <QDialog>

#include "tilelayer.h"

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
    QString m_ImagesFolder;
    QVector<Tiled::Layer *> m_SelectedLayers;
    bool    m_Optimize;
    bool    m_OptimizeHV;
    QStandardItemModel * m_Model;

private slots:
    void on_btSelectAllLayers_clicked();

    void on_btUnselectAllLayers_clicked();

    void on_ckEnableCellOptimization_toggled(bool checked);

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::OptionsDialog *ui;

};

#endif // OPTIONSDIALOG_H

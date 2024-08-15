#pragma once

#include <QCoreApplication>
#include <QScrollArea>
#include <QString>
#include <QWidget>

#include <memory>
#include <unordered_map>

class QGridLayout;

namespace Tiled {

class EditorFactory
{
    Q_DECLARE_TR_FUNCTIONS(EditorFactory)

public:
    virtual QWidget *createEditor(const QVariant &value,
                                  QWidget *parent) = 0;
};

class VariantEditor : public QScrollArea
{
    Q_OBJECT

public:
    VariantEditor(QWidget *parent = nullptr);

    void registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory);

    void setValue(const QVariant &value);

private:
    QWidget *m_widget;
    QGridLayout *m_gridLayout;
    int m_rowIndex = 0;
    std::unordered_map<int, std::unique_ptr<EditorFactory>> m_factories;

    // QAbstractScrollArea interface
protected:
    QSize viewportSizeHint() const override;
};

} // namespace Tiled

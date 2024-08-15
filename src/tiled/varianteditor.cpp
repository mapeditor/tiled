#include "varianteditor.h"
#include "tiled.h"
#include "utils.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QSpinBox>

namespace Tiled {

// These subclasses don't adjust their horizontal size hint based on the
// minimum or maximum value.
struct MinimumSpinBox : QSpinBox
{
    using QSpinBox::QSpinBox;

    QSize minimumSizeHint() const override
    {
        auto hint = QSpinBox::minimumSizeHint();
        hint.setWidth(Utils::dpiScaled(50));
        return hint;
    }
};
struct MinimumDoubleSpinBox : QDoubleSpinBox
{
    using QDoubleSpinBox::QDoubleSpinBox;

    QSize minimumSizeHint() const override
    {
        auto hint = QDoubleSpinBox::minimumSizeHint();
        hint.setWidth(Utils::dpiScaled(50));
        return hint;
    }
};

// A label that matches its preferred height with that of a line edit
struct LineEditLabel : QLabel
{
    using QLabel::QLabel;

    QSize sizeHint() const override
    {
        const auto lineEditHint = lineEdit.sizeHint();
        auto hint = QLabel::sizeHint();
        hint.setHeight(lineEditHint.height());
        return hint;
    }

private:
    QLineEdit lineEdit;
};

constexpr QSizePolicy preferredWidthIgnoredHeight(QSizePolicy::Preferred, QSizePolicy::Fixed);

class StringEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QLineEdit(parent);
        editor->setSizePolicy(preferredWidthIgnoredHeight);
        editor->setText(value.toString());
        return editor;
    }
};

class IntEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new MinimumSpinBox(parent);
        editor->setSizePolicy(preferredWidthIgnoredHeight);
        editor->setRange(-std::numeric_limits<int>::max(),
                         std::numeric_limits<int>::max());
        editor->setValue(value.toInt());
        return editor;
    }
};

class FloatEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new MinimumDoubleSpinBox(parent);
        editor->setSizePolicy(preferredWidthIgnoredHeight);
        editor->setRange(-std::numeric_limits<double>::max(),
                         std::numeric_limits<double>::max());
        editor->setValue(value.toDouble());
        return editor;
    }
};

class BoolEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QCheckBox(parent);
        bool checked = value.toBool();
        editor->setChecked(checked);
        editor->setText(checked ? tr("True") : tr("False"));

        QObject::connect(editor, &QCheckBox::toggled, [editor](bool checked) {
            editor->setText(checked ? QObject::tr("True") : QObject::tr("False"));
        });

        return editor;
    }
};

class PointEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QWidget(parent);
        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());

        auto xLabel = new QLabel(QStringLiteral("X"), editor);
        horizontalLayout->addWidget(xLabel);

        auto xSpinBox = new MinimumSpinBox(editor);
        xSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        xLabel->setBuddy(xSpinBox);
        xSpinBox->setRange(-std::numeric_limits<int>::max(),
                           std::numeric_limits<int>::max());
        xSpinBox->setValue(value.toPoint().x());
        horizontalLayout->addWidget(xSpinBox, 0, Qt::AlignRight);

        horizontalLayout->addStretch();

        auto yLabel = new QLabel(QStringLiteral("Y"), editor);
        horizontalLayout->addWidget(yLabel);

        auto ySpinBox = new MinimumSpinBox(editor);
        ySpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        yLabel->setBuddy(ySpinBox);
        ySpinBox->setRange(-std::numeric_limits<int>::max(),
                           std::numeric_limits<int>::max());
        ySpinBox->setValue(value.toPoint().y());
        horizontalLayout->addWidget(ySpinBox, 0, Qt::AlignRight);

        // Avoid vertical stretching
        auto sizePolicy = editor->sizePolicy();
        sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        editor->setSizePolicy(sizePolicy);

        return editor;
    }
};

class SizeEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QWidget(parent);
        auto horizontalLayout = new QHBoxLayout(editor);
        horizontalLayout->setContentsMargins(QMargins());

        auto widthLabel = new QLabel(QStringLiteral("W"), editor);
        horizontalLayout->addWidget(widthLabel);

        auto widthSpinBox = new MinimumSpinBox(editor);
         widthSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        widthLabel->setBuddy(widthSpinBox);
        widthSpinBox->setRange(-std::numeric_limits<int>::max(),
                           std::numeric_limits<int>::max());
        widthSpinBox->setValue(value.toSize().width());
        horizontalLayout->addWidget(widthSpinBox, 0, Qt::AlignRight);

        horizontalLayout->addStretch();

        auto heightLabel = new QLabel(QStringLiteral("H"), editor);
        horizontalLayout->addWidget(heightLabel);

        auto heightSpinBox = new MinimumSpinBox(editor);
         heightSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        heightLabel->setBuddy(heightSpinBox);
        heightSpinBox->setRange(-std::numeric_limits<int>::max(),
                           std::numeric_limits<int>::max());
        heightSpinBox->setValue(value.toSize().height());
        horizontalLayout->addWidget(heightSpinBox, 0, Qt::AlignRight);

        // Avoid vertical stretching
        auto sizePolicy = editor->sizePolicy();
        sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        editor->setSizePolicy(sizePolicy);

        return editor;
    }
};

class RectFEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QWidget(parent);
        auto gridLayout = new QGridLayout(editor);
        gridLayout->setContentsMargins(QMargins());

        auto xLabel = new QLabel(QStringLiteral("X"), editor);
        gridLayout->addWidget(xLabel, 0, 0, Qt::AlignRight);

        auto xSpinBox = new MinimumDoubleSpinBox(editor);
        xSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        xLabel->setBuddy(xSpinBox);
        xSpinBox->setRange(-std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max());
        xSpinBox->setValue(value.toRectF().x());
        gridLayout->addWidget(xSpinBox, 0, 1, Qt::AlignRight);

        // gridLayout->addStretch();

        auto yLabel = new QLabel(QStringLiteral("Y"), editor);
        gridLayout->addWidget(yLabel, 0, 2, Qt::AlignRight);

        auto ySpinBox = new MinimumDoubleSpinBox(editor);
        ySpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        yLabel->setBuddy(ySpinBox);
        ySpinBox->setRange(-std::numeric_limits<double>::max(),
                           std::numeric_limits<double>::max());
        ySpinBox->setValue(value.toRectF().y());
        gridLayout->addWidget(ySpinBox, 0, 3, Qt::AlignRight);

        auto widthLabel = new QLabel(QStringLiteral("W"), editor);
        gridLayout->addWidget(widthLabel, 1, 0, Qt::AlignRight);

        auto widthSpinBox = new MinimumDoubleSpinBox(editor);
        widthSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        widthLabel->setBuddy(widthSpinBox);
        widthSpinBox->setRange(-std::numeric_limits<double>::max(),
                               std::numeric_limits<double>::max());
        widthSpinBox->setValue(value.toRectF().width());
        gridLayout->addWidget(widthSpinBox, 1, 1, Qt::AlignRight);

        // horizontalLayout->addStretch();

        auto heightLabel = new QLabel(QStringLiteral("H"), editor);
        gridLayout->addWidget(heightLabel, 1, 2, Qt::AlignRight);

        auto heightSpinBox = new MinimumDoubleSpinBox(editor);
        heightSpinBox->setSizePolicy(preferredWidthIgnoredHeight);
        heightLabel->setBuddy(heightSpinBox);
        heightSpinBox->setRange(-std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max());
        heightSpinBox->setValue(value.toRectF().height());
        gridLayout->addWidget(heightSpinBox, 1, 3, Qt::AlignRight);

        // Avoid vertical stretching
        auto sizePolicy = editor->sizePolicy();
        sizePolicy.setHorizontalPolicy(QSizePolicy::Preferred);
        sizePolicy.setVerticalPolicy(QSizePolicy::Fixed);
        editor->setSizePolicy(sizePolicy);

        return editor;
    }
};

class EnumEditorFactory : public EditorFactory
{
public:
    QWidget *createEditor(const QVariant &value, QWidget *parent) override
    {
        auto editor = new QComboBox(parent);
        editor->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        editor->addItems(m_enumNames);
        editor->setCurrentIndex(value.toInt());
        return editor;
    }

    void setEnumNames(const QStringList &enumNames)
    {
        m_enumNames = enumNames;
    }

private:
    QStringList m_enumNames;
};


VariantEditor::VariantEditor(QWidget *parent)
    : QScrollArea(parent)
{
    m_widget = new QWidget;
    auto verticalLayout = new QVBoxLayout(m_widget);
    m_gridLayout = new QGridLayout;
    verticalLayout->addLayout(m_gridLayout);
    verticalLayout->addStretch();

    setWidget(m_widget);
    setWidgetResizable(true);
    // setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // m_widget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored));

    m_gridLayout->setSpacing(Utils::dpiScaled(2));

    registerEditorFactory(QMetaType::QString, std::make_unique<StringEditorFactory>());
    registerEditorFactory(QMetaType::Int, std::make_unique<IntEditorFactory>());
    registerEditorFactory(QMetaType::Double, std::make_unique<FloatEditorFactory>());
    registerEditorFactory(QMetaType::Bool, std::make_unique<BoolEditorFactory>());
    registerEditorFactory(QMetaType::QPoint, std::make_unique<PointEditorFactory>());
    registerEditorFactory(QMetaType::QSize, std::make_unique<SizeEditorFactory>());
    registerEditorFactory(QMetaType::QRectF, std::make_unique<RectFEditorFactory>());

    auto alignmentEditorFactory = std::make_unique<EnumEditorFactory>();
    alignmentEditorFactory->setEnumNames({
                                             tr("Unspecified"),
                                             tr("Top Left"),
                                             tr("Top"),
                                             tr("Top Right"),
                                             tr("Left"),
                                             tr("Center"),
                                             tr("Right"),
                                             tr("Bottom Left"),
                                             tr("Bottom"),
                                             tr("Bottom Right"),
                                         });
    registerEditorFactory(qMetaTypeId<Alignment>(), std::move(alignmentEditorFactory));

    setValue(QVariantMap {
                 { QStringLiteral("Name"), QVariant(QLatin1String("Hello")) },
                 { QStringLiteral("Rectangle"), QVariant(QRectF(15, 50, 35, 400)) },
                 { QStringLiteral("Margin"), QVariant(10) },
                 { QStringLiteral("Opacity"), QVariant(0.5) },
                 { QStringLiteral("Visible"), true },
                 { QStringLiteral("Object Alignment"), QVariant::fromValue(TopLeft) },
             });

    m_gridLayout->setColumnStretch(0, 0);
    m_gridLayout->setColumnStretch(1, 0);
    m_gridLayout->setColumnStretch(2, 1);
    m_gridLayout->setColumnMinimumWidth(1, Utils::dpiScaled(10));

    // setValue(QVariantList {
    //              QVariant(QLatin1String("Hello")),
    //              QVariant(10),
    //              QVariant(3.14)
    //          });
}

void VariantEditor::registerEditorFactory(int type, std::unique_ptr<EditorFactory> factory)
{
    m_factories[type] = std::move(factory);
}

void VariantEditor::setValue(const QVariant &value)
{
    const int type = value.userType();
    switch (type) {
    case QMetaType::QVariantList: {
        const auto list = value.toList();
        for (const auto &item : list)
            setValue(item);
        break;
    }
    case QMetaType::QVariantMap: {
        const auto map = value.toMap();
        for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
            auto label = new LineEditLabel(it.key(), m_widget);
            // label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
            label->setBuddy(new QLabel(m_widget));
            m_gridLayout->addWidget(label, m_rowIndex, 0, Qt::AlignTop | Qt::AlignRight);
            setValue(it.value());
        }
        break;
    }
    default: {
        auto factory = m_factories.find(type);
        if (factory != m_factories.end()) {
            const auto editor = factory->second->createEditor(value, m_widget);
            m_gridLayout->addWidget(editor, m_rowIndex, 2);
            ++m_rowIndex;
        } else {
            qDebug() << "No editor factory for type" << type;
        }
    }
    }
}

QSize VariantEditor::viewportSizeHint() const
{
    qDebug() << m_widget->minimumSizeHint() << m_widget->sizeHint();
    return m_widget->minimumSizeHint();
}

} // namespace Tiled

#include "moc_varianteditor.cpp"

/*
 * locatorwidget.cpp
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "locatorwidget.h"

#include "documentmanager.h"
#include "filteredit.h"
#include "projectmanager.h"
#include "utils.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStaticText>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>

#include <QDebug>

namespace Tiled {

static QFont scaledFont(const QFont &font, qreal scale)
{
    QFont scaled(font);
    if (font.pixelSize() > 0)
        scaled.setPixelSize(font.pixelSize() * scale);
    else
        scaled.setPointSizeF(font.pointSizeF() * scale);
    return scaled;
}

class FileMatchDelegate : public QStyledItemDelegate
{
public:
    FileMatchDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void setWords(const QStringList &words) { mWords = words; }

private:
    class Fonts {
    public:
        Fonts(const QFont &base)
            : small(scaledFont(base, 0.9))
            , big(scaledFont(base, 1.2))
        {}

        const QFont small;
        const QFont big;
    };

    QStringList mWords;
};

FileMatchDelegate::FileMatchDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{}

QSize FileMatchDelegate::sizeHint(const QStyleOptionViewItem &option,
                                  const QModelIndex &) const
{
    const QFont bigFont = scaledFont(option.font, 1.2);
    const QFontMetrics bigFontMetrics(bigFont);

    const int margin = Utils::dpiScaled(2);
    return QSize(margin * 2, margin * 2 + bigFontMetrics.lineSpacing() * 2);
}

void FileMatchDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    painter->save();

    QString filePath = index.data().toString();
    const int lastSlash = filePath.lastIndexOf(QLatin1Char('/'));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    const auto ranges = Utils::matchingRanges(mWords, &filePath);
#else
    const auto ranges = Utils::matchingRanges(mWords, filePath);
#endif

    filePath = QDir::toNativeSeparators(filePath);

    // Since we're using HTML to markup the entries we'll need to escape the
    // filePath and fileName to avoid them introducing any formatting, however
    // unlikely this may be.
    QString filePathHtml;
    QString fileNameHtml;
    int filePathIndex = 0;

    auto escapedRange = [&] (int first, int last) -> QString {
        return filePath.mid(first, last - first + 1).toHtmlEscaped();
    };

    for (const auto &range : ranges) {
        if (range.first > filePathIndex)
            filePathHtml.append(escapedRange(filePathIndex, range.first - 1));

        filePathHtml.append(QStringLiteral("<b>"));
        filePathHtml.append(escapedRange(range.first, range.second));
        filePathHtml.append(QStringLiteral("</b>"));

        if (range.second > lastSlash) {
            const auto first = qMax(range.first, lastSlash + 1);
            const auto fileNameIndex = qMax(filePathIndex, lastSlash + 1);

            if (first > fileNameIndex)
                fileNameHtml.append(escapedRange(fileNameIndex, first - 1));

            fileNameHtml.append(QStringLiteral("<b>"));
            fileNameHtml.append(escapedRange(first, range.second));
            fileNameHtml.append(QStringLiteral("</b>"));
        }

        filePathIndex = range.second + 1;
    }

    filePathHtml.append(escapedRange(filePathIndex, filePath.size() - 1));
    fileNameHtml.append(escapedRange(qMax(filePathIndex, lastSlash + 1), filePath.size() - 1));

    const Fonts fonts(option.font);
    const QFontMetrics bigFontMetrics(fonts.big);

    const int margin = Utils::dpiScaled(2);
    const auto fileNameRect = option.rect.adjusted(margin, margin, -margin, 0);
    const auto filePathRect = option.rect.adjusted(margin, margin + bigFontMetrics.lineSpacing(), -margin, 0);

    // draw the background (covers selection)
    QStyle *style = QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    // adjust text color to state
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                          ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    QTextOption textOption;
    textOption.setWrapMode(QTextOption::NoWrap);

    QStaticText staticText(fileNameHtml);
    staticText.setTextOption(textOption);
    staticText.setTextFormat(Qt::RichText);

    painter->setFont(fonts.big);
    painter->drawStaticText(fileNameRect.topLeft(), staticText);

    staticText.setText(filePathHtml);

    painter->setOpacity(0.75);
    painter->setFont(fonts.small);
    painter->drawStaticText(filePathRect.topLeft(), staticText);

    // draw the focus rect
    if (option.state & QStyle::State_HasFocus) {
        QStyleOptionFocusRect o;
        o.QStyleOption::operator=(option);
        o.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option);
        o.state |= QStyle::State_KeyboardFocusChange;
        o.state |= QStyle::State_Item;
        QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                ? QPalette::Normal : QPalette::Disabled;
        o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                                ? QPalette::Highlight : QPalette::Window);
        style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
    }

    painter->restore();
}

///////////////////////////////////////////////////////////////////////////////

class ResultsView : public QTreeView
{
public:
    explicit ResultsView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void updateMaximumHeight();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};

ResultsView::ResultsView(QWidget *parent)
    : QTreeView(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QSize ResultsView::sizeHint() const
{
    return QSize(Utils::dpiScaled(600), Utils::dpiScaled(600));
}

void ResultsView::updateMaximumHeight()
{
    int maximumHeight = 0;

    if (auto m = model()) {
        int rowCount = m->rowCount();
        if (rowCount > 0) {
            const int itemHeight = indexRowSizeHint(m->index(0, 0));
            maximumHeight = itemHeight * rowCount;
        }
    }

    setMaximumHeight(maximumHeight);
}

void ResultsView::keyPressEvent(QKeyEvent *event)
{
    // Make sure the Enter and Return keys activate the current index. This
    // doesn't happen otherwise on macOS.
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (currentIndex().isValid())
            emit activated(currentIndex());
        return;
    }

    QTreeView::keyPressEvent(event);
}

///////////////////////////////////////////////////////////////////////////////

LocatorWidget::LocatorWidget(LocatorSource *locatorSource,
                             QWidget *parent)
    : QFrame(parent, Qt::Popup)
    , mLocatorSource(locatorSource)
    , mFilterEdit(new FilterEdit(this))
    , mResultsView(new ResultsView(this))
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

    mLocatorSource->setParent(this);        // take ownership of source

    mResultsView->setUniformRowHeights(true);
    mResultsView->setRootIsDecorated(false);
    mResultsView->setItemDelegate(mLocatorSource->delegate());
    mResultsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mResultsView->setModel(mLocatorSource);
    mResultsView->setHeaderHidden(true);

    mFilterEdit->setPlaceholderText(mLocatorSource->placeholderText());
    mFilterEdit->setFilteredView(mResultsView);
    mFilterEdit->setClearTextOnEscape(false);
    mFilterEdit->setFont(scaledFont(mFilterEdit->font(), 1.5));

    setFocusProxy(mFilterEdit);
    mResultsView->setFocusProxy(mFilterEdit);

    mResultsView->setFrameShape(QFrame::NoFrame);
    mResultsView->viewport()->setBackgroundRole(QPalette::Window);

    auto margin = Utils::dpiScaled(4);
    auto verticalLayout = new QVBoxLayout;
    verticalLayout->setContentsMargins(margin, margin, margin, margin);
    verticalLayout->setSpacing(margin);
    verticalLayout->addWidget(mFilterEdit);
    verticalLayout->addWidget(mResultsView);
    verticalLayout->addStretch(0);
    setLayout(verticalLayout);

    connect(mFilterEdit, &QLineEdit::textChanged, this, &LocatorWidget::setFilterText);
    connect(mResultsView, &QAbstractItemView::activated, this, [this] (const QModelIndex &index) {
        close();
        mLocatorSource->activate(index);
    });
}

void LocatorWidget::setVisible(bool visible)
{
    QFrame::setVisible(visible);

    if (visible) {
        setFocus();

        if (!mFilterEdit->text().isEmpty())
            mFilterEdit->clear();
        else
            setFilterText(QString());
    }
}

void LocatorWidget::setFilterText(const QString &text)
{
    const QString normalized = QDir::fromNativeSeparators(text);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QStringList words = normalized.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
    const QStringList words = normalized.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif

    mLocatorSource->setFilterWords(words);

    mResultsView->updateGeometry();
    mResultsView->updateMaximumHeight();

    // Restore or introduce selection
    if (auto index = mLocatorSource->index(0, 0); index.isValid())
        mResultsView->setCurrentIndex(index);

    layout()->activate();
    resize(sizeHint());
}

///////////////////////////////////////////////////////////////////////////////

FileLocatorSource::FileLocatorSource(QObject *parent)
    : LocatorSource(parent)
    , mDelegate(new FileMatchDelegate(this))
{}

int FileLocatorSource::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mMatches.size();
}

QVariant FileLocatorSource::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        const ProjectModel::Match &match = mMatches.at(index.row());
        return match.relativePath().toString();
    }
    }
    return QVariant();
}

QAbstractItemDelegate *FileLocatorSource::delegate() const
{
    return mDelegate;
}

QString FileLocatorSource::placeholderText() const
{
    return QCoreApplication::translate("Tiled::LocatorWidget", "Filename");
}

void FileLocatorSource::activate(const QModelIndex &index)
{
    const QString file = mMatches.at(index.row()).path;
    DocumentManager::instance()->openFile(file);
}

void FileLocatorSource::setFilterWords(const QStringList &words)
{
    auto projectModel = ProjectManager::instance()->projectModel();
    auto matches = projectModel->findFiles(words);

    std::stable_sort(matches.begin(), matches.end(), [] (const ProjectModel::Match &a, const ProjectModel::Match &b) {
        // Sort based on score first
        if (a.score != b.score)
            return a.score > b.score;

        // If score is the same, sort alphabetically
        return a.relativePath().compare(b.relativePath(), Qt::CaseInsensitive) < 0;
    });

    mDelegate->setWords(words);

    beginResetModel();
    mMatches = std::move(matches);
    endResetModel();
}

} // namespace Tiled

#include "moc_locatorwidget.cpp"

/*
 * prolificstyle.cpp
 * Copyright 2023, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "prolificstyle.h"

#include <QAbstractButton>
#include <QAbstractItemView>
#include <QAbstractSlider>
#include <QAbstractSpinBox>
#include <QComboBox>
#include <QPainter>
#include <QProgressBar>
#include <QScrollBar>
#include <QSplitterHandle>
#include <QStyleOption>

namespace Tiled {

// todo: de-duplicate between tiledproxystyle.cpp

#ifdef Q_OS_DARWIN
static const qreal baseDpi = 72;
#else
static const qreal baseDpi = 96;
#endif

static qreal dpi(const QStyleOption *option)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    // Expect that QStyleOption::QFontMetrics::QFont has the correct DPI set
    if (option)
        return option->fontMetrics.fontDpi();
    return baseDpi;
#else
    Q_UNUSED(option)
    return Utils::defaultDpi();
#endif
}

static qreal dpiScaled(qreal value, qreal dpi)
{
    return value * dpi / baseDpi;
}

static int dpiScaled(int value, const QStyleOption *option)
{
    return qRound(dpiScaled(qreal(value), dpi(option)));
}


ProlificStyle::ProlificStyle()
{

}

void ProlificStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (pe) {
    case PE_IndicatorButtonDropDown:
    case PE_PanelButtonBevel:
    case PE_PanelButtonCommand:
    case PE_PanelButtonTool:
        p->save();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(opt->palette.light().color(), 1.0));
        p->translate(0.5, 0.5);
        p->drawRoundedRect(opt->rect.adjusted(1, 1, -1, -1), 5.0, 5.0);
        p->restore();
        return;
    case PE_CustomBase:
    case PE_Frame:
    case PE_FrameButtonBevel:
    case PE_FrameButtonTool:
    case PE_FrameDefaultButton:
    case PE_FrameDockWidget:
    case PE_FrameFocusRect:
    case PE_FrameGroupBox:
    case PE_FrameLineEdit:
    case PE_FrameMenu:
    case PE_FrameStatusBarItem:
    case PE_FrameTabBarBase:
    case PE_FrameTabWidget:
    case PE_FrameWindow:
    case PE_IndicatorArrowDown:
    case PE_IndicatorArrowLeft:
    case PE_IndicatorArrowRight:
    case PE_IndicatorArrowUp:
    case PE_IndicatorBranch:
    case PE_IndicatorCheckBox:
    case PE_IndicatorColumnViewArrow:
    case PE_IndicatorDockWidgetResizeHandle:
    case PE_IndicatorHeaderArrow:
    case PE_IndicatorItemViewItemCheck:
    case PE_IndicatorItemViewItemDrop:
    case PE_IndicatorMenuCheckMark:
    case PE_IndicatorProgressChunk:
    case PE_IndicatorRadioButton:
    case PE_IndicatorSpinDown:
    case PE_IndicatorSpinMinus:
    case PE_IndicatorSpinPlus:
    case PE_IndicatorSpinUp:
    case PE_IndicatorTabClose:
    case PE_IndicatorTabTear:
    case PE_IndicatorTabTearRight:
    case PE_IndicatorToolBarHandle:
    case PE_IndicatorToolBarSeparator:
    case PE_PanelItemViewItem:
    case PE_PanelItemViewRow:
    case PE_PanelLineEdit:
    case PE_PanelMenu:
    case PE_PanelMenuBar:
    case PE_PanelScrollAreaCorner:
    case PE_PanelStatusBar:
    case PE_PanelTipLabel:
    case PE_PanelToolBar:
    case PE_Widget:
        break;
    }

    QCommonStyle::drawPrimitive(pe, opt, p, w);
}

void ProlificStyle::drawControl(ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    switch (element) {
    case CE_ScrollBarAddLine:
    case CE_ScrollBarAddPage:
    case CE_ScrollBarSubLine:
    case CE_ScrollBarSubPage:
        return; // this style does not support these control elements
    case CE_ScrollBarSlider:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            scrollBarDrawSlider(scrollBar, p);
        return;
    case CE_PushButton:
    case CE_PushButtonBevel:
    case CE_CheckBox:
    case CE_CheckBoxLabel:
    case CE_DockWidgetTitle:
    case CE_Header:
    case CE_HeaderLabel:
    case CE_HeaderSection:
    case CE_MenuBarEmptyArea:
    case CE_MenuBarItem:
    case CE_MenuEmptyArea:
    case CE_MenuHMargin:
    case CE_MenuItem:
    case CE_MenuScroller:
    case CE_MenuTearoff:
    case CE_MenuVMargin:
    case CE_ProgressBar:
    case CE_ProgressBarContents:
    case CE_ProgressBarGroove:
    case CE_ProgressBarLabel:
    case CE_PushButtonLabel:
    case CE_RadioButton:
    case CE_RadioButtonLabel:
    case CE_RubberBand:
    case CE_SizeGrip:
    case CE_Splitter:
    case CE_TabBarTab:
    case CE_TabBarTabLabel:
    case CE_TabBarTabShape:
    case CE_ToolBoxTab:
    case CE_ToolButtonLabel:
    case CE_ColumnViewGrip:
    case CE_ComboBoxLabel:
    case CE_CustomBase:
    case CE_FocusFrame:
    case CE_HeaderEmptyArea:
    case CE_ItemViewItem:
    case CE_ScrollBarFirst:
    case CE_ScrollBarLast:
    case CE_ShapedFrame:
    case CE_ToolBar:
    case CE_ToolBoxTabLabel:
    case CE_ToolBoxTabShape:
        break;
    }

    QCommonStyle::drawControl(element, opt, p, w);
}

QRect ProlificStyle::subElementRect(SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    switch (r) {
    case SE_CheckBoxClickRect:
    case SE_CheckBoxContents:
    case SE_CheckBoxFocusRect:
    case SE_CheckBoxIndicator:
    case SE_CheckBoxLayoutItem:
    case SE_ComboBoxFocusRect:
    case SE_ComboBoxLayoutItem:
    case SE_CustomBase:
    case SE_DateTimeEditLayoutItem:
    case SE_DockWidgetCloseButton:
    case SE_DockWidgetFloatButton:
    case SE_DockWidgetIcon:
    case SE_DockWidgetTitleBarText:
    case SE_FrameContents:
    case SE_FrameLayoutItem:
    case SE_GroupBoxLayoutItem:
    case SE_HeaderArrow:
    case SE_HeaderLabel:
    case SE_ItemViewItemCheckIndicator:
    case SE_ItemViewItemDecoration:
    case SE_ItemViewItemFocusRect:
    case SE_ItemViewItemText:
    case SE_LabelLayoutItem:
    case SE_LineEditContents:
    case SE_ProgressBarContents:
    case SE_ProgressBarGroove:
    case SE_ProgressBarLabel:
    case SE_ProgressBarLayoutItem:
    case SE_PushButtonBevel:
    case SE_PushButtonContents:
    case SE_PushButtonFocusRect:
    case SE_PushButtonLayoutItem:
    case SE_RadioButtonClickRect:
    case SE_RadioButtonContents:
    case SE_RadioButtonFocusRect:
    case SE_RadioButtonIndicator:
    case SE_RadioButtonLayoutItem:
    case SE_ShapedFrameContents:
    case SE_SliderFocusRect:
    case SE_SliderLayoutItem:
    case SE_SpinBoxLayoutItem:
    case SE_TabBarScrollLeftButton:
    case SE_TabBarScrollRightButton:
    case SE_TabBarTabLeftButton:
    case SE_TabBarTabRightButton:
    case SE_TabBarTabText:
    case SE_TabBarTearIndicator:
    case SE_TabBarTearIndicatorRight:
    case SE_TabWidgetLayoutItem:
    case SE_TabWidgetLeftCorner:
    case SE_TabWidgetRightCorner:
    case SE_TabWidgetTabBar:
    case SE_TabWidgetTabContents:
    case SE_TabWidgetTabPane:
    case SE_ToolBarHandle:
    case SE_ToolBoxTabContents:
    case SE_ToolButtonLayoutItem:
    case SE_TreeViewDisclosureItem:
        break;
    }

    return QCommonStyle::subElementRect(r, opt, widget);
}

void ProlificStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            scrollBarDrawGroove(scrollBar, p);

            QStyleOptionSlider newScrollBar = *scrollBar;
            newScrollBar.subControls &= SC_ScrollBarSlider; // we only need the slider to be rendered
            QCommonStyle::drawComplexControl(cc, &newScrollBar, p, w);
        }
        return;
    case CC_ComboBox:
    case CC_CustomBase:
    case CC_Dial:
    case CC_GroupBox:
    case CC_MdiControls:
    case CC_Slider:
    case CC_SpinBox:
    case CC_TitleBar:
    case CC_ToolButton:
        break;
    }

    QCommonStyle::drawComplexControl(cc, opt, p, w);
}

QRect ProlificStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc, const QWidget *w) const
{
    switch (cc) {
    case CC_ScrollBar:
        if (const QStyleOptionSlider *scrollBar = qstyleoption_cast<const QStyleOptionSlider *>(opt))
            return scrollBarSubControlRect(scrollBar, sc, w);
    case CC_ComboBox:
    case CC_CustomBase:
    case CC_Dial:
    case CC_GroupBox:
    case CC_MdiControls:
    case CC_Slider:
    case CC_SpinBox:
    case CC_TitleBar:
    case CC_ToolButton:
        break;
    }

    return QCommonStyle::subControlRect(cc, opt, sc, w);
}

int ProlificStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    switch (m) {
    case PM_ScrollBarExtent:
        return scrollBarWidth(opt);
    case PM_ScrollView_ScrollBarOverlap:
        return mScrollBarOverlaps ? scrollBarWidth(opt) : 0;
    case PM_ScrollBarSliderMin:
        return scrollBarSliderMin(opt);
    case PM_ButtonDefaultIndicator:
    case PM_ButtonIconSize:
    case PM_ButtonMargin:
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
    case PM_CheckBoxLabelSpacing:
    case PM_ComboBoxFrameWidth:
    case PM_CustomBase:
    case PM_DefaultFrameWidth:
    case PM_DialogButtonsButtonHeight:
    case PM_DialogButtonsButtonWidth:
    case PM_DialogButtonsSeparator:
    case PM_DockWidgetFrameWidth:
    case PM_DockWidgetHandleExtent:
    case PM_DockWidgetSeparatorExtent:
    case PM_DockWidgetTitleBarButtonMargin:
    case PM_DockWidgetTitleMargin:
    case PM_ExclusiveIndicatorHeight:
    case PM_ExclusiveIndicatorWidth:
    case PM_FocusFrameHMargin:
    case PM_FocusFrameVMargin:
    case PM_HeaderDefaultSectionSizeHorizontal:
    case PM_HeaderDefaultSectionSizeVertical:
    case PM_HeaderGripMargin:
    case PM_HeaderMargin:
    case PM_HeaderMarkSize:
    case PM_IconViewIconSize:
    case PM_IndicatorHeight:
    case PM_IndicatorWidth:
    case PM_LargeIconSize:
    case PM_LayoutBottomMargin:
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutLeftMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutVerticalSpacing:
    case PM_LineEditIconMargin:
    case PM_LineEditIconSize:
    case PM_ListViewIconSize:
    case PM_MaximumDragDistance:
    case PM_MdiSubWindowFrameWidth:
    case PM_MdiSubWindowMinimizedWidth:
    case PM_MenuBarHMargin:
    case PM_MenuBarItemSpacing:
    case PM_MenuBarPanelWidth:
    case PM_MenuBarVMargin:
    case PM_MenuButtonIndicator:
    case PM_MenuDesktopFrameWidth:
    case PM_MenuHMargin:
    case PM_MenuPanelWidth:
    case PM_MenuScrollerHeight:
    case PM_MenuTearoffHeight:
    case PM_MenuVMargin:
    case PM_MessageBoxIconSize:
    case PM_ProgressBarChunkWidth:
    case PM_RadioButtonLabelSpacing:
    case PM_ScrollView_ScrollBarSpacing:
    case PM_SizeGripSize:
    case PM_SliderControlThickness:
    case PM_SliderLength:
    case PM_SliderSpaceAvailable:
    case PM_SliderThickness:
    case PM_SliderTickmarkOffset:
    case PM_SmallIconSize:
    case PM_SpinBoxFrameWidth:
    case PM_SpinBoxSliderHeight:
    case PM_SplitterWidth:
    case PM_SubMenuOverlap:
    case PM_TabBarBaseHeight:
    case PM_TabBarBaseOverlap:
    case PM_TabBarIconSize:
    case PM_TabBarScrollButtonWidth:
    case PM_TabBarTabHSpace:
    case PM_TabBarTabOverlap:
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
    case PM_TabBarTabVSpace:
    case PM_TabBar_ScrollButtonOverlap:
    case PM_TabCloseIndicatorHeight:
    case PM_TabCloseIndicatorWidth:
    case PM_TextCursorWidth:
    case PM_TitleBarButtonIconSize:
    case PM_TitleBarButtonSize:
    case PM_TitleBarHeight:
    case PM_ToolBarExtensionExtent:
    case PM_ToolBarFrameWidth:
    case PM_ToolBarHandleExtent:
    case PM_ToolBarIconSize:
    case PM_ToolBarItemMargin:
    case PM_ToolBarItemSpacing:
    case PM_ToolBarSeparatorExtent:
    case PM_ToolTipLabelFrameWidth:
    case PM_TreeViewIndentation:
        break;
    }

    return QCommonStyle::pixelMetric(m, opt, widget);
}

int ProlificStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w, QStyleHintReturn *shret) const
{
    switch (sh) {
    case SH_ScrollBar_MiddleClickAbsolutePosition:
    case SH_Slider_StopMouseOverSlider:
        return true;
    case SH_ItemView_ScrollMode:
        return QAbstractItemView::ScrollPerPixel;
    case SH_BlinkCursorWhenTextSelected:
    case SH_Button_FocusPolicy:
    case SH_ComboBox_AllowWheelScrolling:
    case SH_ComboBox_LayoutDirection:
    case SH_ComboBox_ListMouseTracking:
    case SH_ComboBox_Popup:
    case SH_ComboBox_PopupFrameStyle:
    case SH_ComboBox_UseNativePopup:
    case SH_CustomBase:
    case SH_Dial_BackgroundRole:
    case SH_DialogButtonBox_ButtonsHaveIcons:
    case SH_DialogButtonLayout:
    case SH_DialogButtons_DefaultButton:
    case SH_DitherDisabledText:
    case SH_DockWidget_ButtonsHaveFrame:
    case SH_DrawMenuBarSeparator:
    case SH_EtchDisabledText:
    case SH_FocusFrame_AboveWidget:
    case SH_FocusFrame_Mask:
    case SH_FontDialog_SelectAssociatedText:
    case SH_FormLayoutFieldGrowthPolicy:
    case SH_FormLayoutFormAlignment:
    case SH_FormLayoutLabelAlignment:
    case SH_FormLayoutWrapPolicy:
    case SH_GroupBox_TextLabelColor:
    case SH_GroupBox_TextLabelVerticalAlignment:
    case SH_Header_ArrowAlignment:
    case SH_ItemView_ActivateItemOnSingleClick:
    case SH_ItemView_ArrowKeysNavigateIntoChildren:
    case SH_ItemView_ChangeHighlightOnFocus:
    case SH_ItemView_DrawDelegateFrame:
    case SH_ItemView_EllipsisLocation:
    case SH_ItemView_MovementWithoutUpdatingSelection:
    case SH_ItemView_PaintAlternatingRowColorsForEmptyArea:
    case SH_ItemView_ShowDecorationSelected:
    case SH_LineEdit_PasswordCharacter:
    case SH_LineEdit_PasswordMaskDelay:
    case SH_ListViewExpand_SelectMouseType:
    case SH_MainWindow_SpaceBelowMenuBar:
    case SH_MenuBar_AltKeyNavigation:
    case SH_MenuBar_MouseTracking:
    case SH_Menu_AllowActiveAndDisabled:
    case SH_Menu_FadeOutOnHide:
    case SH_Menu_FillScreenWithScroll:
    case SH_Menu_FlashTriggeredItem:
    case SH_Menu_KeyboardSearch:
    case SH_Menu_Mask:
    case SH_Menu_MouseTracking:
    case SH_Menu_Scrollable:
    case SH_Menu_SelectionWrap:
    case SH_Menu_SloppySubMenus:
    case SH_Menu_SpaceActivatesItem:
    case SH_Menu_SubMenuDontStartSloppyOnLeave:
    case SH_Menu_SubMenuPopupDelay:
    case SH_Menu_SubMenuResetWhenReenteringParent:
    case SH_Menu_SubMenuSloppyCloseTimeout:
    case SH_Menu_SubMenuSloppySelectOtherActions:
    case SH_Menu_SubMenuUniDirection:
    case SH_Menu_SubMenuUniDirectionFailCount:
    case SH_Menu_SupportsSections:
    case SH_MessageBox_CenterButtons:
    case SH_MessageBox_TextInteractionFlags:
    case SH_MessageBox_UseBorderForButtonSpacing:
    case SH_PrintDialog_RightAlignButtons:
    case SH_ProgressDialog_CenterCancelButton:
    case SH_ProgressDialog_TextLabelAlignment:
    case SH_RequestSoftwareInputPanel:
    case SH_RichText_FullWidthSelection:
    case SH_RubberBand_Mask:
    case SH_ScrollBar_ContextMenu:
    case SH_ScrollBar_LeftClickAbsolutePosition:
    case SH_ScrollBar_RollBetweenButtons:
    case SH_ScrollBar_ScrollWhenPointerLeavesControl:
    case SH_ScrollBar_Transient:
    case SH_ScrollView_FrameOnlyAroundContents:
    case SH_Slider_AbsoluteSetButtons:
    case SH_Slider_PageSetButtons:
    case SH_Slider_SloppyKeyEvents:
    case SH_Slider_SnapToValue:
    case SH_SpinBox_AnimateButton:
    case SH_SpinBox_ButtonsInsideFrame:
    case SH_SpinBox_ClickAutoRepeatRate:
    case SH_SpinBox_ClickAutoRepeatThreshold:
    case SH_SpinBox_KeyPressAutoRepeatRate:
    case SH_SpinBox_SelectOnStep:
    case SH_SpinBox_StepModifier:
    case SH_SpinControls_DisableOnBounds:
    case SH_Splitter_OpaqueResize:
    case SH_TabBar_Alignment:
    case SH_TabBar_AllowWheelScrolling:
    case SH_TabBar_ChangeCurrentDelay:
    case SH_TabBar_CloseButtonPosition:
    case SH_TabBar_ElideMode:
    case SH_TabBar_PreferNoArrows:
    case SH_TabBar_SelectMouseType:
    case SH_TabWidget_DefaultTabPosition:
    case SH_Table_AlwaysDrawLeftTopGridLines:
    case SH_Table_GridLineColor:
    case SH_TextControl_FocusIndicatorTextCharFormat:
    case SH_TitleBar_AutoRaise:
    case SH_TitleBar_ModifyNotification:
    case SH_TitleBar_NoBorder:
    case SH_TitleBar_ShowToolTipsOnButtons:
    case SH_ToolBar_Movable:
    case SH_ToolBox_SelectedPageTitleBold:
    case SH_ToolButtonStyle:
    case SH_ToolButton_PopupDelay:
    case SH_ToolTipLabel_Opacity:
    case SH_ToolTip_FallAsleepDelay:
    case SH_ToolTip_Mask:
    case SH_ToolTip_WakeUpDelay:
    case SH_UnderlineShortcut:
    case SH_Widget_Animate:
    case SH_Widget_Animation_Duration:
    case SH_Widget_ShareActivation:
    case SH_WindowFrame_Mask:
    case SH_WizardStyle:
    case SH_Workspace_FillSpaceOnMaximize:
        break;
    }

    return QCommonStyle::styleHint(sh, opt, w, shret);
}

static bool doesHoverOrNonOpaquePaint(QWidget *w)
{
    return (qobject_cast<QAbstractButton*>(w) ||
            qobject_cast<QComboBox *>(w) ||
            qobject_cast<QProgressBar *>(w) ||
            qobject_cast<QScrollBar *>(w) ||
            qobject_cast<QSplitterHandle *>(w) ||
            qobject_cast<QAbstractSlider *>(w) ||
            qobject_cast<QAbstractSpinBox *>(w) ||
            w->inherits("QDockSeparator") ||
            w->inherits("QDockWidgetSeparator"));
}

void ProlificStyle::polish(QWidget *w)
{
    QCommonStyle::polish(w);

    if (doesHoverOrNonOpaquePaint(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, false);
        w->setAttribute(Qt::WA_Hover, true);
    }
}

void ProlificStyle::unpolish(QWidget *w)
{
    if (doesHoverOrNonOpaquePaint(w)) {
        w->setAttribute(Qt::WA_OpaquePaintEvent, true);
        w->setAttribute(Qt::WA_Hover, false);
    }

    QCommonStyle::unpolish(w);
}

QRect ProlificStyle::scrollBarSubControlRect(const QStyleOptionSlider *opt,
                                             SubControl sc,
                                             const QWidget *w) const
{
    switch (sc) {
    default:
    case SC_ScrollBarAddLine:
    case SC_ScrollBarSubLine:
    case SC_ScrollBarFirst:
    case SC_ScrollBarLast:
        // These sub-controls are not supported by this style
        return QRect();

    case SC_ScrollBarGroove:
        // The groove covers the entire widget
        return opt->rect;

    case SC_ScrollBarSlider:
    case SC_ScrollBarAddPage:
    case SC_ScrollBarSubPage:
        break;
    }

    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    const QSize scrollBarSize = isHorizontal ? opt->rect.size().transposed()
                                             : opt->rect.size();

    const int maxlen = scrollBarSize.height();
    int sliderlen = maxlen;

    // calculate slider length
    if (opt->maximum != opt->minimum) {
        uint range = opt->maximum - opt->minimum;
        sliderlen = (qint64(opt->pageStep) * maxlen) / (range + opt->pageStep);

        int slidermin = proxy()->pixelMetric(PM_ScrollBarSliderMin, opt, w);
        if (sliderlen < slidermin || range > INT_MAX / 2)
            sliderlen = slidermin;
        if (sliderlen > maxlen)
            sliderlen = maxlen;
    }

    const int sliderstart = sliderPositionFromValue(opt->minimum,
                                                    opt->maximum,
                                                    opt->sliderPosition,
                                                    maxlen - sliderlen,
                                                    opt->upsideDown);

    QRect ret;

    switch (sc) {
    case SC_ScrollBarSlider:
        ret.setRect(0, sliderstart, scrollBarSize.width(), sliderlen);
        break;
    case SC_ScrollBarAddPage:
        ret.setRect(0, sliderstart + sliderlen,
                    scrollBarSize.width(), maxlen - sliderstart - sliderlen);
        break;
    case SC_ScrollBarSubPage:
        ret.setRect(0, 0, scrollBarSize.width(), sliderstart);
        break;
    default:
        break;
    }

    if (isHorizontal)
        ret.setRect(ret.y(), ret.x(), ret.height(), ret.width());

    return visualRect(opt->direction, opt->rect, ret);
}

void ProlificStyle::scrollBarDrawGroove(const QStyleOptionSlider *opt, QPainter *p) const
{
    // Draw the background only on mouse hover or while the scroll bar is used
    if (mScrollBarOverlaps && !opt->activeSubControls)
        return;

    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    QRect rect = opt->rect;
    const int margin = (isHorizontal ? rect.height()
                                     : rect.width()) / 8;
    rect.adjust(margin, margin, -margin, -margin);
    const qreal radius = std::min(rect.width(), rect.height()) * 0.5;

    QColor color = opt->palette.dark().color();
    if (mScrollBarOverlaps)
        color.setAlpha(128);

    p->save();

    p->setPen(Qt::NoPen);
    p->setBrush(color);
    p->setRenderHint(QPainter::Antialiasing);

    p->drawRoundedRect(rect, radius, radius);

    p->restore();
}

void ProlificStyle::scrollBarDrawSlider(const QStyleOptionSlider *opt, QPainter *p) const
{
    const bool isHorizontal = opt->orientation == Qt::Horizontal;
    QRect rect = opt->rect;
    if (mScrollBarOverlaps && !opt->activeSubControls) {
        if (isHorizontal)
            rect.adjust(0, rect.height() / 2, 0, 0);
        else
            rect.adjust(rect.width() / 2, 0, 0, 0);
    }

    const int margin = (isHorizontal ? rect.height()
                                     : rect.width()) / 4;
    rect.adjust(margin, margin, -margin, -margin);
    const qreal radius = std::min(rect.width(), rect.height()) * 0.5;

    // Use text color for slider, since it contrasts well with the base color
    const bool isPressed = opt->state & State_Sunken;
    QColor foreground = isPressed ? opt->palette.highlight().color()
                                  : opt->palette.text().color();
    if (!isPressed && (mScrollBarOverlaps || !opt->activeSubControls))
        foreground.setAlpha(192);

    p->save();

    p->setPen(Qt::NoPen);
    p->setBrush(foreground);
    p->setRenderHint(QPainter::Antialiasing);

    p->drawRoundedRect(rect, radius, radius);

    p->restore();
}

int ProlificStyle::scrollBarWidth(const QStyleOption *opt) const
{
    return dpiScaled(8, opt);
}

int ProlificStyle::scrollBarSliderMin(const QStyleOption *opt) const
{
    return dpiScaled(12, opt);
}

} // namespace Tiled

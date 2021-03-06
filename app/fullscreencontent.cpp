// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
// Self
#include "fullscreencontent.h"

// Qt
#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QEvent>
#include <QLabel>
#include <QMenu>
#include <QPointer>
#include <QToolButton>
#include <QWidgetAction>

// KDE
#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KIconLoader>

// Local
#include <lib/document/documentfactory.h>
#include <lib/eventwatcher.h>
#include <lib/fullscreenbar.h>
#include <lib/gwenviewconfig.h>
#include <lib/imagemetainfomodel.h>
#include <lib/thumbnailview/thumbnailbarview.h>
#include <lib/shadowfilter.h>
#include <lib/slideshow.h>

namespace Gwenview
{

/**
 * A widget which behaves more or less like a QToolBar, but which uses real
 * widgets for the toolbar items. We need a real widget to be able to position
 * the option menu.
 */
class FullScreenToolBar : public QWidget
{
public:
    FullScreenToolBar(QWidget* parent = 0)
    : QWidget(parent)
    , mLayout(new QHBoxLayout(this))
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        mLayout->setSpacing(0);
        mLayout->setMargin(0);
    }

    void addAction(QAction* action)
    {
        QToolButton* button = new QToolButton;
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonIconOnly);
        button->setAutoRaise(true);
        const int extent = KIconLoader::SizeMedium;
        button->setIconSize(QSize(extent, extent));
        mLayout->addWidget(button);
    }

    void addSeparator()
    {
        mLayout->addSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    }

    void addStretch()
    {
        mLayout->addStretch();
    }

    void setDirection(QBoxLayout::Direction direction)
    {
        mLayout->setDirection(direction);
    }

private:
    QBoxLayout* mLayout;
};


FullScreenContent::FullScreenContent(QObject* parent)
: QObject(parent)
{
    mFullScreenMode = false;
    mViewPageVisible = false;
}

void FullScreenContent::init(KActionCollection* actionCollection, QWidget* autoHideParentWidget, SlideShow* slideShow)
{
    mSlideShow = slideShow;
    mActionCollection = actionCollection;
    connect(actionCollection->action("view"), SIGNAL(toggled(bool)),
        SLOT(slotViewModeActionToggled(bool)));

    // mAutoHideContainer
    mAutoHideContainer = new FullScreenBar(autoHideParentWidget);
    mAutoHideContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QVBoxLayout* layout = new QVBoxLayout(mAutoHideContainer);
    layout->setMargin(0);
    layout->setSpacing(0);

    // mContent
    mContent = new QWidget;
    mContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mContent->setAutoFillBackground(true);
    EventWatcher::install(mContent, QEvent::Show, this, SLOT(updateCurrentUrlWidgets()));
    EventWatcher::install(mContent, QEvent::PaletteChange, this, SLOT(slotPaletteChanged()));
    layout->addWidget(mContent);

    createOptionsAction();

    // mToolBar
    mToolBar = new FullScreenToolBar(mContent);

    #define addAction(name) mToolBar->addAction(actionCollection->action(name))
    addAction("browse");
    addAction("view");
    mToolBar->addSeparator();
    addAction("go_previous");
    addAction("toggle_slideshow");
    addAction("go_next");
    mToolBar->addSeparator();
    addAction("rotate_left");
    addAction("rotate_right");
    #undef addAction
    mToolBarShadow = new ShadowFilter(mToolBar);

    // mInformationLabel
    mInformationLabel = new QLabel;
    mInformationLabel->setWordWrap(true);
    mInformationLabel->setContentsMargins(6, 0, 6, 0);
    mInformationLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    mInformationLabel->setAutoFillBackground(true);
    mInformationLabelShadow = new ShadowFilter(mInformationLabel);

    // Thumbnail bar
    mThumbnailBar = new ThumbnailBarView(mContent);
    mThumbnailBar->setThumbnailScaleMode(ThumbnailView::ScaleToSquare);
    ThumbnailBarItemDelegate* delegate = new ThumbnailBarItemDelegate(mThumbnailBar);
    mThumbnailBar->setItemDelegate(delegate);
    mThumbnailBar->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Right bar
    mRightToolBar = new FullScreenToolBar(mContent);
    mRightToolBar->addAction(mActionCollection->action("leave_fullscreen"));
    mRightToolBar->addAction(mOptionsAction);
    mRightToolBar->addStretch();
    mRightToolBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    mRightToolBarShadow = new ShadowFilter(mRightToolBar);

    updateLayout();

    updateContainerAppearance();
}

ThumbnailBarView* FullScreenContent::thumbnailBar() const
{
    return mThumbnailBar;
}

void FullScreenContent::setCurrentUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        mCurrentDocument = Document::Ptr();
    } else {
        mCurrentDocument = DocumentFactory::instance()->load(url);
        connect(mCurrentDocument.data(), SIGNAL(metaInfoUpdated()),
                SLOT(updateCurrentUrlWidgets()));
    }
    updateCurrentUrlWidgets();
}

void FullScreenContent::updateInformationLabel()
{
    if (!mCurrentDocument) {
        return;
    }

    if (!mInformationLabel->isVisible()) {
        return;
    }

    ImageMetaInfoModel* model = mCurrentDocument->metaInfo();

    QStringList valueList;
    Q_FOREACH(const QString & key, GwenviewConfig::fullScreenPreferredMetaInfoKeyList()) {
        const QString value = model->getValueForKey(key);
        if (value.length() > 0) {
            valueList << value;
        }
    }
    QString text = valueList.join(i18nc("@item:intext fullscreen meta info separator", ", "));

    mInformationLabel->setText(text);
}

void FullScreenContent::slotPaletteChanged()
{
    QPalette pal = mContent->palette();
    pal.setColor(QPalette::Window, pal.color(QPalette::Window).dark(110));
    mInformationLabel->setPalette(pal);
}

void FullScreenContent::updateCurrentUrlWidgets()
{
    updateInformationLabel();
    updateMetaInfoDialog();
}

void FullScreenContent::showImageMetaInfoDialog()
{
    if (!mImageMetaInfoDialog) {
        mImageMetaInfoDialog = new ImageMetaInfoDialog(mInformationLabel);
        // Do not let the fullscreen theme propagate to this dialog for now,
        // it's already quite complicated to create a theme
        mImageMetaInfoDialog->setStyle(QApplication::style());
        mImageMetaInfoDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        connect(mImageMetaInfoDialog, SIGNAL(preferredMetaInfoKeyListChanged(QStringList)),
                SLOT(slotPreferredMetaInfoKeyListChanged(QStringList)));
        connect(mImageMetaInfoDialog, SIGNAL(destroyed()),
                SLOT(slotImageMetaInfoDialogClosed()));
    }
    if (mCurrentDocument) {
        mImageMetaInfoDialog->setMetaInfo(mCurrentDocument->metaInfo(), GwenviewConfig::fullScreenPreferredMetaInfoKeyList());
    }
    mAutoHideContainer->setAutoHidingEnabled(false);
    mImageMetaInfoDialog->show();
}

void FullScreenContent::slotPreferredMetaInfoKeyListChanged(const QStringList& list)
{
    GwenviewConfig::setFullScreenPreferredMetaInfoKeyList(list);
    GwenviewConfig::self()->save();
    updateInformationLabel();
}

void FullScreenContent::updateMetaInfoDialog()
{
    if (!mImageMetaInfoDialog) {
        return;
    }
    ImageMetaInfoModel* model = mCurrentDocument ? mCurrentDocument->metaInfo() : 0;
    mImageMetaInfoDialog->setMetaInfo(model, GwenviewConfig::fullScreenPreferredMetaInfoKeyList());
}

static QString formatSlideShowIntervalText(int value)
{
    return i18ncp("Slideshow interval in seconds", "%1 sec", "%1 secs", value);
}

void FullScreenContent::updateLayout()
{
    delete mContent->layout();

    if (GwenviewConfig::showFullScreenThumbnails()) {
        mRightToolBar->setDirection(QBoxLayout::TopToBottom);

        QHBoxLayout* layout = new QHBoxLayout(mContent);
        layout->setMargin(0);
        layout->setSpacing(0);
        QVBoxLayout* vLayout;

        // First column
        vLayout = new QVBoxLayout;
        vLayout->addWidget(mToolBar);
        vLayout->addWidget(mInformationLabel);
        layout->addLayout(vLayout);
        // Second column
        layout->addSpacing(2);
        layout->addWidget(mThumbnailBar);
        layout->addSpacing(2);
        // Third column
        vLayout = new QVBoxLayout;
        vLayout->addWidget(mRightToolBar);
        layout->addLayout(vLayout);

        mThumbnailBar->setFixedHeight(GwenviewConfig::fullScreenBarHeight());
        mAutoHideContainer->setFixedHeight(GwenviewConfig::fullScreenBarHeight());

        // Shadows
        mToolBarShadow->reset();
        mToolBarShadow->setShadow(ShadowFilter::RightEdge, QColor(0, 0, 0, 64));
        mToolBarShadow->setShadow(ShadowFilter::BottomEdge, QColor(255, 255, 255, 8));

        mInformationLabelShadow->reset();
        mInformationLabelShadow->setShadow(ShadowFilter::LeftEdge, QColor(0, 0, 0, 64));
        mInformationLabelShadow->setShadow(ShadowFilter::TopEdge, QColor(0, 0, 0, 64));
        mInformationLabelShadow->setShadow(ShadowFilter::RightEdge, QColor(0, 0, 0, 128));
        mInformationLabelShadow->setShadow(ShadowFilter::BottomEdge, QColor(255, 255, 255, 8));

        mRightToolBarShadow->reset();
        mRightToolBarShadow->setShadow(ShadowFilter::LeftEdge, QColor(0, 0, 0, 64));
        mRightToolBarShadow->setShadow(ShadowFilter::BottomEdge, QColor(255, 255, 255, 8));
    } else {
        mRightToolBar->setDirection(QBoxLayout::RightToLeft);

        QHBoxLayout* layout = new QHBoxLayout(mContent);
        layout->setMargin(0);
        layout->setSpacing(0);
        layout->addWidget(mToolBar);
        layout->addWidget(mInformationLabel);
        layout->addWidget(mRightToolBar);

        mAutoHideContainer->setFixedHeight(mContent->sizeHint().height());

        // Shadows
        mToolBarShadow->reset();

        mInformationLabelShadow->reset();
        mInformationLabelShadow->setShadow(ShadowFilter::LeftEdge, QColor(0, 0, 0, 64));
        mInformationLabelShadow->setShadow(ShadowFilter::RightEdge, QColor(0, 0, 0, 32));

        mRightToolBarShadow->reset();
        mRightToolBarShadow->setShadow(ShadowFilter::LeftEdge, QColor(255, 255, 255, 8));
    }
}


void FullScreenContent::updateContainerAppearance()
{
    if (!mFullScreenMode || !mViewPageVisible) {
        mAutoHideContainer->setActivated(false);
        return;
    }

    mThumbnailBar->setVisible(GwenviewConfig::showFullScreenThumbnails());
    mAutoHideContainer->adjustSize();
    mAutoHideContainer->setActivated(true);
}


void FullScreenContent::createOptionsAction()
{
    // We do not use a KActionMenu because:
    //
    // - It causes the button to show a small down arrow on its right,
    // which makes it wider
    //
    // - We can't control where the menu shows: in no-thumbnail-mode, the
    // menu should not be aligned to the right edge of the screen because
    // if the mode is changed to thumbnail-mode, we want the option button
    // to remain visible
    mOptionsAction = new QAction(this);
    mOptionsAction->setPriority(QAction::LowPriority);
    mOptionsAction->setIcon(QIcon::fromTheme("configure"));
    mOptionsAction->setToolTip(i18nc("@info:tooltip", "Configure full screen mode"));
    QObject::connect(mOptionsAction, SIGNAL(triggered()), this, SLOT(showOptionsMenu()));
}


void FullScreenContent::updateSlideShowIntervalLabel()
{
    Q_ASSERT(mConfigWidget);
    int value = mConfigWidget->mSlideShowIntervalSlider->value();
    QString text = formatSlideShowIntervalText(value);
    mConfigWidget->mSlideShowIntervalLabel->setText(text);
}

void FullScreenContent::setFullScreenBarHeight(int value)
{
    mThumbnailBar->setFixedHeight(value);
    mAutoHideContainer->setFixedHeight(value);
    GwenviewConfig::setFullScreenBarHeight(value);
}

void FullScreenContent::showOptionsMenu()
{
    Q_ASSERT(!mConfigWidget);

    mConfigWidget = new FullScreenConfigWidget;
    FullScreenConfigWidget* widget = mConfigWidget;

    // Put widget in a menu
    QMenu menu;
    QWidgetAction* action = new QWidgetAction(&menu);
    action->setDefaultWidget(widget);
    menu.addAction(action);

    // Slideshow checkboxes
    widget->mSlideShowLoopCheckBox->setChecked(mSlideShow->loopAction()->isChecked());
    connect(widget->mSlideShowLoopCheckBox, SIGNAL(toggled(bool)),
            mSlideShow->loopAction(), SLOT(trigger()));

    widget->mSlideShowRandomCheckBox->setChecked(mSlideShow->randomAction()->isChecked());
    connect(widget->mSlideShowRandomCheckBox, SIGNAL(toggled(bool)),
            mSlideShow->randomAction(), SLOT(trigger()));

    // Interval slider
    widget->mSlideShowIntervalSlider->setValue(int(GwenviewConfig::interval()));
    connect(widget->mSlideShowIntervalSlider, SIGNAL(valueChanged(int)),
            mSlideShow, SLOT(setInterval(int)));
    connect(widget->mSlideShowIntervalSlider, SIGNAL(valueChanged(int)),
            SLOT(updateSlideShowIntervalLabel()));

    // Interval label
    QString text = formatSlideShowIntervalText(88);
    int width = widget->mSlideShowIntervalLabel->fontMetrics().width(text);
    widget->mSlideShowIntervalLabel->setFixedWidth(width);
    updateSlideShowIntervalLabel();

    // Image information
    connect(widget->mConfigureDisplayedInformationButton, SIGNAL(clicked()),
            SLOT(showImageMetaInfoDialog()));

    // Thumbnails
    widget->mThumbnailGroupBox->setVisible(mViewPageVisible);
    if (mViewPageVisible) {
        widget->mShowThumbnailsCheckBox->setChecked(GwenviewConfig::showFullScreenThumbnails());
        widget->mHeightSlider->setMinimum(mRightToolBar->sizeHint().height());
        widget->mHeightSlider->setValue(mThumbnailBar->height());
        connect(widget->mShowThumbnailsCheckBox, SIGNAL(toggled(bool)),
                SLOT(slotShowThumbnailsToggled(bool)));
        connect(widget->mHeightSlider, SIGNAL(valueChanged(int)),
                SLOT(setFullScreenBarHeight(int)));
    }

    // Show menu below its button
    QPoint pos;
    QWidget* button = mOptionsAction->associatedWidgets().first();
    Q_ASSERT(button);
    qWarning() << button << button->geometry();
    if (QApplication::isRightToLeft()) {
        pos = button->mapToGlobal(button->rect().bottomLeft());
    } else {
        pos = button->mapToGlobal(button->rect().bottomRight());
        pos.rx() -= menu.sizeHint().width();
    }
    qWarning() << pos;
    menu.exec(pos);
}

void FullScreenContent::setFullScreenMode(bool fullScreenMode)
{
    mFullScreenMode = fullScreenMode;
    updateContainerAppearance();
}

void FullScreenContent::setDistractionFreeMode(bool distractionFreeMode)
{
    mAutoHideContainer->setAutoHidingEnabled(!distractionFreeMode);
}

void FullScreenContent::slotImageMetaInfoDialogClosed()
{
    mAutoHideContainer->setAutoHidingEnabled(true);
}

void FullScreenContent::slotShowThumbnailsToggled(bool value)
{
    GwenviewConfig::setShowFullScreenThumbnails(value);
    GwenviewConfig::self()->save();
    mThumbnailBar->setVisible(value);
    updateLayout();
    mContent->adjustSize();
    mAutoHideContainer->adjustSize();
}

void FullScreenContent::slotViewModeActionToggled(bool value)
{
    mViewPageVisible = value;
    updateContainerAppearance();
}

} // namespace

#include "animatedlistwidget.h"
#include "animationutils.h"
#include <QPainter>
#include <QPaintEvent>

AnimatedListWidget::AnimatedListWidget(QWidget *parent)
    : QListWidget(parent)
    , m_slideTimer(new QTimer(this))
{
    m_slideTimer->setSingleShot(true);
    connect(m_slideTimer, &QTimer::timeout, this, &AnimatedListWidget::slideInNext);
}

void AnimatedListWidget::addAnimatedItem(const QString &text) {
    if (!AnimationUtils::animationsEnabled()) {
        addItem(text);
        return;
    }

    m_pendingItems.append(text);

    if (!m_slideTimer->isActive()) {
        slideInNext();
    }
}

void AnimatedListWidget::addAnimatedItems(const QStringList &items) {
    m_pendingItems.append(items);

    if (!AnimationUtils::animationsEnabled()) {
        for (const QString &item : items) {
            addItem(item);
        }
        m_pendingItems.clear();
        return;
    }

    if (!m_slideTimer->isActive()) {
        slideInNext();
    }
}

void AnimatedListWidget::setSlideDuration(int msecs) {
    m_slideDuration = msecs;
}

qreal AnimatedListWidget::slideProgress() const {
    return m_slideProgress;
}

void AnimatedListWidget::setSlideProgress(qreal progress) {
    if (qFuzzyCompare(m_slideProgress, progress))
        return;

    m_slideProgress = progress;
    viewport()->update();
}

void AnimatedListWidget::paintEvent(QPaintEvent *event) {
    // If no animation is in progress, use default painting
    if (m_animatingRow < 0 || qFuzzyCompare(m_slideProgress, 1.0)) {
        QListWidget::paintEvent(event);
        return;
    }

    // Paint all items normally except the animating one
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    // First, let the base class paint everything
    QListWidget::paintEvent(event);

    // Now paint the animating item with slide effect
    QListWidgetItem *animatingItem = item(m_animatingRow);
    if (!animatingItem) {
        return;
    }

    QRect itemRect = visualItemRect(animatingItem);
    if (!itemRect.isValid()) {
        return;
    }

    // Calculate slide offset (slide in from right)
    int slideOffset = static_cast<int>((1.0 - m_slideProgress) * itemRect.width());

    // Set up clipping to prevent drawing outside item bounds
    painter.setClipRect(itemRect);

    // Save the painter state
    painter.save();

    // Translate the painter for slide effect
    painter.translate(slideOffset, 0);

    // Get the item's visual rect and draw it with translation
    QStyleOptionViewItem opt;
    opt.initFrom(viewport());
    opt.rect = itemRect.translated(-slideOffset, 0);
    opt.text = animatingItem->text();
    opt.state |= QStyle::State_Enabled;

    // Draw the item background
    if (selectionModel()->isSelected(indexFromItem(animatingItem))) {
        opt.state |= QStyle::State_Selected;
    }

    style()->drawControl(QStyle::CE_ItemViewItem, &opt, &painter, this);

    // Restore painter state
    painter.restore();
}

void AnimatedListWidget::slideInNext() {
    if (m_pendingItems.isEmpty()) return;

    QString text = m_pendingItems.takeFirst();

    // Create new item
    QListWidgetItem *item = new QListWidgetItem(text);

    // Get insertion position
    int row = count();
    insertItem(row, item);

    // Track the animating row
    m_animatingRow = row;
    m_slideProgress = 0.0;

    if (AnimationUtils::animationsEnabled()) {
        // Create slide-in animation using the slideProgress property
        QPropertyAnimation *slideIn = new QPropertyAnimation(this, "slideProgress");
        slideIn->setDuration(m_slideDuration);
        slideIn->setStartValue(0.0);
        slideIn->setEndValue(1.0);
        slideIn->setEasingCurve(QEasingCurve::OutCubic);

        connect(slideIn, &QPropertyAnimation::finished, this, [this]() {
            m_animatingRow = -1;
            m_slideProgress = 1.0;
            viewport()->update();
        });

        slideIn->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        m_animatingRow = -1;
        m_slideProgress = 1.0;
    }

    // Process next item with delay
    if (!m_pendingItems.isEmpty()) {
        m_slideTimer->start(m_slideDuration + 30);
    }
}
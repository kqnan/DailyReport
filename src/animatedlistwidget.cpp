#include "animatedlistwidget.h"
#include "animationutils.h"

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

void AnimatedListWidget::slideInNext() {
    if (m_pendingItems.isEmpty()) return;

    QString text = m_pendingItems.takeFirst();

    // 创建新项，初始位置在右侧
    QListWidgetItem *item = new QListWidgetItem(text);
    item->setData(Qt::UserRole, QVariant()); // 用于动画状态

    // 获取插入位置
    int row = count();
    insertItem(row, item);

    // 创建滑入动画
    QWidget *itemWidget = itemWidget(item);
    if (!itemWidget) {
        // 使用项的几何动画
        QPropertyAnimation *slideIn = new QPropertyAnimation(this, QByteArray());
        slideIn->setDuration(m_slideDuration);
        slideIn->setStartValue(0.0);
        slideIn->setEndValue(1.0);
        slideIn->setEasingCurve(QEasingCurve::OutCubic);

        // 触发重绘
        connect(slideIn, &QPropertyAnimation::finished, this, [this]() {
            viewport()->update();
        });

        slideIn->start(QAbstractAnimation::DeleteWhenStopped);
    }

    // 处理下一项
    if (!m_pendingItems.isEmpty()) {
        m_slideTimer->start(m_slideDuration / 2);
    }
}
#include "animatedlabel.h"
#include <QPainter>
#include "animationutils.h"

AnimatedLabel::AnimatedLabel(QWidget *parent)
    : QLabel(parent)
{
}

void AnimatedLabel::setAnimatedText(const QString &text) {
    if (text == this->text()) return;

    if (!AnimationUtils::animationsEnabled()) {
        QLabel::setText(text);
        return;
    }

    m_oldText = this->text();
    m_newText = text;

    if (m_animation) {
        m_animation->stop();
        delete m_animation;
    }

    m_slideOffset = 0;
    m_animation = new QPropertyAnimation(this, "slideOffset");
    m_animation->setDuration(m_animationDuration);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
        QLabel::setText(m_newText);
        m_oldText.clear();
        update();
    });

    m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void AnimatedLabel::setAnimationDuration(int msecs) {
    m_animationDuration = msecs;
}

void AnimatedLabel::setTextImmediate(const QString &text) {
    if (m_animation) {
        m_animation->stop();
        delete m_animation;
        m_animation = nullptr;
    }
    QLabel::setText(text);
    m_oldText.clear();
    m_newText = text;
    m_slideOffset = 0;
    update();
}

qreal AnimatedLabel::slideOffset() const {
    return m_slideOffset;
}

void AnimatedLabel::setSlideOffset(qreal offset) {
    if (qFuzzyCompare(m_slideOffset, offset)) return;
    m_slideOffset = offset;
    update();
}

void AnimatedLabel::paintEvent(QPaintEvent *event) {
    if (m_oldText.isEmpty() || m_slideOffset <= 0 || m_slideOffset >= 1) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    int slideDistance = height();

    // Draw old text sliding up
    qreal oldOpacity = 1.0 - m_slideOffset;
    painter.setOpacity(oldOpacity);
    painter.translate(0, -slideDistance * m_slideOffset);
    painter.drawText(rect(), alignment(), m_oldText);

    // Reset and draw new text sliding in
    painter.resetTransform();
    qreal newOpacity = m_slideOffset;
    painter.setOpacity(newOpacity);
    painter.translate(0, slideDistance * (1.0 - m_slideOffset));
    painter.drawText(rect(), alignment(), m_newText);
}
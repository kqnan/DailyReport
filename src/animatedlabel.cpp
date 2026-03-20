#include "animatedlabel.h"
#include "animationutils.h"
#include <QPainter>
#include <QEasingCurve>

AnimatedLabel::AnimatedLabel(QWidget *parent)
    : QLabel(parent)
    , m_animation(new QPropertyAnimation(this, "slideOffset", this))
{
    m_animation->setDuration(m_duration);
    m_animation->setEasingCurve(QEasingCurve::OutQuad);
    connect(m_animation, &QPropertyAnimation::finished, this, &AnimatedLabel::onAnimationFinished);
}

void AnimatedLabel::setAnimatedText(const QString &text) {
    if (text == m_newText || text == this->text()) {
        return;
    }

    if (!AnimationUtils::animationsEnabled()) {
        setText(text);
        m_newText = text;
        m_oldText = text;
        return;
    }

    if (m_isAnimating) {
        m_animation->stop();
        m_oldText = m_newText;
    } else {
        m_oldText = this->text();
    }

    m_newText = text;
    m_isAnimating = true;
    m_slideOffset = 0;

    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start();

    update();
}

void AnimatedLabel::setAnimationDuration(int msecs) {
    m_duration = msecs;
    m_animation->setDuration(msecs);
}

void AnimatedLabel::setTextImmediate(const QString &text) {
    m_animation->stop();
    m_isAnimating = false;
    m_oldText = text;
    m_newText = text;
    setText(text);
}

void AnimatedLabel::setSlideOffset(qreal offset) {
    m_slideOffset = offset;
    update();
}

void AnimatedLabel::paintEvent(QPaintEvent *event) {
    if (!m_isAnimating) {
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font = this->font();
    painter.setFont(font);

    QRect rect = this->rect();
    int yOffset = rect.height() * m_slideOffset;

    // Draw old text sliding up
    painter.setPen(QPen(palette().color(foregroundRole())));
    painter.drawText(rect.translated(0, -yOffset), Qt::AlignCenter, m_oldText);

    // Draw new text sliding in from below
    painter.setPen(QPen(palette().color(foregroundRole())));
    painter.drawText(rect.translated(0, rect.height() - yOffset), Qt::AlignCenter, m_newText);
}

void AnimatedLabel::onAnimationFinished() {
    m_isAnimating = false;
    setText(m_newText);
    m_oldText = m_newText;
    update();
}

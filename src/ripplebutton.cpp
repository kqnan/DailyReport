#include "ripplebutton.h"
#include "animationutils.h"
#include <QPainter>
#include <QMouseEvent>
#include <QEasingCurve>

RippleButton::RippleButton(QWidget *parent)
    : QPushButton(parent)
{
}

RippleButton::RippleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
}

RippleButton::~RippleButton() {
    for (auto &ripple : m_ripples) {
        if (ripple.animation) {
            ripple.animation->stop();
            delete ripple.animation;
        }
    }
}

void RippleButton::setRippleColor(const QColor &color) {
    m_rippleColor = color;
}

void RippleButton::setRippleDuration(int msecs) {
    m_rippleDuration = msecs;
}

void RippleButton::setRippleEnabled(bool enabled) {
    m_rippleEnabled = enabled;
}

void RippleButton::mousePressEvent(QMouseEvent *event) {
    if (!m_rippleEnabled || !AnimationUtils::animationsEnabled()) {
        QPushButton::mousePressEvent(event);
        return;
    }

    Ripple ripple;
    ripple.center = event->pos();
    ripple.radius = 0;
    ripple.maxRadius = qMax(width(), height()) * 1.5;
    ripple.opacity = 1.0;
    ripple.animation = new QPropertyAnimation(this);
    ripple.animation->setDuration(m_rippleDuration);
    ripple.animation->setStartValue(0.0);
    ripple.animation->setEndValue(ripple.maxRadius);
    ripple.animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(ripple.animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value) {
        // Find the ripple that owns this animation
        for (auto &r : m_ripples) {
            if (r.animation == qobject_cast<QPropertyAnimation*>(sender())) {
                r.radius = value.toReal();
                r.opacity = 1.0 - (r.radius / r.maxRadius);
                break;
            }
        }
        update();
    });

    connect(ripple.animation, &QPropertyAnimation::finished, this, &RippleButton::removeRipple);

    m_ripples.append(ripple);
    ripple.animation->start();

    QPushButton::mousePressEvent(event);
}

void RippleButton::paintEvent(QPaintEvent *event) {
    QPushButton::paintEvent(event);

    if (m_ripples.isEmpty()) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    for (const auto &ripple : m_ripples) {
        if (ripple.radius <= 0) continue;

        QColor color = m_rippleColor;
        color.setAlphaF(color.alphaF() * ripple.opacity);

        painter.setBrush(color);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(ripple.center, ripple.radius, ripple.radius);
    }
}

void RippleButton::updateRipple() {
    update();
}

void RippleButton::removeRipple() {
    auto *animation = qobject_cast<QPropertyAnimation*>(sender());
    if (!animation) return;

    for (int i = 0; i < m_ripples.size(); ++i) {
        if (m_ripples[i].animation == animation) {
            delete m_ripples[i].animation;
            m_ripples.removeAt(i);
            break;
        }
    }
    update();
}

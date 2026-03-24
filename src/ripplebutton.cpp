#include "ripplebutton.h"
#include <QPainter>
#include <QMouseEvent>
#include "animationutils.h"

RippleButton::RippleButton(QWidget *parent)
    : QPushButton(parent)
    , m_rippleColor(255, 255, 255, 80)
    , m_rippleDuration(400)
{
}

RippleButton::RippleButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
    , m_rippleColor(255, 255, 255, 80)
    , m_rippleDuration(400)
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

void RippleButton::mousePressEvent(QMouseEvent *event) {
    QPushButton::mousePressEvent(event);

    if (!AnimationUtils::animationsEnabled()) return;

    Ripple ripple;
    ripple.center = event->pos();
    ripple.radius = 0;
    ripple.maxRadius = qMax(width(), height()) * 1.5;
    ripple.opacity = 0.4;

    ripple.animation = new QPropertyAnimation(this, QByteArray());
    ripple.animation->setDuration(m_rippleDuration);
    ripple.animation->setStartValue(0.0);
    ripple.animation->setEndValue(1.0);
    ripple.animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(ripple.animation, &QPropertyAnimation::valueChanged, this, [this](const QVariant &value) {
        qreal progress = value.toReal();
        for (int i = 0; i < m_ripples.size(); ++i) {
            if (m_ripples[i].animation == sender()) {
                m_ripples[i].radius = m_ripples[i].maxRadius * progress;
                m_ripples[i].opacity = 0.4 * (1.0 - progress);
                break;
            }
        }
        update();
    });
    connect(ripple.animation, &QPropertyAnimation::finished, this, &RippleButton::removeRipple);

    m_ripples.append(ripple);
    ripple.animation->start();
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
        painter.drawEllipse(ripple.center, static_cast<int>(ripple.radius), static_cast<int>(ripple.radius));
    }
}

void RippleButton::removeRipple() {
    auto *animation = qobject_cast<QPropertyAnimation*>(sender());
    if (!animation) return;

    for (int i = 0; i < m_ripples.size(); ++i) {
        if (m_ripples[i].animation == animation) {
            m_ripples.removeAt(i);
            break;
        }
    }
    animation->deleteLater();
    update();
}
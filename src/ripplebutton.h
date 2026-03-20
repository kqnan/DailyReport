#pragma once

#include <QPushButton>
#include <QColor>
#include <QList>
#include <QPropertyAnimation>

struct Ripple {
    QPoint center;
    qreal radius;
    qreal maxRadius;
    qreal opacity;
    QPropertyAnimation *animation;
};

class RippleButton : public QPushButton {
    Q_OBJECT
    Q_PROPERTY(QColor rippleColor READ rippleColor WRITE setRippleColor)
    Q_PROPERTY(int rippleDuration READ rippleDuration WRITE setRippleDuration)

public:
    explicit RippleButton(QWidget *parent = nullptr);
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);
    ~RippleButton();

    void setRippleColor(const QColor &color);
    QColor rippleColor() const { return m_rippleColor; }

    void setRippleDuration(int msecs);
    int rippleDuration() const { return m_rippleDuration; }

    void setRippleEnabled(bool enabled);
    bool isRippleEnabled() const { return m_rippleEnabled; }

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateRipple();
    void removeRipple();

private:
    QColor m_rippleColor = QColor(255, 255, 255, 100);
    int m_rippleDuration = 400;
    bool m_rippleEnabled = true;
    QList<Ripple> m_ripples;
};

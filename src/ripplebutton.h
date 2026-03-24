#pragma once
#include <QPushButton>
#include <QPropertyAnimation>
#include <QPoint>
#include <QList>

struct Ripple {
    QPoint center;
    qreal radius;
    qreal maxRadius;
    qreal opacity;
    QPropertyAnimation *animation;
};

class RippleButton : public QPushButton {
    Q_OBJECT

public:
    explicit RippleButton(QWidget *parent = nullptr);
    explicit RippleButton(const QString &text, QWidget *parent = nullptr);
    ~RippleButton();

    void setRippleColor(const QColor &color);
    void setRippleDuration(int msecs);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void removeRipple();

private:
    QColor m_rippleColor;
    int m_rippleDuration;
    QList<Ripple> m_ripples;
};
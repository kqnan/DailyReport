#pragma once
#include <QLabel>
#include <QPropertyAnimation>

class AnimatedLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit AnimatedLabel(QWidget *parent = nullptr);

    void setAnimatedText(const QString &text);
    void setAnimationDuration(int msecs);
    void setTextImmediate(const QString &text);

    qreal slideOffset() const;
    void setSlideOffset(qreal offset);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_oldText;
    QString m_newText;
    qreal m_slideOffset = 0;
    int m_animationDuration = 200;
    QPropertyAnimation *m_animation = nullptr;
};
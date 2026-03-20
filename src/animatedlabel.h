#pragma once

#include <QLabel>
#include <QPropertyAnimation>
#include <QString>

class AnimatedLabel : public QLabel {
    Q_OBJECT
    Q_PROPERTY(qreal slideOffset READ slideOffset WRITE setSlideOffset)

public:
    explicit AnimatedLabel(QWidget *parent = nullptr);

    void setAnimatedText(const QString &text);
    void setAnimationDuration(int msecs);
    void setTextImmediate(const QString &text);

    qreal slideOffset() const { return m_slideOffset; }
    void setSlideOffset(qreal offset);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimationFinished();

private:
    int m_duration = 200;
    QString m_oldText;
    QString m_newText;
    qreal m_slideOffset = 0;
    bool m_isAnimating = false;
    QPropertyAnimation *m_animation;
};

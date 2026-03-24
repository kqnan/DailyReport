#pragma once
#include <QListWidget>
#include <QPropertyAnimation>
#include <QTimer>

class AnimatedListWidget : public QListWidget {
    Q_OBJECT
    Q_PROPERTY(qreal slideProgress READ slideProgress WRITE setSlideProgress)

public:
    explicit AnimatedListWidget(QWidget *parent = nullptr);

    void addAnimatedItem(const QString &text);
    void addAnimatedItems(const QStringList &items);
    void setSlideDuration(int msecs);

    qreal slideProgress() const;
    void setSlideProgress(qreal progress);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void slideInNext();

private:
    int m_slideDuration = 150;
    QStringList m_pendingItems;
    QTimer *m_slideTimer;
    qreal m_slideProgress = 1.0;
    int m_animatingRow = -1;
};
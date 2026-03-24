#pragma once
#include <QListWidget>
#include <QPropertyAnimation>
#include <QTimer>

class AnimatedListWidget : public QListWidget {
    Q_OBJECT

public:
    explicit AnimatedListWidget(QWidget *parent = nullptr);

    void addAnimatedItem(const QString &text);
    void addAnimatedItems(const QStringList &items);
    void setSlideDuration(int msecs);

private slots:
    void slideInNext();

private:
    int m_slideDuration = 150;
    QStringList m_pendingItems;
    QTimer *m_slideTimer;
};
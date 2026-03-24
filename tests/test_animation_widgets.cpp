#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "../src/animationutils.h"
#include "../src/ripplebutton.h"
#include "../src/animatedlabel.h"
#include "../src/animatedlistwidget.h"

class TestAnimationWidgets : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        AnimationUtils::setAnimationsEnabled(true);
    }

    void testAnimationUtilsDefaultEnabled() {
        AnimationUtils::setAnimationsEnabled(true);
        QVERIFY(AnimationUtils::animationsEnabled());
    }

    void testAnimationUtilsCanBeDisabled() {
        AnimationUtils::setAnimationsEnabled(false);
        QVERIFY(!AnimationUtils::animationsEnabled());
        AnimationUtils::setAnimationsEnabled(true);
    }

    void testRippleButtonCreation() {
        RippleButton button("Test");
        QCOMPARE(button.text(), QString("Test"));
    }

    void testRippleButtonClick() {
        RippleButton button("Test");
        QSignalSpy spy(&button, &RippleButton::clicked);
        button.click();
        QCOMPARE(spy.count(), 1);
    }

    void testRippleButtonSetRippleColor() {
        RippleButton button("Test");
        button.setRippleColor(Qt::red);
        // 验证设置不会崩溃
        QVERIFY(true);
    }

    void testRippleButtonSetRippleDuration() {
        RippleButton button("Test");
        button.setRippleDuration(500);
        QVERIFY(true);
    }

    void testAnimatedLabelCreation() {
        AnimatedLabel label;
        QVERIFY(label.text().isEmpty());
    }

    void testAnimatedLabelSetTextImmediate() {
        AnimatedLabel label;
        label.setTextImmediate("Test Text");
        QCOMPARE(label.text(), QString("Test Text"));
    }

    void testAnimatedLabelSetAnimatedText() {
        AnimatedLabel label;
        label.setTextImmediate("Initial");
        QCOMPARE(label.text(), QString("Initial"));
        label.setAnimatedText("Second");
        // 动画期间文本仍是旧值
        QCOMPARE(label.text(), QString("Initial"));
    }

    void testAnimatedListWidgetCreation() {
        AnimatedListWidget list;
        QCOMPARE(list.count(), 0);
    }

    void testAnimatedListWidgetAddItem() {
        AnimatedListWidget list;
        list.addAnimatedItem("Test Item");
        QCOMPARE(list.count(), 1);
        QCOMPARE(list.item(0)->text(), QString("Test Item"));
    }

    void testWidgetsWithAnimationsDisabled() {
        AnimationUtils::setAnimationsEnabled(false);

        AnimatedLabel label;
        label.setAnimatedText("Test");
        QCOMPARE(label.text(), QString("Test"));

        RippleButton button("Click");
        QSignalSpy spy(&button, &RippleButton::clicked);
        button.click();
        QCOMPARE(spy.count(), 1);

        AnimatedListWidget list;
        list.addAnimatedItem("Item");
        QCOMPARE(list.count(), 1);

        AnimationUtils::setAnimationsEnabled(true);
    }
};

QTEST_MAIN(TestAnimationWidgets)
#include "test_animation_widgets.moc"
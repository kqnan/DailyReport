#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QThread>
#include "../src/animationutils.h"
#include "../src/ripplebutton.h"
#include "../src/animatedlabel.h"

class TestAnimationWidgets : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        // Ensure animations are enabled at start of tests
        AnimationUtils::setAnimationsEnabled(true);
    }

    // AnimationUtils tests
    void testAnimationUtilsDefaultEnabled() {
        AnimationUtils::setAnimationsEnabled(true);
        QVERIFY(AnimationUtils::animationsEnabled());
    }

    void testAnimationUtilsCanBeDisabled() {
        AnimationUtils::setAnimationsEnabled(false);
        QVERIFY(!AnimationUtils::animationsEnabled());
        // Restore for other tests
        AnimationUtils::setAnimationsEnabled(true);
    }

    void testAnimationUtilsToggle() {
        bool initial = AnimationUtils::animationsEnabled();
        AnimationUtils::setAnimationsEnabled(!initial);
        QCOMPARE(AnimationUtils::animationsEnabled(), !initial);
        AnimationUtils::setAnimationsEnabled(initial);
    }

    // RippleButton tests
    void testRippleButtonCreation() {
        RippleButton button("Test");
        QCOMPARE(button.text(), QString("Test"));
        QVERIFY(button.isEnabled());
    }

    void testRippleButtonDefaultConstruction() {
        RippleButton button;
        QVERIFY(button.text().isEmpty());
    }

    void testRippleButtonSetRippleColor() {
        RippleButton button("Test");
        QColor customColor(255, 0, 0, 50);
        button.setRippleColor(customColor);
        // Color is stored internally, no direct getter
        // Test passes if no crash
    }

    void testRippleButtonSetRippleDuration() {
        RippleButton button("Test");
        button.setRippleDuration(200);
        // Duration is stored internally, no direct getter
        // Test passes if no crash
    }

    void testRippleButtonClick() {
        RippleButton button("Test");
        QSignalSpy spy(&button, &RippleButton::clicked);
        button.click();
        QCOMPARE(spy.count(), 1);
    }

    // AnimatedLabel tests
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
        // Animation starts, old text remains until animation completes
        QCOMPARE(label.text(), QString("Initial"));
    }

    void testAnimatedLabelDuplicateTextNoAnimation() {
        AnimatedLabel label;
        label.setTextImmediate("Same");
        label.setAnimatedText("Same");
        // Should not start animation for same text
        QCOMPARE(label.text(), QString("Same"));
    }

    void testAnimatedLabelSetAnimationDuration() {
        AnimatedLabel label;
        label.setAnimationDuration(500);
        // Duration is stored internally
        // Test passes if no crash
    }

    // Integration tests
    void testWidgetsWithAnimationsDisabled() {
        AnimationUtils::setAnimationsEnabled(false);

        AnimatedLabel label;
        label.setAnimatedText("Test");
        // Should set text immediately when animations disabled
        QCOMPARE(label.text(), QString("Test"));

        RippleButton button("Click");
        QVERIFY(button.isEnabled());
        // Ripple effect should be disabled, but button still works
        QSignalSpy spy(&button, &RippleButton::clicked);
        button.click();
        QCOMPARE(spy.count(), 1);

        AnimationUtils::setAnimationsEnabled(true);
    }

    void testMultipleAnimatedLabelUpdates() {
        AnimatedLabel label;
        label.setTextImmediate("0");

        QStringList texts = {"1", "2", "3", "4", "5"};
        for (const QString& text : texts) {
            label.setAnimatedText(text);
        }

        // Final text should eventually be set after animation completes
        // Note: In real scenario, we'd wait for animation, but here we verify no crash
    }
};

QTEST_MAIN(TestAnimationWidgets)
#include "test_animation_widgets.moc"

#include "animationutils.h"

bool AnimationUtils::s_enabled = true;

bool AnimationUtils::animationsEnabled() {
    return s_enabled;
}

void AnimationUtils::setAnimationsEnabled(bool enabled) {
    s_enabled = enabled;
}
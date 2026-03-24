#pragma once

class AnimationUtils {
public:
    static bool animationsEnabled();
    static void setAnimationsEnabled(bool enabled);

private:
    static bool s_enabled;
};
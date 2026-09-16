/*
Echo
Copyright (C) 2025 Voidscape Development
Distributed under the AGPLv3. See the LICENSE file for details.
*/

#pragma once

#include <juce_graphics/juce_graphics.h>

namespace echo::ui
{

/*
    Design tokens for Echo's windows.

    The palette is derived from the two colours OBS gives us (window background and text)
    so the windows belong to whatever OBS theme the user runs, plus one accent colour of
    our own for active, mapped and selected states. Everything else - surfaces, borders,
    dimmed text - is lifted or sunk from the background, which keeps the hierarchy intact
    on dark and light themes alike.
*/

namespace metrics
{
inline constexpr float controlCorner = 5.0f;
inline constexpr float panelCorner = 9.0f;

inline constexpr int windowMargin = 16;
inline constexpr int gutter = 12;
inline constexpr int panelPadding = 12;
inline constexpr int panelHeaderHeight = 34;

inline constexpr int controlHeight = 30;
inline constexpr int footerHeight = 40;
inline constexpr int titleBarHeight = 38;

inline constexpr float bodyFontSize = 14.0f;
inline constexpr float smallFontSize = 12.0f;
inline constexpr float headerFontSize = 13.0f;
} // namespace metrics

/** Moves a colour away from itself: brighter when it is dark, darker when it is light. */
inline juce::Colour lift(juce::Colour base, float amount)
{
    return base.getPerceivedBrightness() < 0.5f ? base.brighter(amount) : base.darker(amount * 0.7f);
}

/** Moves a colour towards the background: the inverse of lift(). */
inline juce::Colour sink(juce::Colour base, float amount)
{
    return base.getPerceivedBrightness() < 0.5f ? base.darker(amount) : base.brighter(amount * 0.7f);
}

struct Palette
{
    juce::Colour windowBg;     // the window itself, straight from OBS
    juce::Colour panel;        // raised card surface
    juce::Colour panelHeader;  // card header strip
    juce::Colour control;      // buttons, combo boxes, inputs
    juce::Colour controlHover; // hover state for the above
    juce::Colour rowAlt;       // alternating list/table rows
    juce::Colour border;       // hairlines between surfaces
    juce::Colour borderStrong; // outlines that need to read as an edge
    juce::Colour text;         // primary text, straight from OBS
    juce::Colour textDim;      // secondary text, labels, disabled
    juce::Colour accent;       // active / mapped / selected
    juce::Colour accentHover;
    juce::Colour accentSoft; // translucent accent for fills
    juce::Colour onAccent;   // text drawn on top of the accent

    static Palette fromObsColours(juce::Colour background, juce::Colour foreground)
    {
        Palette p;

        // A fully black or white OBS background leaves no room to build a hierarchy from,
        // so nudge it towards the middle before deriving anything.
        const auto brightness = background.getPerceivedBrightness();
        if (brightness < 0.04f)
            background = background.brighter(0.06f);
        else if (brightness > 0.96f)
            background = background.darker(0.04f);

        p.windowBg = background;
        p.panel = lift(background, 0.09f);
        p.panelHeader = lift(background, 0.15f);
        p.control = lift(background, 0.20f);
        p.controlHover = lift(background, 0.32f);
        p.rowAlt = lift(background, 0.05f);
        p.border = lift(background, 0.28f).withAlpha(0.55f);
        p.borderStrong = lift(background, 0.45f).withAlpha(0.8f);

        p.text = foreground;
        p.textDim = foreground.withAlpha(0.62f);

        // A single fixed hue, brightness-corrected so it keeps contrast on light themes.
        const juce::Colour base(0xff3ea6ff);
        p.accent = background.getPerceivedBrightness() > 0.5f ? base.darker(0.32f) : base;
        p.accentHover = p.accent.brighter(0.18f);
        p.accentSoft = p.accent.withAlpha(0.22f);
        p.onAccent = p.accent.contrasting(0.95f);

        return p;
    }
};

inline juce::Font bodyFont(float size = metrics::bodyFontSize, bool bold = false)
{
    return juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
}

} // namespace echo::ui

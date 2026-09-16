/*
Echo
Copyright (C) 2025 Voidscape Development
Distributed under the AGPLv3. See the LICENSE file for details.
*/

#pragma once

#include "EchoTheme.h"

#include <juce_graphics/juce_graphics.h>

namespace echo::ui
{

/*
    Drawing primitives shared by the plugin's windows and the ui-preview tool, so the
    preview renders exactly what ships rather than an approximation of it.
*/

/** A tick, sized to the given bounds. */
inline void drawCheckMark(juce::Graphics& g, juce::Rectangle<float> bounds, juce::Colour colour, float thickness)
{
    juce::Path tick;
    tick.startNewSubPath(bounds.getX(), bounds.getCentreY() + bounds.getHeight() * 0.02f);
    tick.lineTo(bounds.getCentreX() - bounds.getWidth() * 0.08f, bounds.getBottom());
    tick.lineTo(bounds.getRight(), bounds.getY());

    g.setColour(colour);
    g.strokePath(tick, juce::PathStrokeType(thickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

/** A checkbox. Replaces the "[X]" / "[ ]" text the device trees used to draw. */
inline void drawCheckbox(juce::Graphics& g, juce::Rectangle<float> bounds, bool checked, const Palette& palette)
{
    const auto box = bounds.withSizeKeepingCentre(
        juce::jmin(bounds.getWidth(), bounds.getHeight(), 15.0f),
        juce::jmin(bounds.getWidth(), bounds.getHeight(), 15.0f)
    );
    const float corner = 3.5f;

    if (checked)
    {
        g.setColour(palette.accent);
        g.fillRoundedRectangle(box, corner);
        drawCheckMark(g, box.reduced(box.getWidth() * 0.27f), palette.onAccent, 1.9f);
    }
    else
    {
        g.setColour(palette.control);
        g.fillRoundedRectangle(box, corner);
        g.setColour(palette.borderStrong);
        g.drawRoundedRectangle(box.reduced(0.5f), corner, 1.0f);
    }
}

/** One cell of a routing matrix. Replaces the bold monospace "X". */
inline void drawMatrixCell(juce::Graphics& g, juce::Rectangle<float> bounds, bool mapped, const Palette& palette)
{
    // Hairline grid, so an empty matrix still reads as a clickable surface.
    g.setColour(palette.border.withMultipliedAlpha(0.5f));
    g.drawLine(bounds.getRight() - 0.5f, bounds.getY(), bounds.getRight() - 0.5f, bounds.getBottom(), 1.0f);
    g.drawLine(bounds.getX(), bounds.getBottom() - 0.5f, bounds.getRight(), bounds.getBottom() - 0.5f, 1.0f);

    const auto cell = bounds.withSizeKeepingCentre(
        juce::jmin(bounds.getWidth() - 8.0f, 18.0f),
        juce::jmin(bounds.getHeight() - 6.0f, 18.0f)
    );

    if (mapped)
    {
        g.setColour(palette.accent);
        g.fillRoundedRectangle(cell, 4.0f);
        drawCheckMark(g, cell.reduced(cell.getWidth() * 0.29f), palette.onAccent, 2.0f);
    }
    else
    {
        g.setColour(palette.borderStrong.withMultipliedAlpha(0.26f));
        g.drawRoundedRectangle(cell.reduced(0.5f), 4.0f, 1.0f);
    }
}

/** A raised card. Panels group the trees and matrices instead of floating on the window. */
inline void drawPanel(juce::Graphics& g, juce::Rectangle<int> bounds, const Palette& palette)
{
    const auto r = bounds.toFloat();

    g.setColour(palette.panel);
    g.fillRoundedRectangle(r, metrics::panelCorner);

    g.setColour(palette.border);
    g.drawRoundedRectangle(r.reduced(0.5f), metrics::panelCorner, 1.0f);
}

/**
    A card's header strip: a title, an optional right-aligned hint, and a separator. Returns
    the area left over for the card's content.
*/
inline juce::Rectangle<int> drawPanelHeader(
    juce::Graphics& g,
    juce::Rectangle<int> panelBounds,
    const juce::String& title,
    const juce::String& hint,
    const Palette& palette
)
{
    auto remaining = panelBounds;
    auto header = remaining.removeFromTop(metrics::panelHeaderHeight);

    // Round the top corners with the panel, square off the bottom edge.
    juce::Path headerShape;
    headerShape.addRoundedRectangle(
        header.toFloat().getX(),
        header.toFloat().getY(),
        header.toFloat().getWidth(),
        header.toFloat().getHeight(),
        metrics::panelCorner,
        metrics::panelCorner,
        true,
        true,
        false,
        false
    );

    g.setColour(palette.panelHeader);
    g.fillPath(headerShape);

    g.setColour(palette.border);
    g.drawLine(
        (float)header.getX(),
        (float)header.getBottom() - 0.5f,
        (float)header.getRight(),
        (float)header.getBottom() - 0.5f,
        1.0f
    );

    auto textArea = header.reduced(metrics::panelPadding, 0);

    if (hint.isNotEmpty())
    {
        g.setColour(palette.textDim);
        g.setFont(bodyFont(metrics::smallFontSize));
        const auto hintWidth = juce::jmin(textArea.getWidth() / 2, 160);
        g.drawText(hint, textArea.removeFromRight(hintWidth), juce::Justification::centredRight, true);
    }

    g.setColour(palette.text);
    g.setFont(bodyFont(metrics::headerFontSize, true));
    g.drawText(title.toUpperCase(), textArea, juce::Justification::centredLeft, true);

    return remaining;
}

} // namespace echo::ui

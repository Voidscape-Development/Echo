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

/**
    The Device IO 2 window's geometry, in one place so the window and the ui-preview tool
    lay out identically rather than drifting apart.

    Two rows of cards - device trees above, routing matrices below - over a footer that
    separates the device action on the left from reset/discard/apply on the right.
*/
struct DeviceIo2Layout
{
    juce::Rectangle<int> inputPanel;
    juce::Rectangle<int> outputPanel;
    juce::Rectangle<int> inputMatrixPanel;
    juce::Rectangle<int> outputMatrixPanel;

    juce::Rectangle<int> deviceButton;
    juce::Rectangle<int> resetButton;
    juce::Rectangle<int> discardButton;
    juce::Rectangle<int> applyButton;

    /** The area inside a card, below its header. */
    static juce::Rectangle<int> panelContent(juce::Rectangle<int> panel)
    {
        return panel.withTrimmedTop(metrics::panelHeaderHeight).reduced(1, 1);
    }

    static DeviceIo2Layout compute(juce::Rectangle<int> windowBounds)
    {
        DeviceIo2Layout layout;

        auto bounds = windowBounds.reduced(metrics::windowMargin);

        auto footer = bounds.removeFromBottom(metrics::footerHeight);
        layout.deviceButton = footer.removeFromLeft(110).withSizeKeepingCentre(110, metrics::controlHeight);

        layout.applyButton = footer.removeFromRight(96).withSizeKeepingCentre(96, metrics::controlHeight);
        footer.removeFromRight(8);
        layout.discardButton = footer.removeFromRight(88).withSizeKeepingCentre(88, metrics::controlHeight);
        footer.removeFromRight(4);
        layout.resetButton = footer.removeFromRight(80).withSizeKeepingCentre(80, metrics::controlHeight);

        bounds.removeFromBottom(metrics::gutter);

        auto deviceRow = bounds.removeFromTop((int)((float)bounds.getHeight() * 0.52f));
        layout.inputPanel = deviceRow.removeFromLeft((deviceRow.getWidth() - metrics::gutter) / 2);
        deviceRow.removeFromLeft(metrics::gutter);
        layout.outputPanel = deviceRow;

        bounds.removeFromTop(metrics::gutter);
        layout.inputMatrixPanel = bounds.removeFromLeft((bounds.getWidth() - metrics::gutter) / 2);
        bounds.removeFromLeft(metrics::gutter);
        layout.outputMatrixPanel = bounds;

        return layout;
    }
};

} // namespace echo::ui

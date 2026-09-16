/*
Echo
Copyright (C) 2025 Voidscape Development
Distributed under the AGPLv3. See the LICENSE file for details.
*/

#pragma once

#include "EchoTheme.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace echo::ui
{

/*
    Echo's look and feel.

    Derives from LookAndFeel_V4 and replaces the parts of it that read as dated: square
    outlined controls, heavy scrollbars, the V4 tick box, plain table headers and title
    bars. Colours come from the OBS palette by way of setEchoPalette(), so a theme change
    in OBS repaints these windows to match.

    Buttons opt into two variants through component properties, which keeps the call sites
    in the window code to a single line each:
      - "echoPrimary": filled with the accent colour, for the confirming action
      - "echoQuiet":   no fill until hovered, for secondary actions
*/
class EchoLookAndFeel : public juce::LookAndFeel_V4
{
public:
    EchoLookAndFeel();
    ~EchoLookAndFeel() override;

    /** Rebuilds the palette from the OBS window background and text colours. */
    void setEchoPalette(juce::Colour background, juce::Colour text);

    const Palette& getPalette() const noexcept
    {
        return palette;
    }

    static constexpr const char* primaryButtonProperty = "echoPrimary";
    static constexpr const char* quietButtonProperty = "echoQuiet";

    //==============================================================================
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawTickBox(juce::Graphics&, juce::Component&, float, float, float, float, bool, bool, bool, bool) override;

    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

    juce::Font getLabelFont(juce::Label&) override;

    int getDefaultScrollbarWidth() override;
    void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int, int, int, int, bool, int, int, bool, bool) override;

    void drawTreeviewPlusMinusBox(juce::Graphics&, const juce::Rectangle<float>&, juce::Colour, bool, bool) override;

    void drawTableHeaderBackground(juce::Graphics&, juce::TableHeaderComponent&) override;
    void drawTableHeaderColumn(
        juce::Graphics&,
        juce::TableHeaderComponent&,
        const juce::String&,
        int,
        int,
        int,
        bool,
        bool,
        int
    ) override;

    void fillResizableWindowBackground(juce::Graphics&, int, int, const juce::BorderSize<int>&, juce::ResizableWindow&)
        override;
    void
    drawDocumentWindowTitleBar(juce::DocumentWindow&, juce::Graphics&, int, int, int, int, const juce::Image*, bool)
        override;
    juce::Button* createDocumentWindowButton(int buttonType) override;

    void drawPopupMenuBackgroundWithOptions(juce::Graphics&, int, int, const juce::PopupMenu::Options&) override;

private:
    void applyColourIds();

    Palette palette;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EchoLookAndFeel)
};

/**
    The palette a component should paint with. Falls back to deriving one from whatever
    look and feel is in force, so custom paint code still works if Echo's is not installed.
*/
inline Palette paletteFor(juce::Component& component)
{
    auto& lf = component.getLookAndFeel();

    if (auto* echoLookAndFeel = dynamic_cast<EchoLookAndFeel*>(&lf))
        return echoLookAndFeel->getPalette();

    return Palette::fromObsColours(
        lf.findColour(juce::ResizableWindow::backgroundColourId),
        lf.findColour(juce::Label::textColourId)
    );
}

} // namespace echo::ui

/*
Echo
Copyright (C) 2025 Voidscape Development
Distributed under the AGPLv3. See the LICENSE file for details.
*/

#include "EchoLookAndFeel.h"

#include "EchoWidgets.h"

namespace echo::ui
{

namespace
{
bool hasFlag(const juce::Component& c, const char* property)
{
    return static_cast<bool>(c.getProperties().getWithDefault(property, false));
}

/** The close/minimise/maximise glyphs, drawn as strokes rather than V4's filled shapes. */
class TitleBarButton final : public juce::Button
{
public:
    TitleBarButton(int type, const Palette& p)
        : juce::Button({})
        , buttonType(type)
        , palette(p)
    {
    }

    void paintButton(juce::Graphics& g, bool isOver, bool isDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);

        if (isOver || isDown)
        {
            const auto highlight = buttonType == juce::DocumentWindow::closeButton
                ? juce::Colour(0xffe05260).withAlpha(isDown ? 1.0f : 0.85f)
                : palette.controlHover;
            g.setColour(highlight);
            g.fillRoundedRectangle(bounds, 4.0f);
        }

        const auto glyph = bounds.withSizeKeepingCentre(9.0f, 9.0f);
        const auto colour = (isOver || isDown) && buttonType == juce::DocumentWindow::closeButton ? juce::Colours::white
                                                                                                  : palette.textDim;
        g.setColour(colour);

        constexpr float thickness = 1.4f;

        switch (buttonType)
        {
        case juce::DocumentWindow::closeButton:
            g.drawLine(glyph.getX(), glyph.getY(), glyph.getRight(), glyph.getBottom(), thickness);
            g.drawLine(glyph.getX(), glyph.getBottom(), glyph.getRight(), glyph.getY(), thickness);
            break;

        case juce::DocumentWindow::minimiseButton:
            g.drawLine(glyph.getX(), glyph.getCentreY(), glyph.getRight(), glyph.getCentreY(), thickness);
            break;

        default:
            g.drawRoundedRectangle(glyph.reduced(0.5f), 1.5f, thickness);
            break;
        }
    }

private:
    int buttonType;
    const Palette& palette;
};
} // namespace

EchoLookAndFeel::EchoLookAndFeel()
{
    palette = Palette::fromObsColours(juce::Colour(0xff272a33), juce::Colour(0xffe8eaed));
    applyColourIds();
}

EchoLookAndFeel::~EchoLookAndFeel() = default;

void EchoLookAndFeel::setEchoPalette(juce::Colour background, juce::Colour text)
{
    palette = Palette::fromObsColours(background, text);
    applyColourIds();
}

void EchoLookAndFeel::applyColourIds()
{
    using namespace juce;

    setColour(ResizableWindow::backgroundColourId, palette.windowBg);
    setColour(DocumentWindow::textColourId, palette.text);

    setColour(Label::textColourId, palette.text);
    setColour(Label::backgroundColourId, Colours::transparentBlack);

    setColour(TextButton::buttonColourId, palette.control);
    setColour(TextButton::buttonOnColourId, palette.accent);
    setColour(TextButton::textColourOffId, palette.text);
    setColour(TextButton::textColourOnId, palette.onAccent);

    setColour(ToggleButton::textColourId, palette.text);
    setColour(ToggleButton::tickColourId, palette.accent);
    setColour(ToggleButton::tickDisabledColourId, palette.textDim);

    setColour(ComboBox::backgroundColourId, palette.control);
    setColour(ComboBox::textColourId, palette.text);
    setColour(ComboBox::outlineColourId, palette.border);
    setColour(ComboBox::arrowColourId, palette.textDim);
    setColour(ComboBox::focusedOutlineColourId, palette.accent);

    setColour(PopupMenu::backgroundColourId, palette.panel);
    setColour(PopupMenu::textColourId, palette.text);
    setColour(PopupMenu::highlightedBackgroundColourId, palette.accent);
    setColour(PopupMenu::highlightedTextColourId, palette.onAccent);
    setColour(PopupMenu::headerTextColourId, palette.textDim);

    setColour(TextEditor::backgroundColourId, palette.control);
    setColour(TextEditor::textColourId, palette.text);
    setColour(TextEditor::outlineColourId, palette.border);
    setColour(TextEditor::focusedOutlineColourId, palette.accent);
    setColour(TextEditor::highlightColourId, palette.accentSoft);
    setColour(TextEditor::highlightedTextColourId, palette.text);
    setColour(CaretComponent::caretColourId, palette.accent);

    setColour(ScrollBar::backgroundColourId, Colours::transparentBlack);
    setColour(ScrollBar::thumbColourId, palette.borderStrong);
    setColour(ScrollBar::trackColourId, Colours::transparentBlack);

    setColour(ListBox::backgroundColourId, Colours::transparentBlack);
    setColour(ListBox::outlineColourId, palette.border);
    setColour(ListBox::textColourId, palette.text);

    setColour(TreeView::backgroundColourId, Colours::transparentBlack);
    setColour(TreeView::linesColourId, palette.border);
    setColour(TreeView::dragAndDropIndicatorColourId, palette.accent);
    setColour(TreeView::selectedItemBackgroundColourId, palette.accentSoft);
    setColour(TreeView::oddItemsColourId, Colours::transparentBlack);
    setColour(TreeView::evenItemsColourId, Colours::transparentBlack);

    setColour(TableHeaderComponent::backgroundColourId, palette.panelHeader);
    setColour(TableHeaderComponent::textColourId, palette.textDim);
    setColour(TableHeaderComponent::outlineColourId, palette.border);
    setColour(TableHeaderComponent::highlightColourId, palette.controlHover);

    setColour(Slider::rotarySliderFillColourId, palette.accent);
    setColour(Slider::rotarySliderOutlineColourId, palette.control);
    setColour(Slider::thumbColourId, palette.text);
    setColour(Slider::trackColourId, palette.accent);
    setColour(Slider::backgroundColourId, palette.control);
    setColour(Slider::textBoxTextColourId, palette.text);
    setColour(Slider::textBoxBackgroundColourId, palette.control);
    setColour(Slider::textBoxOutlineColourId, palette.border);

    setColour(GroupComponent::outlineColourId, palette.border);
    setColour(GroupComponent::textColourId, palette.textDim);

    setColour(TooltipWindow::backgroundColourId, palette.panel);
    setColour(TooltipWindow::textColourId, palette.text);
    setColour(TooltipWindow::outlineColourId, palette.border);

    setColour(AlertWindow::backgroundColourId, palette.panel);
    setColour(AlertWindow::textColourId, palette.text);
    setColour(AlertWindow::outlineColourId, palette.border);

    setColour(ProgressBar::backgroundColourId, palette.control);
    setColour(ProgressBar::foregroundColourId, palette.accent);
}

//==============================================================================
juce::Font EchoLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight)
{
    return bodyFont(juce::jmin(metrics::bodyFontSize, (float)buttonHeight * 0.45f));
}

void EchoLookAndFeel::drawButtonBackground(
    juce::Graphics& g,
    juce::Button& button,
    const juce::Colour& /*backgroundColour*/,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown
)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
    const bool primary = hasFlag(button, primaryButtonProperty);
    const bool quiet = hasFlag(button, quietButtonProperty);
    const bool enabled = button.isEnabled();

    juce::Colour fill;
    juce::Colour outline;

    if (primary)
    {
        fill = shouldDrawButtonAsDown       ? palette.accent.darker(0.15f)
            : shouldDrawButtonAsHighlighted ? palette.accentHover
                                            : palette.accent;
        outline = juce::Colours::transparentBlack;
    }
    else if (quiet)
    {
        fill = shouldDrawButtonAsDown       ? palette.control
            : shouldDrawButtonAsHighlighted ? palette.control.withAlpha(0.7f)
                                            : juce::Colours::transparentBlack;
        outline = juce::Colours::transparentBlack;
    }
    else
    {
        fill = shouldDrawButtonAsDown       ? sink(palette.control, 0.12f)
            : shouldDrawButtonAsHighlighted ? palette.controlHover
                                            : palette.control;
        outline = palette.border;
    }

    if (!enabled)
        fill = fill.withMultipliedAlpha(0.5f);

    g.setColour(fill);
    g.fillRoundedRectangle(bounds, metrics::controlCorner);

    if (!outline.isTransparent())
    {
        g.setColour(enabled ? outline : outline.withMultipliedAlpha(0.5f));
        g.drawRoundedRectangle(bounds, metrics::controlCorner, 1.0f);
    }

    if (button.hasKeyboardFocus(false) && enabled)
    {
        g.setColour(palette.accent.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(1.0f), metrics::controlCorner - 1.0f, 1.5f);
    }
}

void EchoLookAndFeel::drawButtonText(
    juce::Graphics& g,
    juce::TextButton& button,
    bool /*shouldDrawButtonAsHighlighted*/,
    bool /*shouldDrawButtonAsDown*/
)
{
    const bool primary = hasFlag(button, primaryButtonProperty);
    const bool quiet = hasFlag(button, quietButtonProperty);

    auto colour = primary ? palette.onAccent : (quiet ? palette.textDim : palette.text);
    if (!button.isEnabled())
        colour = colour.withMultipliedAlpha(0.5f);

    g.setColour(colour);
    g.setFont(getTextButtonFont(button, button.getHeight()));
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
}

void EchoLookAndFeel::drawToggleButton(
    juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool shouldDrawButtonAsDown
)
{
    const auto boxSize = 16.0f;
    auto bounds = button.getLocalBounds().toFloat();
    const auto box = bounds.removeFromLeft(boxSize + 4.0f).withSizeKeepingCentre(boxSize, boxSize);

    drawTickBox(
        g,
        button,
        box.getX(),
        box.getY(),
        box.getWidth(),
        box.getHeight(),
        button.getToggleState(),
        button.isEnabled(),
        shouldDrawButtonAsHighlighted,
        shouldDrawButtonAsDown
    );

    g.setColour(button.isEnabled() ? palette.text : palette.textDim);
    g.setFont(bodyFont());
    g.drawText(button.getButtonText(), bounds.withTrimmedLeft(6.0f), juce::Justification::centredLeft, true);
}

void EchoLookAndFeel::drawTickBox(
    juce::Graphics& g,
    juce::Component& /*component*/,
    float x,
    float y,
    float w,
    float h,
    bool ticked,
    bool isEnabled,
    bool shouldDrawButtonAsHighlighted,
    bool /*shouldDrawButtonAsDown*/
)
{
    auto working = palette;
    if (shouldDrawButtonAsHighlighted)
        working.control = palette.controlHover;
    if (!isEnabled)
    {
        working.accent = palette.accent.withMultipliedAlpha(0.5f);
        working.control = palette.control.withMultipliedAlpha(0.5f);
    }

    drawCheckbox(g, {x, y, w, h}, ticked, working);
}

//==============================================================================
void EchoLookAndFeel::drawComboBox(
    juce::Graphics& g,
    int width,
    int height,
    bool isButtonDown,
    int /*buttonX*/,
    int /*buttonY*/,
    int /*buttonW*/,
    int /*buttonH*/,
    juce::ComboBox& box
)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(0.5f);

    g.setColour(isButtonDown || box.isMouseOver(true) ? palette.controlHover : palette.control);
    g.fillRoundedRectangle(bounds, metrics::controlCorner);

    g.setColour(box.hasKeyboardFocus(false) ? palette.accent : palette.border);
    g.drawRoundedRectangle(bounds, metrics::controlCorner, 1.0f);

    // Chevron
    const auto arrowArea = juce::Rectangle<float>((float)width - 26.0f, 0.0f, 16.0f, (float)height);
    const auto centre = arrowArea.getCentre();

    juce::Path chevron;
    chevron.startNewSubPath(centre.x - 4.5f, centre.y - 2.0f);
    chevron.lineTo(centre.x, centre.y + 2.75f);
    chevron.lineTo(centre.x + 4.5f, centre.y - 2.0f);

    g.setColour(box.isEnabled() ? palette.textDim : palette.textDim.withMultipliedAlpha(0.5f));
    g.strokePath(chevron, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

juce::Font EchoLookAndFeel::getComboBoxFont(juce::ComboBox&)
{
    return bodyFont();
}

void EchoLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(10, 1, box.getWidth() - 34, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
    label.setColour(juce::Label::textColourId, box.findColour(juce::ComboBox::textColourId));
}

juce::Font EchoLookAndFeel::getLabelFont(juce::Label& label)
{
    const auto height = (float)label.getHeight();
    return bodyFont(height > 0.0f ? juce::jlimit(11.0f, metrics::bodyFontSize, height * 0.55f) : metrics::bodyFontSize);
}

//==============================================================================
int EchoLookAndFeel::getDefaultScrollbarWidth()
{
    return 10;
}

void EchoLookAndFeel::drawScrollbar(
    juce::Graphics& g,
    juce::ScrollBar& scrollbar,
    int x,
    int y,
    int width,
    int height,
    bool isScrollbarVertical,
    int thumbStartPosition,
    int thumbSize,
    bool isMouseOver,
    bool isMouseDown
)
{
    juce::ignoreUnused(scrollbar);

    if (thumbSize <= 0)
        return;

    auto thumb = isScrollbarVertical ? juce::Rectangle<int>(x, thumbStartPosition, width, thumbSize)
                                     : juce::Rectangle<int>(thumbStartPosition, y, thumbSize, height);

    const auto inset = isScrollbarVertical ? juce::Point<int>(3, 1) : juce::Point<int>(1, 3);
    const auto r = thumb.reduced(inset.x, inset.y).toFloat();

    const float alpha = isMouseDown ? 0.95f : (isMouseOver ? 0.75f : 0.45f);

    g.setColour(palette.borderStrong.withMultipliedAlpha(alpha));
    g.fillRoundedRectangle(r, juce::jmin(r.getWidth(), r.getHeight()) * 0.5f);
}

void EchoLookAndFeel::drawTreeviewPlusMinusBox(
    juce::Graphics& g,
    const juce::Rectangle<float>& area,
    juce::Colour /*backgroundColour*/,
    bool isItemOpen,
    bool isMouseOver
)
{
    const auto centre = area.getCentre();
    const float size = 4.0f;

    juce::Path chevron;

    if (isItemOpen)
    {
        chevron.startNewSubPath(centre.x - size, centre.y - size * 0.55f);
        chevron.lineTo(centre.x, centre.y + size * 0.6f);
        chevron.lineTo(centre.x + size, centre.y - size * 0.55f);
    }
    else
    {
        chevron.startNewSubPath(centre.x - size * 0.55f, centre.y - size);
        chevron.lineTo(centre.x + size * 0.6f, centre.y);
        chevron.lineTo(centre.x - size * 0.55f, centre.y + size);
    }

    g.setColour(isMouseOver ? palette.text : palette.textDim);
    g.strokePath(chevron, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

//==============================================================================
void EchoLookAndFeel::drawTableHeaderBackground(juce::Graphics& g, juce::TableHeaderComponent& header)
{
    g.setColour(palette.panelHeader);
    g.fillRect(header.getLocalBounds());

    g.setColour(palette.border);
    g.drawLine(
        0.0f,
        (float)header.getHeight() - 0.5f,
        (float)header.getWidth(),
        (float)header.getHeight() - 0.5f,
        1.0f
    );
}

void EchoLookAndFeel::drawTableHeaderColumn(
    juce::Graphics& g,
    juce::TableHeaderComponent& header,
    const juce::String& columnName,
    int /*columnId*/,
    int width,
    int height,
    bool isMouseOver,
    bool isMouseDown,
    int /*columnFlags*/
)
{
    juce::ignoreUnused(header);

    if (isMouseDown || isMouseOver)
    {
        g.setColour(palette.controlHover.withAlpha(isMouseDown ? 0.6f : 0.35f));
        g.fillRect(juce::Rectangle<int>(0, 0, width, height).reduced(1));
    }

    g.setColour(palette.border.withMultipliedAlpha(0.7f));
    g.drawLine((float)width - 0.5f, 6.0f, (float)width - 0.5f, (float)height - 6.0f, 1.0f);

    g.setColour(palette.textDim);
    g.setFont(bodyFont(metrics::smallFontSize, true));
    g.drawText(columnName, juce::Rectangle<int>(8, 0, width - 12, height), juce::Justification::centredLeft, true);
}

//==============================================================================
void EchoLookAndFeel::
    fillResizableWindowBackground(juce::Graphics& g, int /*w*/, int /*h*/, const juce::BorderSize<int>&, juce::ResizableWindow&)
{
    g.fillAll(palette.windowBg);
}

void EchoLookAndFeel::drawDocumentWindowTitleBar(
    juce::DocumentWindow& window,
    juce::Graphics& g,
    int w,
    int h,
    int titleSpaceX,
    int titleSpaceW,
    const juce::Image* icon,
    bool drawTitleTextOnLeft
)
{
    juce::ignoreUnused(icon, drawTitleTextOnLeft);

    g.setColour(palette.panelHeader);
    g.fillRect(0, 0, w, h);

    g.setColour(palette.border);
    g.drawLine(0.0f, (float)h - 0.5f, (float)w, (float)h - 0.5f, 1.0f);

    // A small accent tick at the leading edge, so the window reads as ours at a glance.
    g.setColour(palette.accent);
    g.fillRoundedRectangle(juce::Rectangle<float>(metrics::panelPadding, (float)h * 0.5f - 6.0f, 3.0f, 12.0f), 1.5f);

    const auto textX = metrics::panelPadding + 10;
    auto textArea = juce::Rectangle<int>(textX, 0, juce::jmax(0, titleSpaceX + titleSpaceW - textX), h);

    g.setColour(palette.text);
    g.setFont(bodyFont(metrics::bodyFontSize, true));
    g.drawText(window.getName(), textArea, juce::Justification::centredLeft, true);
}

juce::Button* EchoLookAndFeel::createDocumentWindowButton(int buttonType)
{
    return new TitleBarButton(buttonType, palette);
}

void EchoLookAndFeel::
    drawPopupMenuBackgroundWithOptions(juce::Graphics& g, int width, int height, const juce::PopupMenu::Options&)
{
    const auto bounds = juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height).reduced(0.5f);

    g.setColour(palette.panel);
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(palette.border);
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

} // namespace echo::ui

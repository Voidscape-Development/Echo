/*
Echo
Copyright (C) 2025 Voidscape Development
Distributed under the AGPLv3. See the LICENSE file for details.

Renders the Device IO windows to PNGs offscreen so design changes can be reviewed without
building OBS. "Before" uses the current atk::LookAndFeel verbatim; "after" uses Echo's.

    cmake -S tools/ui-preview -B build-ui-preview && cmake --build build-ui-preview
    ./build-ui-preview/ui-preview_artefacts/ui-preview <output-dir>
*/

#include "../../src/core/atkaudio/LookAndFeel.h"
#include "../../src/ui/EchoLayout.h"
#include "../../src/ui/EchoLookAndFeel.h"
#include "../../src/ui/EchoWidgets.h"

#include <juce_gui_basics/juce_gui_basics.h>

using namespace echo::ui;

namespace
{
juce::Colour obsBackground = juce::Colour(0xff272a33);
juce::Colour obsText = juce::Colour(0xffe8eaed);

Palette currentPalette()
{
    return Palette::fromObsColours(obsBackground, obsText);
}

//==============================================================================
// A device tree item mirroring AudioServerSettingsComponent::DeviceChannelTreeItem.
class MockTreeItem final : public juce::TreeViewItem
{
public:
    enum class Kind
    {
        DeviceType,
        Device,
        Channel
    };

    MockTreeItem(juce::String nameIn, Kind kindIn, bool modernIn, bool subscribedIn = false)
        : name(std::move(nameIn))
        , kind(kindIn)
        , modern(modernIn)
        , subscribed(subscribedIn)
    {
    }

    bool mightContainSubItems() override
    {
        return kind != Kind::Channel;
    }

    int getItemHeight() const override
    {
        return modern ? 24 : 20;
    }

    void paintItem(juce::Graphics& g, int width, int height) override
    {
        if (modern)
        {
            const auto palette = currentPalette();
            auto bounds = juce::Rectangle<int>(0, 0, width, height);

            if (isSelected())
            {
                g.setColour(palette.accentSoft);
                g.fillRoundedRectangle(bounds.toFloat().reduced(1.0f), 4.0f);
            }

            if (kind == Kind::Channel)
            {
                auto boxArea = bounds.removeFromLeft(24).toFloat();
                drawCheckbox(g, boxArea, subscribed, palette);

                g.setColour(subscribed ? palette.text : palette.textDim);
                g.setFont(bodyFont(13.0f));
                g.drawText(name, bounds.withTrimmedLeft(2), juce::Justification::centredLeft, true);
            }
            else
            {
                g.setColour(palette.text);
                g.setFont(bodyFont(13.0f, kind == Kind::DeviceType));
                g.drawText(name, bounds.withTrimmedLeft(4), juce::Justification::centredLeft, true);
            }
        }
        else
        {
            // Verbatim from the current implementation.
            auto& lf = getOwnerView()->getLookAndFeel();

            if (isSelected())
                g.fillAll(lf.findColour(juce::TreeView::selectedItemBackgroundColourId));

            g.setColour(lf.findColour(juce::Label::textColourId));
            g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), height * 0.7f, juce::Font::plain));

            juce::String displayText = name;
            if (kind == Kind::Channel)
                displayText = (subscribed ? "[X] " : "[ ] ") + displayText;

            g.drawText(displayText, 4, 0, width - 4, height, juce::Justification::centredLeft, true);
        }
    }

    void addKid(MockTreeItem* item)
    {
        addSubItem(item);
    }

private:
    juce::String name;
    Kind kind;
    bool modern;
    bool subscribed;
};

//==============================================================================
// A routing matrix mirroring AudioServerSettingsComponent::ChannelMappingMatrix.
class MockMatrix final : public juce::Component, public juce::TableListBoxModel
{
public:
    MockMatrix(bool modernIn, juce::StringArray rowsIn, int numChannels, juce::String firstColumn)
        : modern(modernIn)
        , rowLabels(std::move(rowsIn))
        , channels(numChannels)
    {
        addAndMakeVisible(table);
        table.setModel(this);
        table.setHeaderHeight(modern ? 28 : 22);
        table.setRowHeight(modern ? 26 : 22);
        table.setMultipleSelectionEnabled(false);
        table.setClickingTogglesRowSelection(false);

        auto& header = table.getHeader();
        header.addColumn(firstColumn, 1, 170, 120, 260, juce::TableHeaderComponent::notSortable);
        for (int i = 0; i < channels; ++i)
            header.addColumn(juce::String(i + 1), i + 2, 44, 30, 70, juce::TableHeaderComponent::notSortable);

        grid.resize((size_t)rowLabels.size(), std::vector<bool>((size_t)channels, false));
        for (size_t r = 0; r < grid.size(); ++r)
            grid[r][r % (size_t)channels] = true;

        if (!modern)
            table.setColour(juce::ListBox::outlineColourId, juce::Colours::black.withAlpha(0.3f));
    }

    int getNumRows() override
    {
        return rowLabels.size();
    }

    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool) override
    {
        if (!modern)
            return;

        if (rowNumber % 2 == 1)
        {
            g.setColour(currentPalette().rowAlt.withAlpha(0.6f));
            g.fillRect(0, 0, width, height);
        }
    }

    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool) override
    {
        if (rowNumber >= rowLabels.size())
            return;

        if (columnId == 1)
        {
            if (modern)
            {
                const auto palette = currentPalette();
                g.setColour(palette.text);
                g.setFont(bodyFont(12.5f));
            }
            else
            {
                g.setColour(juce::Colours::white);
                g.setFont(11.0f);
            }

            g.drawText(rowLabels[rowNumber], 8, 0, width - 12, height, juce::Justification::centredLeft, true);
            return;
        }

        const int channel = columnId - 2;
        if (channel < 0 || channel >= channels)
            return;

        const bool mapped = grid[(size_t)rowNumber][(size_t)channel];

        if (modern)
        {
            drawMatrixCell(
                g,
                juce::Rectangle<float>(0.0f, 0.0f, (float)width, (float)height),
                mapped,
                currentPalette()
            );
        }
        else
        {
            if (mapped)
            {
                g.setColour(juce::Colours::white);
                g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 16.0f, juce::Font::bold));
                g.drawText("X", 0, 0, width, height, juce::Justification::centred);
            }

            g.setColour(juce::Colours::black.withAlpha(0.3f));
            g.drawRect(0, 0, width, height, 1);
        }
    }

    void resized() override
    {
        table.setBounds(getLocalBounds());
    }

private:
    bool modern;
    juce::TableListBox table;
    juce::StringArray rowLabels;
    int channels;
    std::vector<std::vector<bool>> grid;
};

//==============================================================================
/** The Device IO 2 window content, in either the current or the redesigned layout. */
class DeviceIo2Window final : public juce::Component
{
public:
    explicit DeviceIo2Window(bool modernIn)
        : modern(modernIn)
    {
        buildTree(inputTree, inputRoot, true);
        buildTree(outputTree, outputRoot, false);

        inputMatrix = std::make_unique<MockMatrix>(
            modern,
            juce::StringArray{"OBS 1", "OBS 2", "Scarlett 18i20 Ch 1", "Scarlett 18i20 Ch 2"},
            4,
            "Channel"
        );
        outputMatrix = std::make_unique<MockMatrix>(
            modern,
            juce::StringArray{"OBS 1", "OBS 2", "Scarlett 18i20 Ch 3", "MOTU M4 Ch 1"},
            4,
            "Channel"
        );

        addAndMakeVisible(*inputMatrix);
        addAndMakeVisible(*outputMatrix);

        addAndMakeVisible(applyButton);
        addAndMakeVisible(discardButton);
        addAndMakeVisible(resetButton);
        addAndMakeVisible(deviceButton);

        if (modern)
        {
            applyButton.getProperties().set(EchoLookAndFeel::primaryButtonProperty, true);
            discardButton.getProperties().set(EchoLookAndFeel::quietButtonProperty, true);
            resetButton.getProperties().set(EchoLookAndFeel::quietButtonProperty, true);
        }
        else
        {
            inputTreeLabel.setText("Input Devices", juce::dontSendNotification);
            outputTreeLabel.setText("Output Devices", juce::dontSendNotification);
            addAndMakeVisible(inputTreeLabel);
            addAndMakeVisible(outputTreeLabel);
        }
    }

    void paint(juce::Graphics& g) override
    {
        const auto palette = currentPalette();

        if (!modern)
        {
            g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
            return;
        }

        g.fillAll(palette.windowBg);

        const auto layout = DeviceIo2Layout::compute(getLocalBounds());

        const struct
        {
            juce::Rectangle<int> bounds;
            const char* title;
            juce::String hint;
        } cards[] = {
            {layout.inputPanel, "Inputs", "4 channels"},
            {layout.outputPanel, "Outputs", "4 channels"},
            {layout.inputMatrixPanel, "Input routing", "device to OBS"},
            {layout.outputMatrixPanel, "Output routing", "OBS to device"},
        };

        for (const auto& card : cards)
        {
            drawPanel(g, card.bounds, palette);
            drawPanelHeader(g, card.bounds, card.title, card.hint, palette);
        }
    }

    void resized() override
    {
        if (!modern)
        {
            // The current layout, verbatim.
            auto bounds = getLocalBounds().reduced(10);

            auto buttonArea = bounds.removeFromBottom(30);
            buttonArea.removeFromTop(5);
            applyButton.setBounds(buttonArea.removeFromRight(80));
            buttonArea.removeFromRight(5);
            discardButton.setBounds(buttonArea.removeFromRight(80));
            buttonArea.removeFromRight(5);
            resetButton.setBounds(buttonArea.removeFromRight(80));
            buttonArea.removeFromRight(5);
            deviceButton.setBounds(buttonArea.removeFromRight(80));

            bounds.removeFromBottom(10);

            auto topSection = bounds.removeFromTop((int)(bounds.getHeight() * 0.6f));
            auto inputSection = topSection.removeFromLeft(topSection.getWidth() / 2).reduced(5);
            auto outputSection = topSection.reduced(5);

            inputTreeLabel.setBounds(inputSection.removeFromTop(30));
            inputTree.setBounds(inputSection);
            outputTreeLabel.setBounds(outputSection.removeFromTop(30));
            outputTree.setBounds(outputSection);

            bounds.removeFromTop(10);
            auto matrixSection = bounds;
            inputMatrix->setBounds(matrixSection.removeFromLeft(matrixSection.getWidth() / 2).reduced(5));
            outputMatrix->setBounds(matrixSection.reduced(5));
            return;
        }

        // The shipped geometry, straight from the shared layout the component uses.
        const auto layout = DeviceIo2Layout::compute(getLocalBounds());

        deviceButton.setBounds(layout.deviceButton);
        resetButton.setBounds(layout.resetButton);
        discardButton.setBounds(layout.discardButton);
        applyButton.setBounds(layout.applyButton);

        inputTree.setBounds(DeviceIo2Layout::panelContent(layout.inputPanel));
        outputTree.setBounds(DeviceIo2Layout::panelContent(layout.outputPanel));
        inputMatrix->setBounds(DeviceIo2Layout::panelContent(layout.inputMatrixPanel));
        outputMatrix->setBounds(DeviceIo2Layout::panelContent(layout.outputMatrixPanel));
    }

    /** Drawn by the harness rather than a real peer, since these windows are offscreen. */
    juce::String windowTitle{"Echo Device IO 2 - Audio Settings"};

private:
    void buildTree(juce::TreeView& tree, std::unique_ptr<MockTreeItem>& root, bool isInput)
    {
        root = std::make_unique<MockTreeItem>("Devices", MockTreeItem::Kind::DeviceType, modern);

        auto* type = new MockTreeItem(isInput ? "ASIO" : "Windows Audio", MockTreeItem::Kind::DeviceType, modern);
        auto* device = new MockTreeItem("Focusrite Scarlett 18i20", MockTreeItem::Kind::Device, modern);
        device->addKid(new MockTreeItem("Channel 1", MockTreeItem::Kind::Channel, modern, true));
        device->addKid(new MockTreeItem("Channel 2", MockTreeItem::Kind::Channel, modern, isInput));
        device->addKid(new MockTreeItem("Channel 3", MockTreeItem::Kind::Channel, modern, false));
        device->addKid(new MockTreeItem("Channel 4", MockTreeItem::Kind::Channel, modern, false));
        type->addKid(device);

        auto* second = new MockTreeItem("MOTU M4", MockTreeItem::Kind::Device, modern);
        second->addKid(new MockTreeItem("Channel 1", MockTreeItem::Kind::Channel, modern, false));
        second->addKid(new MockTreeItem("Channel 2", MockTreeItem::Kind::Channel, modern, false));
        type->addKid(second);

        root->addKid(type);

        tree.setDefaultOpenness(true);
        tree.setRootItem(root.get());
        tree.setRootItemVisible(false);
        tree.setIndentSize(18);
        addAndMakeVisible(tree);

        root->setOpen(true);
        type->setOpen(true);
        device->setOpen(true);
        second->setOpen(true);

        if (modern)
            tree.setColour(juce::TreeView::backgroundColourId, juce::Colours::transparentBlack);
    }

    bool modern;

    juce::TreeView inputTree, outputTree;
    std::unique_ptr<MockTreeItem> inputRoot, outputRoot;
    juce::Label inputTreeLabel, outputTreeLabel;

    std::unique_ptr<MockMatrix> inputMatrix, outputMatrix;

    juce::TextButton applyButton{"Apply"};
    juce::TextButton discardButton{"Discard"};
    juce::TextButton resetButton{"Reset"};
    juce::TextButton deviceButton{"Device..."};
};

//==============================================================================
/**
    The Device IO (v1) window. The real window hosts juce::AudioDeviceSelectorComponent;
    this mirrors the rows that component lays out, since a headless container has no audio
    devices for the real one to list. Only the skin differs between before and after.
*/
class DeviceIoWindow final : public juce::Component
{
public:
    explicit DeviceIoWindow(bool modernIn)
        : modern(modernIn)
    {
        addRow(typeLabel, "Audio device type:", typeBox, {"ASIO"});
        addRow(deviceLabel, "Device:", deviceBox, {"Focusrite Scarlett 18i20"});
        addRow(rateLabel, "Sample rate:", rateBox, {"48000 Hz"});
        addRow(bufferLabel, "Audio buffer size:", bufferBox, {"256 samples (5.3 ms)"});

        addAndMakeVisible(channelsLabel);
        channelsLabel.setText("Active output channels:", juce::dontSendNotification);
        channelsLabel.setJustificationType(juce::Justification::centredRight);

        for (int i = 0; i < 4; ++i)
        {
            auto* toggle = channels.add(new juce::ToggleButton("Output channel " + juce::String(i + 1)));
            toggle->setToggleState(i < 2, juce::dontSendNotification);
            addAndMakeVisible(*toggle);
        }

        addAndMakeVisible(testButton);
        addAndMakeVisible(panelButton);

        if (modern)
            testButton.getProperties().set(EchoLookAndFeel::quietButtonProperty, true);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void resized() override
    {
        const int rowHeight = modern ? 34 : 30;
        const int labelWidth = 150;

        auto bounds = getLocalBounds().reduced(modern ? metrics::windowMargin : 12);

        auto layoutRow = [&](juce::Label& label, juce::Component& control, juce::Component* trailing)
        {
            auto row = bounds.removeFromTop(rowHeight);
            label.setBounds(row.removeFromLeft(labelWidth).withTrimmedRight(10));

            if (trailing != nullptr)
            {
                trailing->setBounds(row.removeFromRight(70).reduced(0, modern ? 2 : 0));
                row.removeFromRight(8);
            }

            control.setBounds(row.reduced(0, modern ? 2 : 0));
            bounds.removeFromTop(modern ? 8 : 4);
        };

        layoutRow(typeLabel, typeBox, nullptr);
        layoutRow(deviceLabel, deviceBox, &testButton);

        auto channelArea = bounds.removeFromTop(rowHeight * 4);
        channelsLabel.setBounds(channelArea.removeFromLeft(labelWidth).withTrimmedRight(10).withHeight(rowHeight));
        for (auto* toggle : channels)
            toggle->setBounds(channelArea.removeFromTop(rowHeight - (modern ? 6 : 8)));
        bounds.removeFromTop(modern ? 8 : 4);

        layoutRow(rateLabel, rateBox, nullptr);
        layoutRow(bufferLabel, bufferBox, nullptr);

        auto footer = bounds.removeFromTop(rowHeight);
        footer.removeFromLeft(labelWidth);
        panelButton.setBounds(footer.removeFromLeft(140).reduced(0, modern ? 2 : 0));
    }

    juce::String windowTitle{"Echo Device IO - Audio Settings"};

private:
    void addRow(juce::Label& label, const juce::String& text, juce::ComboBox& box, const juce::StringArray& items)
    {
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredRight);
        addAndMakeVisible(label);

        box.addItemList(items, 1);
        box.setSelectedId(1, juce::dontSendNotification);
        addAndMakeVisible(box);
    }

    bool modern;

    juce::OwnedArray<juce::Label> labels;
    juce::Label typeLabel, deviceLabel, rateLabel, bufferLabel;

    juce::Label channelsLabel;
    juce::OwnedArray<juce::ToggleButton> channels;

    juce::ComboBox typeBox, deviceBox, rateBox, bufferBox;
    juce::TextButton testButton{"Test"};
    juce::TextButton panelButton{"Control Panel"};
};

//==============================================================================
/** Composites a JUCE-drawn title bar above the component, the way the real window looks. */
juce::Image renderWindow(juce::Component& content, const juce::String& title, juce::LookAndFeel& lf, float scale)
{
    const int titleHeight = 30;

    juce::DocumentWindow window(title, lf.findColour(juce::ResizableWindow::backgroundColourId), 0);
    window.setLookAndFeel(&lf);

    const int w = content.getWidth();
    const int h = content.getHeight() + titleHeight;

    juce::Image image(juce::Image::ARGB, juce::roundToInt(w * scale), juce::roundToInt(h * scale), true);
    {
        juce::Graphics g(image);
        g.addTransform(juce::AffineTransform::scale(scale));

        lf.drawDocumentWindowTitleBar(window, g, w, titleHeight, 0, w - 40, nullptr, true);

        // The close button, drawn where a real window would place it.
        if (std::unique_ptr<juce::Button> closeButton{lf.createDocumentWindowButton(juce::DocumentWindow::closeButton)})
        {
            closeButton->setLookAndFeel(&lf);
            closeButton->setSize(titleHeight - 10, titleHeight - 10);

            juce::Graphics::ScopedSaveState save(g);
            g.setOrigin(w - titleHeight - 2, 5);
            closeButton->paintEntireComponent(g, false);
            closeButton->setLookAndFeel(nullptr);
        }

        juce::Graphics::ScopedSaveState save(g);
        g.setOrigin(0, titleHeight);
        content.paintEntireComponent(g, true);
    }

    window.setLookAndFeel(nullptr);
    return image;
}

bool writePng(const juce::Image& image, const juce::File& file)
{
    file.deleteFile();
    if (auto stream = file.createOutputStream())
    {
        juce::PNGImageFormat png;
        const bool ok = png.writeImageToStream(image, *stream);
        stream->flush();
        return ok;
    }
    return false;
}
} // namespace

int main(int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    const juce::File outDir = argc > 1 ? juce::File(juce::String(argv[1])) : juce::File::getCurrentWorkingDirectory();
    outDir.createDirectory();

    int written = 0;

    auto render = [&](const juce::String& name,
                      juce::Component& content,
                      const juce::String& title,
                      juce::LookAndFeel& lf,
                      float scale)
    {
        content.setLookAndFeel(&lf);
        content.resized();

        // TreeView and TableListBox lay their rows out asynchronously, so give the message
        // queue a chance to run before snapshotting.
        juce::MessageManager::getInstance()->runDispatchLoopUntil(120);
        content.resized();

        const auto image = renderWindow(content, title, lf, scale);
        content.setLookAndFeel(nullptr);

        if (writePng(image, outDir.getChildFile(name)))
        {
            ++written;
            std::printf("%s (%dx%d)\n", name.toRawUTF8(), image.getWidth(), image.getHeight());
        }
    };

    {
        atk::LookAndFeel before; // the look that ships today
        before.setColors(obsBackground, obsText);

        DeviceIo2Window window(false);
        window.setSize(900, 700);
        render("device-io-2-before.png", window, window.windowTitle, before, 1.4f);

        DeviceIoWindow v1(false);
        v1.setSize(560, 400);
        render("device-io-before.png", v1, v1.windowTitle, before, 1.6f);

        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    {
        EchoLookAndFeel after;
        after.setEchoPalette(obsBackground, obsText);
        juce::LookAndFeel::setDefaultLookAndFeel(&after);

        DeviceIo2Window window(true);
        window.setSize(900, 700);
        render("device-io-2-after.png", window, window.windowTitle, after, 1.4f);

        DeviceIoWindow v1(true);
        v1.setSize(560, 400);
        render("device-io-after.png", v1, v1.windowTitle, after, 1.6f);

        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    {
        // The same window against a light OBS theme, to show the palette tracking it.
        obsBackground = juce::Colour(0xfff2f3f5);
        obsText = juce::Colour(0xff1f2328);

        EchoLookAndFeel light;
        light.setEchoPalette(obsBackground, obsText);
        juce::LookAndFeel::setDefaultLookAndFeel(&light);

        DeviceIo2Window window(true);
        window.setSize(900, 700);
        render("device-io-2-after-light.png", window, window.windowTitle, light, 1.4f);

        juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
    }

    std::printf("wrote %d images to %s\n", written, outDir.getFullPathName().toRawUTF8());
    return written == 5 ? 0 : 1;
}

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ProjectAudioAudioProcessorEditor::ProjectAudioAudioProcessorEditor (ProjectAudioAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    dspOrderButton.onClick = [this]() {
        juce::Random r;
        ProjectAudioAudioProcessor::DSP_Order dspOrder;
        auto range = r.nextInt(juce::Range<int>(
            static_cast<int>(ProjectAudioAudioProcessor::DSP_Option::Phase),
            static_cast<int>(ProjectAudioAudioProcessor::DSP_Option::END_OF_LIST)
        ));

        for (auto& v : dspOrder) {
            auto entry = r.nextInt(range);
            v = static_cast<ProjectAudioAudioProcessor::DSP_Option>(entry);
        }

        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
        jassertfalse;
            
        audioProcessor.dsporderFifo.push(dspOrder);
        };
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(dspOrderButton);
    setSize (400, 300);
}

ProjectAudioAudioProcessorEditor::~ProjectAudioAudioProcessorEditor()
{
}

//==============================================================================
void ProjectAudioAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void ProjectAudioAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    dspOrderButton.setBounds(getLocalBounds().reduced(100));
}

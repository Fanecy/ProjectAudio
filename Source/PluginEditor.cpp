/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::String GetDspOptionName(ProjectAudioAudioProcessor::DSP_Option option)
{
    switch (option)
    {
    case ProjectAudioAudioProcessor::DSP_Option::Phase:
        return "PHASER";
    case ProjectAudioAudioProcessor::DSP_Option::Chorus:
        return "CHORUS";
    case ProjectAudioAudioProcessor::DSP_Option::Overdrive:
        return "OVERDRIVE";
    case ProjectAudioAudioProcessor::DSP_Option::LadderFilter:
        return "LADDERFILTER";
    case ProjectAudioAudioProcessor::DSP_Option::GeneralFilter:
        return "GENERALFILTER";
    case ProjectAudioAudioProcessor::DSP_Option::END_OF_LIST:
        jassertfalse;
        break;
    default:
        jassertfalse;
    }
        return "NO SELECTION";
}

//==============================================================================
ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar): juce::TabBarButton(name,ownerBar)
{

}

//==============================================================================
juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    return new ExtendedTabBarButton(tabName, *this);
}

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

        tabbedComponent.clearTabs(); //clear all tabs

        for (auto& v : dspOrder) {
            auto entry = r.nextInt(range);
            v = static_cast<ProjectAudioAudioProcessor::DSP_Option>(entry);
            tabbedComponent.addTab(GetDspOptionName(v), juce::Colours::white, -1); // -1 meaning put at the end of the list
        }

        
        DBG(juce::Base64::toBase64(dspOrder.data(), dspOrder.size()));
        //jassertfalse;
            
        audioProcessor.dsporderFifo.push(dspOrder);
        };
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(dspOrderButton);
    addAndMakeVisible(tabbedComponent);
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
    g.drawFittedText ("FUCKED UP PROMGRAMMING SKILLS", getLocalBounds(), juce::Justification::centred, 1);
}

void ProjectAudioAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();
    dspOrderButton.setBounds(bounds.removeFromTop(30).withSizeKeepingCentre(150,30));
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.withHeight(30));
}



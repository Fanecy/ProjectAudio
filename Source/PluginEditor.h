/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

/*
 Fane: Create a subclass of TabbedButtonBar which is also a DragAndDragTarget.
 And can create custom tab buttons which allow themselves to be dragged.
*/

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget
{
    ExtendedTabbedButtonBar() :juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop) {}; //construct func

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) { return false; }

    void itemDropped(const SourceDetails& dragSourceDetails) {}
};

struct ExtendedTabBarButton : juce::TabBarButton
{

};

//==============================================================================
/**
*/
class ProjectAudioAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ProjectAudioAudioProcessorEditor (ProjectAudioAudioProcessor&);
    ~ProjectAudioAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProjectAudioAudioProcessor& audioProcessor;

    juce::TextButton dspOrderButton{ "random dspOrder" };

    ExtendedTabbedButtonBar tabbedComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectAudioAudioProcessorEditor)
};

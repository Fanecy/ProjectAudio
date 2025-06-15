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

struct HorizontalConstrainer : juce::ComponentBoundsConstrainer //x-axis constrain
{
    /**
 * 构造一个水平约束器
 *
 * @param confinedBoundsGetter 回调函数，返回需要被约束的目标组件区域
 *                             
 * @param confineeBoundsGetter 回调函数，返回约束参考区域
 *                             
 *
 * 约束逻辑：目标组件的水平位置和宽度将被限制在参考区域内
 */
    HorizontalConstrainer(std::function<juce::Rectangle<int>() > confinedBoundsGetter,
        std::function < juce::Rectangle<int>() > confineeBoundsGetter); 

    void checkBounds(juce::Rectangle<int>& bounds,
        const juce::Rectangle<int>& previousBounds,
        const juce::Rectangle<int>& limits,
        bool isStretchingTop,
        bool isStretchingLeft,
        bool isStretchingBottom,
        bool isStretchingRight) override;  //Set Constrain

private:
    std::function<juce::Rectangle<int>()> boundsToConfineToGetter; //the box
    std::function<juce::Rectangle<int>()> boundsOfConfineeGetter; //the content
};

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget //Fane:whole tabs stuff
{
    ExtendedTabbedButtonBar(); //construct func

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

    void itemDropped(const SourceDetails& dragSourceDetails) override;

    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;
};

struct ExtendedTabBarButton : juce::TabBarButton //make one draggable tab
{
    ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar);

    juce::ComponentDragger dragger;

    std::unique_ptr<HorizontalConstrainer> constrainer;

    void mouseDown(const juce::MouseEvent& e) override;//重写了虚函数，所以要调用基类,保证原函数功能实现

    void mouseDrag(const juce::MouseEvent& e) override;
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

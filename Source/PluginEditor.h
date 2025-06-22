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

struct ExtendedTabbedButtonBar : juce::TabbedButtonBar, juce::DragAndDropTarget,juce::DragAndDropContainer //Fane:whole tabs stuff
{

    ExtendedTabbedButtonBar(); //construct func

    /*
     拖放重新排序的实现方式为：让 TabbedButtonBar 同时作为拖放目标（dragAndDrop Target）和拖放容器（DragAndDropContainer），
     并监听每个 TabBarButton 上的鼠标事件。

     流程如下：
     在选项卡（tab）上按下鼠标（mouseDown）。这会让拖放容器（DragAndDropContainer）开始响应鼠标拖动（mouseDrag）事件。
     首先，当第一个鼠标事件发生时，会调用 itemDragEnter 方法。
     随着鼠标移动，会调用 itemDragMove() 方法。选项卡按钮（tabBarButtons）受限于 ExtendedTabbedButtonBar 的边界，因此它们永远不会被拖出该边界。

     itemDragMove() 会检查正在拖动的项目的 x 坐标，并将其与其相邻项目的 x 坐标进行比较。
     如果一个选项卡越过了另一个选项卡的中间位置，就会交换这些选项卡的索引。
*/

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;

    struct Listener {      //绑定到drag事件，拖拽中触发
        virtual ~Listener() = default;

        virtual void tabOrderChanged(ProjectAudioAudioProcessor::DSP_Order newOrder) = 0;
    };
    void addListener(Listener* l);
    void removeListener(Listener* l);

    void mouseDown(const juce::MouseEvent& e) override;

    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;

private:
    //refrac-funcs to simplize funcs above
    juce::TabBarButton* findDraggedItem(const SourceDetails& dragSourceDetails);
    int findDraggedItemIndex(const SourceDetails& dragSourceDetails);
    juce::Array<juce::TabBarButton*> getTabs();

    juce::ScaledImage draggedImage;
    juce::ListenerList<Listener> listeners;
};

struct ExtendedTabBarButton : juce::TabBarButton //make one draggable tab
{
    ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar,ProjectAudioAudioProcessor::DSP_Option& option);

    juce::ComponentDragger dragger;

    std::unique_ptr<HorizontalConstrainer> constrainer;

    void mouseDown(const juce::MouseEvent& e) override;//重写了虚函数，所以要调用基类,保证原函数功能实现

    void mouseDrag(const juce::MouseEvent& e) override;

    ProjectAudioAudioProcessor::DSP_Option getOption() const { return option; };

    int getBestTabLength(int depth) override;
private:
    ProjectAudioAudioProcessor::DSP_Option option;
};

//==============================================================================
struct RotarySliderWithLabels;
//==============================================================================
struct DSP_GUI:juce::Component
{
    DSP_GUI(ProjectAudioAudioProcessor& p);

    void resized() override;

    void paint(juce::Graphics& g) override;

    void rebuildInterface(std::vector<juce::RangedAudioParameter*> params);

    ProjectAudioAudioProcessor& processor;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::ComboBox>> comboBoxes;
    std::vector<std::unique_ptr<juce::Button>> buttons;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboBoxAttachments;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;
};

//==============================================================================
/**
*/
class ProjectAudioAudioProcessorEditor  : public juce::AudioProcessorEditor,
    ExtendedTabbedButtonBar::Listener,
    juce::Timer
{
public:
    ProjectAudioAudioProcessorEditor (ProjectAudioAudioProcessor&);
    ~ProjectAudioAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void tabOrderChanged(ProjectAudioAudioProcessor::DSP_Order newOrder) override; //每次顺序改变都会触发
    void timerCallback() override;//定期会触发
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProjectAudioAudioProcessor& audioProcessor;
    DSP_GUI dspGUI{ audioProcessor };
    ExtendedTabbedButtonBar tabbedComponent;

    void addTabsFromDSPOrder(ProjectAudioAudioProcessor::DSP_Order order);
    void rebuildInterface(); //当增加tab或者移动tab时调用

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectAudioAudioProcessorEditor)
};

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
 * ����һ��ˮƽԼ����
 *
 * @param confinedBoundsGetter �ص�������������Ҫ��Լ����Ŀ���������
 *                             
 * @param confineeBoundsGetter �ص�����������Լ���ο�����
 *                             
 *
 * Լ���߼���Ŀ�������ˮƽλ�úͿ�Ƚ��������ڲο�������
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
     �Ϸ����������ʵ�ַ�ʽΪ���� TabbedButtonBar ͬʱ��Ϊ�Ϸ�Ŀ�꣨dragAndDrop Target�����Ϸ�������DragAndDropContainer����
     ������ÿ�� TabBarButton �ϵ�����¼���

     �������£�
     ��ѡ���tab���ϰ�����꣨mouseDown����������Ϸ�������DragAndDropContainer����ʼ��Ӧ����϶���mouseDrag���¼���
     ���ȣ�����һ������¼�����ʱ������� itemDragEnter ������
     ��������ƶ�������� itemDragMove() ������ѡ���ť��tabBarButtons�������� ExtendedTabbedButtonBar �ı߽磬���������Զ���ᱻ�ϳ��ñ߽硣

     itemDragMove() ���������϶�����Ŀ�� x ���꣬����������������Ŀ�� x ������бȽϡ�
     ���һ��ѡ�Խ������һ��ѡ����м�λ�ã��ͻύ����Щѡ���������
*/

    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;

    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;

    struct Listener {      //�󶨵�drag�¼�����ק�д���
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

    void mouseDown(const juce::MouseEvent& e) override;//��д���麯��������Ҫ���û���,��֤ԭ��������ʵ��

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
    void tabOrderChanged(ProjectAudioAudioProcessor::DSP_Order newOrder) override; //ÿ��˳��ı䶼�ᴥ��
    void timerCallback() override;//���ڻᴥ��
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ProjectAudioAudioProcessor& audioProcessor;
    DSP_GUI dspGUI{ audioProcessor };
    ExtendedTabbedButtonBar tabbedComponent;

    void addTabsFromDSPOrder(ProjectAudioAudioProcessor::DSP_Order order);
    void rebuildInterface(); //������tab�����ƶ�tabʱ����

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectAudioAudioProcessorEditor)
};

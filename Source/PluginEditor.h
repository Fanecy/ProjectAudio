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

    void mouseDown(const juce::MouseEvent& e) override;

    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;
};

struct ExtendedTabBarButton : juce::TabBarButton //make one draggable tab
{
    ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar);

    juce::ComponentDragger dragger;

    std::unique_ptr<HorizontalConstrainer> constrainer;

    void mouseDown(const juce::MouseEvent& e) override;//��д���麯��������Ҫ���û���,��֤ԭ��������ʵ��

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

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <RotarySliderWithLabels.h>
#include <Utilities.h>

static juce::String GetNameFromDspOption(ProjectAudioAudioProcessor::DSP_Option option)
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

static ProjectAudioAudioProcessor::DSP_Option GetDspOptionFromName(const juce::String& tabName)
{
    if (tabName == "PHASER") { return ProjectAudioAudioProcessor::DSP_Option::Phase; }
    else if (tabName == "CHORUS") {return ProjectAudioAudioProcessor::DSP_Option::Chorus;}
    else if (tabName == "OVERDRIVE") {return  ProjectAudioAudioProcessor::DSP_Option::Overdrive; }
    else if (tabName == "LADDERFILTER") { return ProjectAudioAudioProcessor::DSP_Option::LadderFilter; }
    else if (tabName == "GENERALFILTER") { return ProjectAudioAudioProcessor::DSP_Option::GeneralFilter; }
    return ProjectAudioAudioProcessor::DSP_Option::END_OF_LIST;
}

//==============================================================================
HorizontalConstrainer::HorizontalConstrainer(std::function<juce::Rectangle<int>()> confinedBoundsGetter, 
    std::function<juce::Rectangle<int>()> confineeBoundsGetter):
    boundsToConfineToGetter(std::move(confinedBoundsGetter)),
    boundsOfConfineeGetter(std::move(confineeBoundsGetter))
{

}

void HorizontalConstrainer::checkBounds(juce::Rectangle<int>& bounds, const juce::Rectangle<int>& previousBounds, 
    const juce::Rectangle<int>& limits, bool isStretchingTop, bool isStretchingLeft, bool isStretchingBottom, 
    bool isStretchingRight)
{
    /*
     'bounds' is the bounding box that we are TRYING to set componentToConfine to.
     we only want to support horizontal dragging within the TabButtonBar.

     so, retain the existing Y position given to the TabBarButton by the TabbedButtonBar when the button was created.
     */
    bounds.setY(previousBounds.getY());
    /*
     the X position needs to be limited to the left and right side of the owning TabbedButtonBar.
     however, to prevent the right side of the TabBarButton from being dragged outside the bounds of the TabbedButtonBar,
     we must subtract the width of this button from the right side of the TabbedButtonBar

     in order for this to work, we need to know the bounds of both the TabbedButtonBar and the TabBarButton.
     hence, loose coupling using lambda getter functions via the constructor parameters.
     Loose coupling is preferred vs tight coupling.
     */

    if (boundsToConfineToGetter != nullptr &&
        boundsOfConfineeGetter != nullptr)
    {
        auto boundsToConfineTo = boundsToConfineToGetter();
        auto boundsOfConfinee = boundsOfConfineeGetter();

        bounds.setX(juce::jlimit(boundsToConfineTo.getX(),
            boundsToConfineTo.getRight() - boundsOfConfinee.getWidth(),
            bounds.getX())); 
    }
    else
    {
        bounds.setX(juce::jlimit(limits.getX(),
            limits.getY(),
            bounds.getX()));
    }
}

//==============================================================================
ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar,
                                          ProjectAudioAudioProcessor::DSP_Option& dspoption) :
    juce::TabBarButton(name,ownerBar),option(dspoption)
{
    constrainer = std::make_unique<HorizontalConstrainer>([&ownerBar]() {
        return ownerBar.getLocalBounds();
        },
        [this]() {
            return this->getBounds();
        }); //set 2 getters
    
    constrainer->setMinimumOnscreenAmounts(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff);
    //Fane: set not allowed to move outside of the screen,0xffffffff is a large num in juce
}

//==============================================================================
ExtendedTabbedButtonBar::ExtendedTabbedButtonBar() :juce::TabbedButtonBar(juce::TabbedButtonBar::Orientation::TabsAtTop)
{
    /*
    * used to make the dragged image invisible,so there wont be 2 images
    */
    auto image = juce::Image(juce::Image::PixelFormat::SingleChannel, 1, 1, true);
    auto gfx = juce::Graphics(image);
    gfx.fillAll(juce::Colours::transparentBlack);
    draggedImage = juce::ScaledImage(image, 1.0);
}

//==============================================================================
bool ExtendedTabbedButtonBar::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    if (dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get())) //check if it's ExtendedTabbedButtonBar
    {
        return true;
    }

    return false;
}



//==============================================================================
void ExtendedTabbedButtonBar::itemDragEnter(const SourceDetails& dragSourceDetails) //enter drag
{
    DBG("ExtendedTabbedButtonBar::itemDragEnter");                 
    juce::DragAndDropTarget::itemDragEnter(dragSourceDetails);      //Fane:call base func
}

//==============================================================================
void ExtendedTabbedButtonBar::itemDropped(const SourceDetails& dragSourceDetails) //after item dropped
{
    DBG("ExtendedTabbedButtonBar::itemDropped");
    //find the dropped item,and lock the position in
    resized();//re-put all tabs in bar into position

    //FANE:notify of the new order
    auto tabs = getTabs();
    ProjectAudioAudioProcessor::DSP_Order newOrder;

    jassert(tabs.size() == newOrder.size());
    for (size_t i = 0; i < tabs.size(); i++)
    {
        auto tab = tabs[i];            //false????????
        if (auto* etbb = dynamic_cast<ExtendedTabBarButton*>(tab))
        {
            newOrder[i] = etbb->getOption();
        }
    }

    listeners.call([newOrder](Listener& l) {
        l.tabOrderChanged(newOrder);
        });

}

//==============================================================================
juce::TabBarButton* ExtendedTabbedButtonBar::findDraggedItem(const SourceDetails& dragSourceDetails)
{
    return getTabButton(findDraggedItemIndex(dragSourceDetails));
}

//==============================================================================
int ExtendedTabbedButtonBar::findDraggedItemIndex(const SourceDetails& dragSourceDetails)
{
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();

        auto idx = tabs.indexOf(tabButtonBeingDragged);

        return idx;
    }
    return -1;
}

//==============================================================================
juce::Array<juce::TabBarButton*> ExtendedTabbedButtonBar::getTabs()
{
    auto numTabs = getNumTabs();
    auto tabs = juce::Array<juce::TabBarButton*>();
    tabs.resize(numTabs);
    for (size_t i = 0; i < numTabs; i++)
    {
        tabs.getReference(i) = getTabButton(i);
    }       //get all tabs 
    return tabs;
}

//==============================================================================
void ExtendedTabbedButtonBar::itemDragMove(const SourceDetails& dragSourceDetails) // during moving
{
    DBG("ExtendedTabbedButtonBar::itemDragMove");
    
    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(dragSourceDetails.sourceComponent.get()))
    {
        auto tabs = getTabs();

        auto idx = findDraggedItemIndex(dragSourceDetails);
        if (idx == -1) //check if get index,if not,jassetfalse
        {
            DBG("Fail to find tab being dragged in list of tabs");
            jassertfalse;
            return;
        }

        //find the tab that tabButtonBeingDragged is colliding with.
        //it might be on the right 
        //it might be on the left
        //if it's on the right,
        //if tabBarBeingDragged's x is > [nextTab.getX() + nextTab.getWidth() * 0.5], swap their position
        auto previousTabIndex = idx - 1;
        auto nextTabIndex = idx + 1;
        auto previousTab = getTabButton(previousTabIndex);
        auto nextTab = getTabButton(nextTabIndex);
        /*
        If there is no previousTab, you are in the leftmost position
        else If there is no nextTab, you are in the right-most position
        Otherwise you are in the middle of all the tabs.
            If you are in the middle, you might be switching with the tab on your left, or the tab on
            your right.
        */
        auto centreX = tabButtonBeingDragged->getBounds().getCentreX();
        if (previousTab == nullptr && nextTab != nullptr) //most left
        {

            if (centreX > nextTab->getX())
            {
                moveTab(idx, nextTabIndex);
            }
        }
        else if (previousTab != nullptr && nextTab == nullptr) //most right
        {
            if (centreX < previousTab->getRight())
            {
                moveTab(idx, previousTabIndex);
            }
        }
        else //middle
        {
            if (centreX > nextTab->getX())
            {
                moveTab(idx, nextTabIndex);
            }

            else if (centreX < previousTab->getRight())
            {
                moveTab(idx, previousTabIndex);
            }
        }
        tabButtonBeingDragged->toFront(true);
    }
}

//==============================================================================
void ExtendedTabbedButtonBar::itemDragExit(const SourceDetails& dragSourceDetails) // end drag
{
    DBG("ExtendedTabbedButtonBar::itemDragExit");
    juce::DragAndDropTarget::itemDragExit(dragSourceDetails);
}

//==============================================================================
void ExtendedTabBarButton::mouseDown(const juce::MouseEvent& e)
{
    toFront(true);//make sure tab is in front during dragging
    dragger.startDraggingComponent(this, e);
    juce::TabBarButton::mouseDown(e); 
}

//==============================================================================
void ExtendedTabBarButton::mouseDrag(const juce::MouseEvent& e)
{
    dragger.dragComponent(this, e, constrainer.get());
}

int ExtendedTabBarButton::getBestTabLength(int depth)
{
    /*
    we want the tabs to occupy the entire TabBar width.
    so, after computing the best width for the button and depth,
    we choose whichever value is bigger, the bestWidth, or an equal division of the bar's width based on
    the number of tabs in the bar.
    */
    auto BestWidth = getLookAndFeel().getTabButtonBestWidth(*this,depth);
    auto& tabBar = getTabbedButtonBar();
    return juce::jmax(BestWidth,tabBar.getWidth() / tabBar.getNumTabs());
}

//==============================================================================
void ExtendedTabbedButtonBar::addListener(Listener* l)
{
    listeners.add(l);
}

void ExtendedTabbedButtonBar::removeListener(Listener* l)
{
    listeners.remove(l);
}

//==============================================================================
void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("etbb MouseDown");

    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.eventComponent))
    {
        startDragging(tabButtonBeingDragged->getTabbedButtonBar().getTitle(), tabButtonBeingDragged,draggedImage);
    }
}

//==============================================================================
juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto dspOption = GetDspOptionFromName(tabName);
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this, dspOption);
    etbb->addMouseListener(this, false);
    return etbb.release();   // return a naked ptr
}

//==============================================================================
ProjectAudioAudioProcessorEditor::ProjectAudioAudioProcessorEditor (ProjectAudioAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(tabbedComponent);
    addAndMakeVisible(dspGUI);

    tabbedComponent.addListener(this);
    startTimerHz(30);//call timerCallback() 30 times pre s
    setSize (600, 400);
}

ProjectAudioAudioProcessorEditor::~ProjectAudioAudioProcessorEditor()
{
    tabbedComponent.removeListener(this);
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
    bounds.removeFromTop(10);
    tabbedComponent.setBounds(bounds.removeFromTop(30));
    dspGUI.setBounds(bounds);
}

void ProjectAudioAudioProcessorEditor::tabOrderChanged(ProjectAudioAudioProcessor::DSP_Order newOrder)
{
    rebuildInterface();
    audioProcessor.dsporderFifo.push(newOrder);
}

void ProjectAudioAudioProcessorEditor::timerCallback() //used to store and call dsp order
{
    if (audioProcessor.storedDspOrderFifo.getNumAvailableForReading() == 0) //stored empty
    {
        return;
    }

    using T = ProjectAudioAudioProcessor::DSP_Order;
    T newOrder;
    newOrder.fill(ProjectAudioAudioProcessor::DSP_Option::END_OF_LIST);
    auto empty = newOrder;
    while (audioProcessor.storedDspOrderFifo.pull(newOrder))
    {
      ; //do nothing now
    }
    if (empty != newOrder)//dont create tabs if empty
    {
        addTabsFromDSPOrder(newOrder); 
    }
}

void ProjectAudioAudioProcessorEditor::addTabsFromDSPOrder(ProjectAudioAudioProcessor::DSP_Order order)
{
    tabbedComponent.clearTabs();
    for (auto v : order)
    {
        tabbedComponent.addTab(GetNameFromDspOption(v),juce::Colours::white,-1);
    }
    rebuildInterface();
    audioProcessor.dsporderFifo.push(order);
}

void ProjectAudioAudioProcessorEditor::rebuildInterface()
{
    auto currentTabIndex = tabbedComponent.getCurrentTabIndex();
    auto currentTab = tabbedComponent.getTabButton(currentTabIndex);

    if (auto etbb = dynamic_cast<ExtendedTabBarButton*>(currentTab))
    {
        auto option = etbb->getOption();
        auto params = audioProcessor.GetParamsForOption(option);

        jassert(!params.empty());
        dspGUI.rebuildInterface(params);                              
    }

    
}

DSP_GUI::DSP_GUI(ProjectAudioAudioProcessor& p) : processor(p)
{

}

void DSP_GUI::resized()
{
    auto bounds = getBounds();

    if (!buttons.empty())  //set button size
    {
        auto buttonArea = bounds.removeFromTop(30);
        auto w = buttonArea.getWidth() / buttons.size();
        for (auto& btn : buttons)
        {
            btn->setBounds(buttonArea.removeFromLeft(static_cast<int>(w)));
        }
    }

    if (!comboBoxes.empty())  //set comboBox size
    {
        auto boxArea = bounds.removeFromLeft(bounds.getWidth() / 4);
        auto h = juce::jmin(static_cast<int>(boxArea.getHeight() / comboBoxes.size()),30);
        for (auto& b : comboBoxes)
        {
            b->setBounds(boxArea.removeFromTop(static_cast<int>(h)));
        }
    }

    if (!sliders.empty())  //set slider size
    {
        auto w = bounds.getWidth() / sliders.size();
        for (auto& slider : sliders)
        {
            slider->setBounds(bounds.removeFromLeft(static_cast<int>(w)));
        }
    }



}

void DSP_GUI::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void DSP_GUI::rebuildInterface(std::vector<juce::RangedAudioParameter*> params)
{
    //when rebuild,first clear components from before
    sliderAttachments.clear();
    comboBoxAttachments.clear();
    buttonAttachments.clear();

    sliders.clear();
    comboBoxes.clear();
    buttons.clear();

    for (size_t i = 0; i < params.size(); i++)
    {
        auto p = params[i];

        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(p)) // make box
        {
            comboBoxes.push_back(std::make_unique<juce::ComboBox>());
            auto& cb = *comboBoxes.back();
            cb.addItemList(choice->choices,1); //get all choices

            comboBoxAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>
                (processor.apvts,p->getName(100), cb));//attach box with a param
        }
        else if (auto* toggle = dynamic_cast<juce::AudioParameterBool*>(p)) //make toggle
        {
            buttons.push_back(std::make_unique<juce::ToggleButton>("Bypass"));
            auto& btn = *buttons.back();
            
            buttonAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>
                (processor.apvts,p->getName(100),btn));
        }
        else //make sliders(AudioParameterFloat or AudioParameterInt)
        {
            sliders.push_back(std::make_unique<RotarySliderWithLabels>(p,p->label,p->getName(100)));
            auto& slider = *sliders.back();
            SimpleMBComp::addLabelPairs(slider.labels, *p, p->label);
            slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
            sliderAttachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>
                (processor.apvts,p->getName(100),slider));
        }
    }

    for (auto& box : comboBoxes) //add components on viewport 
    {
        addAndMakeVisible(box.get());
    }

    for (auto& button : buttons)
    {
        addAndMakeVisible(button.get());
    }

    for (auto& slider : sliders)
    {
        addAndMakeVisible(slider.get());
    }

    resized(); 
}

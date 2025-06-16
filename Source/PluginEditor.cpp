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
ExtendedTabBarButton::ExtendedTabBarButton(const juce::String& name, juce::TabbedButtonBar& ownerBar): 
    juce::TabBarButton(name,ownerBar)
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
        if (previousTab == nullptr && nextTab != nullptr) //most left
        {

            if (tabButtonBeingDragged->getX() > nextTab->getBounds().getCentreX())
            {
                moveTab(idx, nextTabIndex);
            }
        }
        else if (previousTab != nullptr && nextTab == nullptr) //most right
        {
            if (tabButtonBeingDragged->getX() < previousTab->getBounds().getCentreX())
            {
                moveTab(idx, previousTabIndex);
            }
        }
        else //middle
        {
            if (tabButtonBeingDragged->getX() > nextTab->getBounds().getCentreX())
            {
                moveTab(idx, nextTabIndex);
            }

            else if (tabButtonBeingDragged->getX() < previousTab->getBounds().getCentreX())
            {
                moveTab(idx, previousTabIndex);
            }
        }
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

//==============================================================================
void ExtendedTabbedButtonBar::mouseDown(const juce::MouseEvent& e)
{
    DBG("etbb MouseDown");

    if (auto tabButtonBeingDragged = dynamic_cast<ExtendedTabBarButton*>(e.eventComponent))
    {
        startDragging(tabButtonBeingDragged->getTabbedButtonBar().getTitle(), tabButtonBeingDragged);
    }
}

//==============================================================================
juce::TabBarButton* ExtendedTabbedButtonBar::createTabButton(const juce::String& tabName, int tabIndex)
{
    auto etbb = std::make_unique<ExtendedTabBarButton>(tabName, *this);
    etbb->addMouseListener(this, false);
    return etbb.release();   // return a naked ptr
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



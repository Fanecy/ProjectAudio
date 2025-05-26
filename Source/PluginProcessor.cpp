/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//** PhaserPramsNameFunc**//
auto getPhaserRateName() { return juce::String("Phaser RateHz"); }
auto getPhaserCenterFreqName() { return juce::String("Phaser Center FreqHz");}
auto getPhaserDepthName() { return juce::String("Phaser Depth %"); }
auto getPhaserFeedbackName() { return juce::String("Phaser Feedback %"); }
auto getPhaserMixName() { return juce::String("Phaser Mix %"); }

//** ChorusPramsNameFunc**//
auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay Ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusrMixName() { return juce::String("Chorus Mix %"); }

//** OverDrivePramsNameFunc**//
auto getOverDriveSaturtationName() { return juce::String("OverDrive Saturation"); }

//==============================================================================
ProjectAudioAudioProcessor::ProjectAudioAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    )
#endif
{
    //***********************************PramsPointers and NameFuncPointers and Init*********************************//
    auto floatParams = std::array{         //floatPrams pointers

        //Phaser
        &PhaserRateHz,
        &PhaserDepthPercent,
        &PhaserCenterFreqHz,
        &PhaserFeedbackPercet,
        &PhaserMixPercent,

        //Chorus
        &ChorusRateHz,
        &ChorusDepthPercent,
        &ChorusCenterDelayMs,
        &ChorusFeedbackPercet,
        &ChorusMixPercent,

        //OverDrive
        &OverDriveSaturation
    };

    auto floatNameFuncs = std::array{          //floatNameFuncs pointers
        //phaser
        &getPhaserRateName,
        &getPhaserDepthName,
        &getPhaserCenterFreqName,
        &getPhaserFeedbackName,
        &getPhaserMixName,

        //chorus
        &getChorusRateName,
        &getChorusDepthName,
        &getChorusCenterDelayName,
        &getChorusFeedbackName,
        &getChorusrMixName,

        //overdrive
        &getOverDriveSaturtationName
    };

    for (size_t i = 0; i < floatParams.size(); i++)
    {
        auto ptrToParamPtr = floatParams[i];

        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(floatNameFuncs[i]()));

        jassert(*ptrToParamPtr != nullptr);
    }

    //***********************************PramsPointers and NameFuncPointers and Init*********************************//
}

    




ProjectAudioAudioProcessor::~ProjectAudioAudioProcessor()
{
}

//==============================================================================
const juce::String ProjectAudioAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ProjectAudioAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ProjectAudioAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ProjectAudioAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ProjectAudioAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ProjectAudioAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ProjectAudioAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ProjectAudioAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ProjectAudioAudioProcessor::getProgramName (int index)
{
    return {};
}

void ProjectAudioAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ProjectAudioAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ProjectAudioAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ProjectAudioAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif  

juce::AudioProcessorValueTreeState::ParameterLayout ProjectAudioAudioProcessor::createParameterLayout() //Fane:createPrameterLayout
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    // false code on adding parameter to layout:
    // layout.add(std::unique_ptr<juce::AudioParameterFloat>
    // juce::parameterID(name,VersionHint),   VersionHint is used to get old plugins to work
    // Name,
    // parameterRange,
    // defaultvalue,
    // unitSuffix
    // );
    const int versionHint = 1;

     /*
      Phaser:
      Rate: hz
      Depth(percent): 0 to 1
      Center Freq: hz
      Feedback(percent): -1 to 1
      Mix(percent): 0 to 1
    */
    auto name = getPhaserRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(   //added PhaserRate
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
        0.2f,
        "Hz"
    ));

    //*****************************************************************************************************//
    name = getPhaserDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(   //added PhaserDepth
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"
    ));

    //*****************************************************************************************************//

    name = getPhaserCenterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(   //added PhaserCenterFreq
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 2.f, 0.01f, 1.f),
        0.2f,
        "Hz"
    ));

    //*****************************************************************************************************//

    name = getPhaserFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(   //added PhaserFeedback
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"
    ));

    //*****************************************************************************************************//

    name = getPhaserMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(   //added PhaserMix
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"
    ));

    //*****************************************************************************************************//
    //=====================================================================================================//
     /*
      Chorus:
      Rate: hz
      Depth(percent): 0 to 1
      Center Delay: 1 to 100ms
      Feedback(percent): -1 to 1
      Mix(percent): 0 to 1
    */
    name = getChorusRateName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 100.f, 0.01f, 1.f),
        0.2f,
        "Hz"
    ));

    //*****************************************************************************************************//

    name = getChorusDepthName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"
    ));

    //*****************************************************************************************************//
    name = getChorusCenterDelayName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(1.f, 100.f, 0.1f, 1.f),
        7.f,
        "%"
    ));

    //*****************************************************************************************************//
    name = getChorusFeedbackName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(-1.f, 1.f, 0.01f, 1.f),
        0.0f,
        "%"
    ));

    //*****************************************************************************************************//
    name = getChorusrMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"
    ));

    //*****************************************************************************************************//
    //=====================================================================================================//
     /*
      OverDrive:
      drive:1-100
    */
    name = getOverDriveSaturtationName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{name,versionHint},
        name,
        juce::NormalisableRange(1.f,100.f,0.1f,1.f),
        1.f,
        ""
    ));




    return layout;
}

void ProjectAudioAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) //Fane£ºused to init fifo
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto newDSPOrder = DSP_Order();
    //try pull FIFO
    while (dsporderFifo.pull(newDSPOrder))
    {

    }

    //once pull replace dsporder 
    if (newDSPOrder != DSP_Order())
    {
        dsporder = newDSPOrder;
    }

    //fill pointers
    DSP_Pointers dspPointers;

    for (size_t i = 0; i < dspPointers.size(); i++)
    {
        switch (dsporder[i])
        {
        case DSP_Option::Phase:
            dspPointers[i] = &phaser;
            break;

        case DSP_Option::Chorus:
            dspPointers[i] = &chorus;
            break;

        case DSP_Option::Overdrive:
            dspPointers[i] = &overdrive;
            break;

        case DSP_Option::LadderFilter:
            dspPointers[i] = &ladderFilter;
            break;

        case DSP_Option::END_OF_LIST:
            jassertfalse;
            break;

        default:

            break;
        }
    }
    
    //now process
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    for (size_t i = 0; i < dspPointers.size(); i++)
    {
        if (dspPointers[i] != nullptr)
        {
            dspPointers[i]->process(context);
        }
    }
}

//==============================================================================
bool ProjectAudioAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ProjectAudioAudioProcessor::createEditor()
{
    return new ProjectAudioAudioProcessorEditor (*this);
}

//==============================================================================
void ProjectAudioAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ProjectAudioAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProjectAudioAudioProcessor();
}

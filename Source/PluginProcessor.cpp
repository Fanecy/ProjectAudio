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
auto getPhaserBypassName() { return juce::String("Phaser Bypass"); }

//** ChorusPramsNameFunc**//
auto getChorusRateName() { return juce::String("Chorus RateHz"); }
auto getChorusDepthName() { return juce::String("Chorus Depth %"); }
auto getChorusCenterDelayName() { return juce::String("Chorus Center Delay Ms"); }
auto getChorusFeedbackName() { return juce::String("Chorus Feedback %"); }
auto getChorusMixName() { return juce::String("Chorus Mix %"); }
auto getChorusBypassName() { return juce::String("Chorus Bypass"); }

//** OverDrivePramsNameFunc**//
auto getOverDriveSaturtationName() { return juce::String("OverDrive Saturation"); }
auto getOverdriveBypassName() { return juce::String("Overdrive Bypass"); }

//** LadderFilterPramsNameFunc**//
auto getLadderFilterModeName() { return juce::String("Ladder Filter Mode"); }
auto getLadderFilterCutoffFrequencyName() { return juce::String("Ladder Filter Cutoff Hz"); }
auto getLadderFilterResonanceName() { return juce::String("Ladder Filter Resonance"); }
auto getLadderFilterDriveName() { return juce::String("Ladder Filter Drive"); }
auto getLadderFilterBypassName() { return juce::String("Ladder Filter Bypass"); }

auto getLadderFilterChoice() {
    return juce::StringArray{
    "LPF12",  // low-pass  12 dB/octave
    "HPF12",  // high-pass 12 dB/octave
    "BPF12",  // band-pass 12 dB/octave
    "LPF24",  // low-pass  24 dB/octave
    "HPF24",  // high-pass 24 dB/octave
    "BPF24"   // band-pass 24 dB/octave
    };
}

//** GeneralFilterPramsNameFunc**//
auto getGeneralFilterChoice() {
    return juce::StringArray{
        "Peak",
        "bandpass",
        "notch",
        "allpass",
    };
}

auto getGeneralFilterModeName() { return juce::String("General Filter Mode"); }
auto getGeneralFilterFreqName() { return juce::String("General Filter Freq Hz"); }
auto getGeneralFilterQualityName() { return juce::String("General Filter Quality"); }
auto getGeneralFilterGainName() { return juce::String("General Filter Gain"); }
auto getGeneralFilterBypassName() { return juce::String("General Filter Bypass"); }





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
    dsporder =
    { {
            DSP_Option::Phase,
            DSP_Option::Chorus,
            DSP_Option::Overdrive,
            DSP_Option::LadderFilter
        } };




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
        &OverDriveSaturation,

        //LadderFilter
        &LadderFilterCutoffHz,
        &LadderFilterResonance,
        &LadderFilterDrive,

        //GeneralFilter
        &GeneralFilterFreqHz,
        &GeneralFilterQuality,
        &GeneralFilterGain,
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
        &getChorusMixName,

        //overdrive
        &getOverDriveSaturtationName,

        //LadderFilter
        &getLadderFilterCutoffFrequencyName,
        &getLadderFilterResonanceName,
        &getLadderFilterDriveName,

        //GeneralFilter
        &getGeneralFilterFreqName,
        &getGeneralFilterQualityName,
        &getGeneralFilterGainName
    };

    /*for (size_t i = 0; i < floatParams.size(); i++)
    {
        auto ptrToParamPtr = floatParams[i];

        *ptrToParamPtr = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(floatNameFuncs[i]()));

        jassert(*ptrToParamPtr != nullptr);
    }*/

    initCachedPtrParam<juce::AudioParameterFloat*>(floatParams, floatNameFuncs);

    //***********************************PramsPointers and NameFuncPointers and Init*********************************//

    //Add FilterMode Pointers

    auto choiceParams = std::array{
        &LadderFilterMode,
        &GeneralFilterMode
    };

    auto choiceNameFuncs = std::array{
        &getLadderFilterModeName,
        &getGeneralFilterModeName
    };

    /*for (size_t i = 0; i < choiceParams.size(); i++)
    {
        auto ptrToParams = choiceParams[i];

        *ptrToParams = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(choiceNameFuncs[i]()));

        jassert(*ptrToParams != nullptr);
    }*/

    initCachedPtrParam<juce::AudioParameterChoice*>(choiceParams, choiceNameFuncs);
    
    //Add Bypass Pointers
    auto bypassParams = std::array{
        &PhaserBypass,
        &ChorusBypass,
        &OverDriveBypass,
        &LadderFilterBypass,
        &GeneralFilterBypass
    };

    auto bypassNameFuncs = std::array{
        &getPhaserBypassName,
        &getChorusBypassName,
        &getOverdriveBypassName,
        &getLadderFilterBypassName,
        &getGeneralFilterBypassName
    };

    //FANE:OLD CODE
    //for (size_t i = 0; i < bypassParams.size(); i++)
    //{
    //   auto ptrToParams = bypassParams[i];

    //    *ptrToParams = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(bypassNameFuncs[i]()));

    //    jassert(*ptrToParams != nullptr);
    //}

    initCachedPtrParam<juce::AudioParameterBool*>(bypassParams, bypassNameFuncs);
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
void ProjectAudioAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) //Fane:Happen before play,init
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


    // Fane: prepare all DSP
    juce::dsp::ProcessSpec Spec;
    Spec.sampleRate = sampleRate;
    Spec.maximumBlockSize = samplesPerBlock;
    Spec.numChannels = getTotalNumInputChannels();

    std::vector<juce::dsp::ProcessorBase*> dps
    {
        &phaser,
        &chorus,
        &overdrive,
        &ladderFilter,
        &generalFilter
    };

    for (auto p : dps )
    {
        p->prepare(Spec);
        p->reset();
    }
    // Fane:  prepare all DSP
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
    
    name = getPhaserBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ name,versionHint },
        name,
        false
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
    name = getChorusMixName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange<float>(0.01f, 1.f, 0.01f, 1.f),
        0.05f,
        "%"
    ));

    //*****************************************************************************************************//
    name = getChorusBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ name,versionHint },
        name,
        false
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
    //*****************************************************************************************************//
    name = getOverdriveBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ name,versionHint },
        name,
        false
    ));

    //*****************************************************************************************************//
    //=====================================================================================================//
     /*
    ladder filter:
    mode: LadderFilterMode enum(int)
    cutoff:hz
    resonance: 0 to 1
    drive: 1 - 100
    */

    name = getLadderFilterModeName();
    auto choices = getLadderFilterChoice();

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ name,versionHint },
        name,
        choices,
        0
    ));
    

    //*****************************************************************************************************//
    name = getLadderFilterCutoffFrequencyName();

   layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(20.f, 20000.f, 0.1f, 1.f),
        20000.f,
        "Hz"
    ));
    

    //*****************************************************************************************************//
    name = getLadderFilterResonanceName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(0.f, 1.f, 0.01f, 1.f),
        0.f,
        ""
    ));
    //*****************************************************************************************************//
    name = getLadderFilterDriveName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(1.f, 100.f, 0.1f, 1.f),
        1.f,
        ""
    ));

    //*****************************************************************************************************//
    name = getLadderFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ name,versionHint },
        name,
        false
    ));
    //*****************************************************************************************************//
    //=====================================================================================================//

    /*
    * general filter:IIRFilter
    * Mode: Peak,bandpass,notch,allpass,
    * freq:20hz - 20,000hz in 1hz steps
    * Q: 0.1 - 10 in 0.05 steps
    * gain: -24db to +24db in 0.5db increments
    */
    name = getGeneralFilterModeName();
    choices = getGeneralFilterChoice();

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID{ name,versionHint },
        name,
        choices,
        0
    ));
    //*****************************************************************************************************//
    name = getGeneralFilterFreqName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(20.f, 20000.f, 1.f, 1.f),
        750.f,
        "Hz"
    ));
    //*****************************************************************************************************//
    name = getGeneralFilterQualityName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(0.1f, 10.f, 0.05f,  1.f),
        1.f,
        ""
    ));
    //*****************************************************************************************************//
    name = getGeneralFilterGainName();
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{ name,versionHint },
        name,
        juce::NormalisableRange(-24.f, 24.f, 0.5f, 1.f),
        0.f,
        "dB"
    ));
    //*****************************************************************************************************//

    name = getGeneralFilterBypassName();
    layout.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{ name,versionHint },
        name,
        false
    ));

    //*****************************************************************************************************//

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

    //save/load parameters for each dspOption
    // 
    //phaser
    phaser.dsp.setRate(PhaserRateHz->get());
    phaser.dsp.setDepth(PhaserDepthPercent->get());
    phaser.dsp.setCentreFrequency(PhaserCenterFreqHz->get());
    phaser.dsp.setFeedback(PhaserFeedbackPercet->get());
    phaser.dsp.setMix(PhaserMixPercent->get());

    //chorus
    chorus.dsp.setRate(ChorusRateHz->get());
    chorus.dsp.setDepth(ChorusDepthPercent->get());
    chorus.dsp.setCentreDelay(ChorusCenterDelayMs->get());
    chorus.dsp.setFeedback(ChorusFeedbackPercet->get());
    chorus.dsp.setMix(ChorusMixPercent->get());

    //overdrive
    overdrive.dsp.setDrive(OverDriveSaturation->get());

    //ladderfilter
    ladderFilter.dsp.setMode(static_cast<juce::dsp::LadderFilterMode>(LadderFilterMode->getIndex()));
    ladderFilter.dsp.setCutoffFrequencyHz(LadderFilterCutoffHz->get());
    ladderFilter.dsp.setDrive(LadderFilterDrive->get());
    ladderFilter.dsp.setResonance(LadderFilterResonance->get());

    //save/load parameters for each dspOption
   

    auto newDSPOrder = DSP_Order();
    //try pull FIFO
    while (dsporderFifo.pull(newDSPOrder))
    {
#if VERYFY_BYPASS_FUNCTIONALITY
        jassertfalse;
#endif
    }

    //once pull replace dsporder 
    if (newDSPOrder != DSP_Order())
    {
        dsporder = newDSPOrder;
    }

    //fill pointers
    DSP_Pointers dspPointers;
    dspPointers.fill({});                                     //oldversion: dspPointers.fill(nullptr);

    for (size_t i = 0; i < dspPointers.size(); ++i)
    {
        switch (dsporder[i])
        {
        case DSP_Option::Phase:
            dspPointers[i].Processor = &phaser;
            dspPointers[i].bypassed = PhaserBypass->get();
            break;

        case DSP_Option::Chorus:
            dspPointers[i].Processor = &chorus;
            dspPointers[i].bypassed = ChorusBypass->get();
            break;

        case DSP_Option::Overdrive:
            dspPointers[i].Processor = &overdrive;
            dspPointers[i].bypassed = OverDriveBypass->get();
            break;

        case DSP_Option::LadderFilter:
            dspPointers[i].Processor = &ladderFilter;
            dspPointers[i].bypassed = LadderFilterBypass->get();
            break;

        case DSP_Option::GeneralFilter:
            dspPointers[i].Processor = &generalFilter;
            dspPointers[i].bypassed = GeneralFilterBypass->get();
            break;

        case DSP_Option::END_OF_LIST:
            jassertfalse;
            break;

        default:
        {
            dspPointers[i] = {};
            break;
        }
        }
    }

        //now process
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto context = juce::dsp::ProcessContextReplacing<float>(block);

        for (size_t i = 0; i < dspPointers.size(); ++i)
        {
            if (dspPointers[i].Processor != nullptr)
            {
                juce::ScopedValueSetter<bool> svs(context.isBypassed, dspPointers[i].bypassed); //reset bypass

#if VERYFY_BYPASS_FUNCTIONALITY
                if (context.isBypassed)
                {
                    jassertfalse;
                }
                if (dspPointers[i].Processor == &generalFilter)
                {
                    continue;
                }
#endif
                
                dspPointers[i].Processor->process(context);
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
    //return new ProjectAudioAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//Fane:Used to convert var to DSP_Order and convert DSP_Order to var
template<>      
struct juce::VariantConverter <ProjectAudioAudioProcessor::DSP_Order>
{
    static ProjectAudioAudioProcessor::DSP_Order fromVar(const juce::var& v) 
    {
        using T = ProjectAudioAudioProcessor::DSP_Order;
        T dspOrder;

        jassert(v.isBinaryData());

        if (v.isBinaryData() == false)
        {
            dspOrder.fill(ProjectAudioAudioProcessor::DSP_Option::END_OF_LIST);
        }
        else
        {
            auto mb = *v.getBinaryData();
            juce::MemoryInputStream mis(mb, false);

            std::vector<int> arr;
            while (!mis.isExhausted())
            {
                arr.push_back(mis.readInt());
            }

            jassert(arr.size() == dspOrder.size());
            

            for (size_t i = 0; i < dspOrder.size(); i++)
            {
                dspOrder[i] = static_cast<ProjectAudioAudioProcessor::DSP_Option>(arr[i]);
            }
            
        }
        return dspOrder;
    }

    static juce::var toVar(const ProjectAudioAudioProcessor::DSP_Order& d)
    {
        juce::MemoryBlock mb;

        {//scoping to boost efficiency,now mos will be gone sooner
            juce::MemoryOutputStream mos(mb, false);

            for (const auto& v : d)
            {
                mos.writeInt(static_cast<int>(v));
            }
        }//also,now it is writing to mb correctly

        return mb;
    }
};


//==============================================================================
void ProjectAudioAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{//FANE:The function is used to save the old state
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    apvts.state.setProperty("dpsOrder",
        juce::VariantConverter<ProjectAudioAudioProcessor::DSP_Order>::toVar(dsporder),
        nullptr);//Fane:set dpsOrder for apvts

    juce::MemoryOutputStream mos(destData, false); //FANE:Memory output stream that allow to write into destData
    apvts.state.writeToStream(mos); //FANE: write the state into memoryBlock
}

void ProjectAudioAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{//FANE:AND THIS IS USED TO SET THE STATE LIKE THE OLD ONE
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes); //Fane:get old state

    if (tree.isValid())
    {
        apvts.replaceState(tree);  //Fane:set the state like the old one

        if (apvts.state.hasProperty("dpsOrder"))
        {
            auto order = juce::VariantConverter<ProjectAudioAudioProcessor::DSP_Order>::fromVar(apvts.state.getProperty("dpsOrder")); 
            //Fane:get dpsOrder from apvts
        }

        DBG(apvts.state.toXmlString());


#if VERYFY_BYPASS_FUNCTIONALITY //test Bypass
        juce::Timer::callAfterDelay(1000, [this]() {
            DSP_Order order;
            order.fill(DSP_Option::LadderFilter);
            order[0] = DSP_Option::Chorus;
            ChorusBypass->setValueNotifyingHost(1.f);
            dsporderFifo.push(order);
            });
#endif

    }
}



//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ProjectAudioAudioProcessor();
}

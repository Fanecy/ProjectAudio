/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <Fifo.h>

//==============================================================================
/**
*/
class ProjectAudioAudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ProjectAudioAudioProcessor();
    ~ProjectAudioAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    enum class DSP_Option //选项ENUM
    {
        Phase,
        Chorus,
        Overdrive,
        LadderFilter,
        GeneralFilter,
        END_OF_LIST
    };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts{ *this,nullptr,"Settings",createParameterLayout() };//Create apvats

    using DSP_Order = std::array<DSP_Option, static_cast<size_t>(DSP_Option::END_OF_LIST)>; //用类型别名替代后面一坨Array

    SimpleMBComp::Fifo<DSP_Order> dsporderFifo; //Fifo

    /*
      Phaser:
      Rate: hz
      Depth(percent): 0 to 1
      Center Freq: hz
      Feedback(percent): -1 to 1
      Mix(percent): 0 to 1
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterFloat* PhaserRateHz = nullptr;
    juce::AudioParameterFloat* PhaserDepthPercent = nullptr;
    juce::AudioParameterFloat* PhaserCenterFreqHz = nullptr;
    juce::AudioParameterFloat* PhaserFeedbackPercet = nullptr;
    juce::AudioParameterFloat* PhaserMixPercent = nullptr;
    juce::AudioParameterBool*  PhaserBypass = nullptr;
    //** added pointers for cached parameters above **//

     /*
      Chorus:
      Rate: hz
      Depth(percent): 0 to 1
      Center Delay: 1 to 100ms
      Feedback(percent): -1 to 1
      Mix(percent): 0 to 1
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterFloat* ChorusRateHz = nullptr;
    juce::AudioParameterFloat* ChorusDepthPercent = nullptr;
    juce::AudioParameterFloat* ChorusCenterDelayMs = nullptr;
    juce::AudioParameterFloat* ChorusFeedbackPercet = nullptr;
    juce::AudioParameterFloat* ChorusMixPercent = nullptr;
    juce::AudioParameterBool*  ChorusBypass = nullptr;
    //** added pointers for cached parameters above **//


     /*
      OverDrive:
      drive:1-100
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterFloat* OverDriveSaturation = nullptr;
    juce::AudioParameterBool*  OverDriveBypass = nullptr;
    //** added pointers for cached parameters above **//

    /*
    ladder filter:
    mode: LadderFilterMode enum(int)
    cutoff:hz
    resonance: 0 to 1
    drive: 1 - 100
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterChoice* LadderFilterMode = nullptr;
    juce::AudioParameterFloat* LadderFilterCutoffHz = nullptr;
    juce::AudioParameterFloat* LadderFilterResonance = nullptr;
    juce::AudioParameterFloat* LadderFilterDrive = nullptr;
    juce::AudioParameterBool*  LadderFilterBypass = nullptr;
    //** added pointers for cached parameters above **//

     /*
    * general filter:IIRFilter
    * Mode: Peak,bandpass,notch,allpass,
    * freq:20hz - 20,000hz in 1hz steps
    * Q: 0.1 - 10 in 0.05 steps
    * gain: -24db to +24db in 0.5db increments
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterChoice* GeneralFilterMode = nullptr;
    juce::AudioParameterFloat* GeneralFilterFreqHz = nullptr;
    juce::AudioParameterFloat* GeneralFilterQuality = nullptr;
    juce::AudioParameterFloat* GeneralFilterGain = nullptr;
    juce::AudioParameterBool* GeneralFilterBypass = nullptr;
     //** added pointers for cached parameters above **//

private:
    DSP_Order dsporder;

    template<typename DSP>
    struct DSP_Choice : juce::dsp::ProcessorBase  //模板继承ProcessorBase
    {
        void prepare(const juce::dsp::ProcessSpec& spec) override
        {
            dsp.prepare(spec);
        }

        void process(const juce::dsp::ProcessContextReplacing<float>& context) override
        {
            dsp.process(context);
        }

        void reset() override
        {
            dsp.reset();
        }

        DSP dsp;
    };
    
    /*Wrap dspChoice into MonoChannel*/
    struct MonoChannelDSP {                                                        
        MonoChannelDSP(ProjectAudioAudioProcessor& proc) : p(proc) {}; //init ProjectAudioAudioProcessor

        DSP_Choice<juce::dsp::DelayLine<float>> delaytime;
        DSP_Choice<juce::dsp::Phaser<float>> phaser;
        DSP_Choice<juce::dsp::Chorus<float>> chorus;
        DSP_Choice<juce::dsp::LadderFilter<float>> overdrive, ladderFilter;
        DSP_Choice<juce::dsp::IIR::Filter<float>> generalFilter;

        void Prepare(const juce::dsp::ProcessSpec& spec);

        void UpdateDSPfromParams();
        
        void Process(juce::dsp::AudioBlock<float> block, const DSP_Order& dsporder);
    private:
        ProjectAudioAudioProcessor& p;
    };

    MonoChannelDSP leftChannel{ *this };  //set 2 instances
    MonoChannelDSP rightChannel{ *this };
    /*Wrap dspChoice into MonoChannel*/

    struct ProcessState {
        juce::dsp::ProcessorBase* Processor;
        bool bypassed = false;
    };
    using DSP_Pointers = std::array<ProcessState, static_cast<size_t>(DSP_Option::END_OF_LIST)>;//用别名替代指针数组

#define VERYFY_BYPASS_FUNCTIONALITY false // Fane:Macro to test Bypass
    
    template <typename ParamType,typename Params,typename Funcs>
    void initCachedPtrParam(Params paramArray,Funcs funcArray )   //Fane:Template Used to init cachedPtrParams
    {
        for (size_t i = 0; i < paramArray.size(); ++i)
        {
            auto ptrToParamsPtr = paramArray[i];
            *ptrToParamsPtr = dynamic_cast<ParamType>(apvts.getParameter(funcArray[i]()));
            jassert(*ptrToParamsPtr != nullptr);
        };
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectAudioAudioProcessor)
};



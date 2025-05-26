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
    //** added pointers for cached parameters above **//


     /*
      OverDrive:
      drive:1-100
    */

    //** added pointers for cached parameters above **//
    juce::AudioParameterFloat* OverDriveSaturation = nullptr;
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
    
    DSP_Choice<juce::dsp::DelayLine<float>> delaytime;
    DSP_Choice<juce::dsp::Phaser<float>> phaser;
    DSP_Choice<juce::dsp::Chorus<float>> chorus;
    DSP_Choice<juce::dsp::LadderFilter<float>> overdrive,ladderFilter;

    using DSP_Pointers = std::array<juce::dsp::ProcessorBase*, static_cast<size_t>(DSP_Option::END_OF_LIST)>;//用别名替代指针数组
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectAudioAudioProcessor)
};

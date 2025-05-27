#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
                     apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    // caching parameter pointers
    delaySizeParam = apvts.getRawParameterValue("delaySize");
    feedbackParam = apvts.getRawParameterValue("feedback");
    wetDryParam = apvts.getRawParameterValue("wetDry");
    gainBeginParam = apvts.getRawParameterValue("gainBegin");
    gainEndParam = apvts.getRawParameterValue("gainEnd");

    // granular parameter caching
    granularModeParam = apvts.getRawParameterValue("granularMode");
    grainSizeParam = apvts.getRawParameterValue("grainSize");
    grainDensityParam = apvts.getRawParameterValue("grainDensity");
    grainPitchParam = apvts.getRawParameterValue("grainPitch");
    grainSpreadParam = apvts.getRawParameterValue("grainSpread");

}

PluginProcessor::~PluginProcessor()
{
}

//======================create parameter layout=================================
juce::AudioProcessorValueTreeState::ParameterLayout PluginProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("delaySize", "Delay Size", 0.01f, 10.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("feedback", "Feedback", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("wetDry", "Wet/Dry", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gainBegin", "Gain Begin", 0.0f, 1.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("gainEnd", "Gain End", 0.0f, 1.0f, 1.0f));

    params.push_back (std::make_unique<juce::AudioParameterBool> ("granularMode", "Granular Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("grainSize", "Grain Size", 10.0f, 500.0f, 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("grainDensity", "Grain Density", 1.0f, 50.0f, 10.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("grainPitch", "Grain Pitch", 0.25f, 4.0f, 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("grainSpread", "Grain Spread", 0.0f, 200.0f, 50.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

    delay.prepare(sampleRate, getTotalNumOutputChannels(), 10.0f);

}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    bool granularMode = *granularModeParam > 0.5f;

    delay.process(buffer,
              *delaySizeParam,
              *feedbackParam,
              *wetDryParam,
              *gainBeginParam,
              *gainEndParam,
              getSampleRate(),
              granularMode,
              *grainSizeParam,
              *grainDensityParam,
              *grainPitchParam,
              *grainSpreadParam);

}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    return new PluginEditor (*this);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}

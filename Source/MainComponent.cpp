#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible (tracheaFrequencySlider);
    tracheaFrequencySlider.setRange (0.0f, 2000.0f);
    tracheaFrequencySlider.setSkewFactorFromMidPoint (1000.0f); // [4]
    tracheaFrequencySlider.setTextValueSuffix("Hz");
    tracheaFrequencySlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, tracheaFrequencySlider.getTextBoxHeight());
    tracheaFrequencySlider.onValueChange = [this]
    {
        if (currentSampleRate > 0.0f)
            updateAngleDelta();
    };
    
    addAndMakeVisible (tracheaFrequencyLabel);
    tracheaFrequencyLabel.setText ("Trachea Frequency", juce::dontSendNotification);
    tracheaFrequencyLabel.attachToComponent (&tracheaFrequencySlider, false);
    
    addAndMakeVisible (ringFreqSlider);
    ringFreqSlider.setRange (0.0f, 200.0f);
    ringFreqSlider.setSkewFactorFromMidPoint (50.0f); // [4]
    ringFreqSlider.setTextValueSuffix("Hz");
    ringFreqSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, ringFreqSlider.getTextBoxHeight());
    ringFreqSlider.onValueChange = [this]
    {
        if (currentSampleRate > 0.0f)
            updateAngleDelta();
    };
    
    addAndMakeVisible (ringFreqLabel);
    ringFreqLabel.setText ("Ring Modulator Frequency", juce::dontSendNotification);
    ringFreqLabel.attachToComponent (&ringFreqSlider, false);
    
    addAndMakeVisible (freqModSlider);
    freqModSlider.setRange (0.0f, 200.0f);
    freqModSlider.setSkewFactorFromMidPoint (50.0f); // [4]
    freqModSlider.setTextValueSuffix("Hz");
    freqModSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, freqModSlider.getTextBoxHeight());
    freqModSlider.onValueChange = [this]
    {
        if (currentSampleRate > 0.0f)
            updateAngleDelta();
    };
    
    addAndMakeVisible (freqModLabel);
    freqModLabel.setText ("Frequency Modulator Frequency", juce::dontSendNotification);
    freqModLabel.attachToComponent (&freqModSlider, false);
    
    addAndMakeVisible (freqModAmpSlider);
    freqModAmpSlider.setRange (0.0f, 1000.0f);
    freqModAmpSlider.setSkewFactorFromMidPoint (250.0f); // [4]
    freqModAmpSlider.setTextValueSuffix("Hz");
    freqModAmpSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, freqModAmpSlider.getTextBoxHeight());
    freqModAmpSlider.onValueChange = [this]
    {
        freqModAmp = freqModAmpSlider.getValue();
    };
    
    addAndMakeVisible (freqModAmpLabel);
    freqModAmpLabel.setText ("Frequency Modulator Amplitude", juce::dontSendNotification);
    freqModAmpLabel.attachToComponent (&freqModAmpSlider, false);
    
    addAndMakeVisible (levelSlider);
    levelSlider.setRange (0.0f, 1.0f);
    levelSlider.setSkewFactorFromMidPoint (0.250f); // [4]
    levelSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 160, levelSlider.getTextBoxHeight());
    levelSlider.onValueChange = [this]
    {
        level.setTargetValue(levelSlider.getValue());
    };
    
    addAndMakeVisible (levelLabel);
    levelLabel.setText ("Level", juce::dontSendNotification);
    levelLabel.attachToComponent (&levelSlider, false);
    
    // specify here on which UDP port number to receive incoming OSC messages
    if (! connect (9001))
        showConnectionErrorMessage ("Error: could not connect to UDP port 9001.");

    // tell the component to listen for OSC messages matching this address:
    addListener (this, tracheaFrequencyAddress);
    addListener (this, ringFrequencyAddress);
    addListener (this, freqModAddress);
    addListener (this, freqModAmpAddress);
    addListener (this, levelAddress);

    setSize (600, 340);
    setAudioChannels (0, 2); // no inputs, two outputs
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    level.reset(sampleRate, 2.0);
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer  = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer (1, bufferToFill.startSample);

    for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        modulateFreq();
        
        auto currentSampleR = (float) std::sin (currentAngle);
        auto currentSampleR2 = (float) std::sin (currentAngle2);
        auto currentSampleR3 = (float) std::sin (currentAngle3);
        auto currentSampleR4 = (float) std::sin (currentAngle4);
        auto currentSampleR5 = (float) std::sin (currentAngle5);
        auto currentSampleR6 = (float) std::sin (currentAngle6);
        
        auto currentSampleRing = (float) std::sin (currentAngleRing);
        
        if (ringFreqSlider.getValue() <= 0.00001f) {
            currentSampleRing = 1.0f;
        }

        currentAngle += angleDelta;
        currentAngle2 += angleDelta2;
        currentAngle3 += angleDelta3;
        currentAngle4 += angleDelta4;
        currentAngle5 += angleDelta5;
        currentAngle6 += angleDelta6;
        
        currentAngleRing += angleDeltaRing;
        
        auto sampleVal = (currentSampleR * 0.0000126f) +
                            (currentSampleR2 * 0.000379f) +
                            (currentSampleR3 * 0.00518f) +
                            (currentSampleR4 * 0.0378f) +
                            (currentSampleR5 * 0.000372f) +
                            (currentSampleR6 * 0.00395f);

        leftBuffer[sample]  = sampleVal * currentSampleRing * level.getNextValue();
        rightBuffer[sample] = sampleVal * currentSampleRing * level.getNextValue();
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::maroon);

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    tracheaFrequencySlider.setBounds (10, 30, getWidth() - 20, 20);
    ringFreqSlider.setBounds(10, 100, getWidth() - 20, 20);
    freqModSlider.setBounds(10, 170, getWidth() - 20, 20);
    freqModAmpSlider.setBounds(10, 240, getWidth() - 20, 20);
    levelSlider.setBounds(10, 310, getWidth() - 20, 20);
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSampleR = tracheaFrequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSampleR * 2.0f * juce::MathConstants<double>::pi;
    
    auto fundamentalFreq = tracheaFrequencySlider.getValue();
    
    auto cyclesPerSampleR2 = fundamentalFreq * 2 / currentSampleRate;
    angleDelta2 = cyclesPerSampleR2 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR3 = fundamentalFreq * 3 / currentSampleRate;
    angleDelta3 = cyclesPerSampleR3 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR4 = fundamentalFreq * 4 / currentSampleRate;
    angleDelta4 = cyclesPerSampleR4 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR5 = fundamentalFreq * 5 / currentSampleRate;
    angleDelta5 = cyclesPerSampleR5 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR6 = fundamentalFreq * 6 / currentSampleRate;
    angleDelta6 = cyclesPerSampleR6 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleRing = ringFreqSlider.getValue() / currentSampleRate;
    angleDeltaRing = cyclesPerSampleRing * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleFreqMod = freqModSlider.getValue() / currentSampleRate;
    angleDeltaFreqMod = cyclesPerSampleFreqMod * 2.0f * juce::MathConstants<double>::pi;
}

void MainComponent::modulateFreq()
{
    auto currentSampleFreqMod = (float) std::sin (currentAngleFreqMod);
    
    if (upModCount > 0) {
        if (currentAngleFreqMod > 1.7f) {
            currentAngleFreqMod = 0.0f;
            upModCount--;
        }
    } else if (currentAngleFreqMod > 3.14f) {
        currentAngleFreqMod = 0.7f;
        downModCount--;
        if (downModCount <= 0) {
            upModCount = juce::Random::getSystemRandom().nextInt(10);
            downModCount = juce::Random::getSystemRandom().nextInt(30);
        }
    }
    
    
    auto fundamentalFreq = tracheaFrequencySlider.getValue();
    
    auto cyclesPerSampleR = (fundamentalFreq + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta = cyclesPerSampleR * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR2 = ((fundamentalFreq * 2) + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta2 = cyclesPerSampleR2 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR3 = ((fundamentalFreq * 3) + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta3 = cyclesPerSampleR3 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR4 = ((fundamentalFreq * 4) + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta4 = cyclesPerSampleR4 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR5 = ((fundamentalFreq * 5) + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta5 = cyclesPerSampleR5 * 2.0f * juce::MathConstants<double>::pi;
    
    auto cyclesPerSampleR6 = ((fundamentalFreq * 6) + (currentSampleFreqMod * freqModAmp)) / currentSampleRate;
    angleDelta6 = cyclesPerSampleR6 * 2.0f * juce::MathConstants<double>::pi;
    
    currentAngleFreqMod += angleDeltaFreqMod;
}

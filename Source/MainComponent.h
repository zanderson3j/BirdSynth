#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
                        private juce::OSCReceiver,
                        private juce::OSCReceiver::ListenerWithOSCAddress<juce::OSCReceiver::MessageLoopCallback>
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void updateAngleDelta();
    void modulateFreq();

private:
    //==============================================================================
    juce::Slider tracheaFrequencySlider;
    juce::Slider ringFreqSlider;
    juce::Slider freqModSlider;
    juce::Slider freqModAmpSlider;
    juce::Slider levelSlider;
    
    juce::Label tracheaFrequencyLabel;
    juce::Label ringFreqLabel;
    juce::Label freqModLabel;
    juce::Label freqModAmpLabel;
    juce::Label levelLabel;
    
    juce::String tracheaFrequencyAddress = "/juce/tracheaFrequency";
    juce::String ringFrequencyAddress = "/juce/ringFrequency";
    juce::String freqModAddress = "/juce/freqModFrequency";
    juce::String freqModAmpAddress = "/juce/freqModAmplitude";
    juce::String levelAddress = "/juce/level";
    
    double currentSampleRate = 0.0f,
            currentAngle = 0.0f,
            currentAngle2 = 0.0f,
            currentAngle3 = 0.0f,
            currentAngle4 = 0.0f,
            currentAngle5 = 0.0f,
            currentAngle6 = 0.0f,
            currentAngleRing = 0.0f,
            currentAngleFreqMod = 0.0f,
            angleDelta = 0.0f,
            angleDelta2 = 0.0f,
            angleDelta3 = 0.0f,
            angleDelta4 = 0.0f,
            angleDelta5 = 0.0f,
            angleDelta6 = 0.0f,
            angleDeltaRing = 0.0f,
            angleDeltaFreqMod = 0.0f,
            freqModAmp = 0.0f;
    
    juce::LinearSmoothedValue<float> level  { 0.0f };
    
    int upModCount = 5;
    int downModCount = 15;
    
    void oscMessageReceived (const juce::OSCMessage& message) override
    {
        if (message.getAddressPattern().matches(tracheaFrequencyAddress) &&
            message.size() == 1 &&
            message[0].isFloat32())
            tracheaFrequencySlider.setValue (juce::jlimit (0.0f, 2000.0f, message[0].getFloat32()));
        
        if (message.getAddressPattern().matches(ringFrequencyAddress) &&
            message.size() == 1 &&
            message[0].isFloat32())
            ringFreqSlider.setValue (juce::jlimit (0.0f, 200.0f, message[0].getFloat32()));
        
        if (message.getAddressPattern().matches(freqModAddress) &&
            message.size() == 1 &&
            message[0].isFloat32())
            freqModSlider.setValue (juce::jlimit (0.0f, 200.0f, message[0].getFloat32()));
        
        if (message.getAddressPattern().matches(freqModAmpAddress) &&
            message.size() == 1 &&
            message[0].isFloat32())
            freqModAmpSlider.setValue (juce::jlimit (0.0f, 1000.0f, message[0].getFloat32()));
        
        if (message.getAddressPattern().matches(levelAddress) &&
            message.size() == 1 &&
            message[0].isFloat32())
            levelSlider.setValue (juce::jlimit (0.0f, 1.0f, message[0].getFloat32()));
    }

    void showConnectionErrorMessage (const juce::String& messageText)
    {
        juce::AlertWindow::showMessageBoxAsync (juce::AlertWindow::WarningIcon,
                                          "Connection error",
                                          messageText,
                                          "OK");
    }


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

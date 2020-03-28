/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Components/TransportComponent.h"
#include "Components/MenuComponent.h"

#include "Synth/MidiSynth.h"
#include "Synth/SfzMidiSynth.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent   : public AudioAppComponent,
                        private Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    bool fileCallback(CommandID);
    void handleFileOpen();

private:
    //==============================================================================
    static String getAbsolutePathOfProject(const String& projectFolderName = "MIDI-Editor");
    void timerCallback() override;

    // Your private member variables go here...
    TransportComponent transportBar;
    MenuComponent menu;
    MidiFile midiFile;

    SfzSynthAudioSource synthAudioSource;
    MidiKeyboardState keyboardState;
    MidiKeyboardComponent keyboardComponent;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() :
    m_pPlayer(std::make_shared<PlayerComponent>()),
    m_pTrackView(std::make_unique<TrackViewComponent>())
{
    // Make sure you set the size of the component after
    // you add any child components.
    addAndMakeVisible(m_transportBar);
    addAndMakeVisible(m_menu);
    m_menu.setCallback(std::bind(&MainComponent::fileCallback, this, std::placeholders::_1));
    m_menu.setPlayer(m_pPlayer);
    addActionListener(&m_menu);

    // Create Player
    m_transportBar.init(m_pPlayer);

    //TracksView
    m_pTrackView->init(m_pPlayer);
    addAndMakeVisible(*m_pTrackView);

    // Some platforms require permissions to open input channels so request that here
    if (RuntimePermissions::isRequired (RuntimePermissions::recordAudio)
        && ! RuntimePermissions::isGranted (RuntimePermissions::recordAudio))
    {
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [&] (bool granted) { if (granted)  setAudioChannels (m_iNumChannels, m_iNumChannels); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (m_iNumChannels, m_iNumChannels);
    }

    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        m_fSampleRate = device->getCurrentSampleRate();
        m_iBitDepth = device->getCurrentBitDepth();
        DBG ("Current audio device: "   + device->getName().quoted());
        DBG ("Sample rate: "    + String (m_fSampleRate) + " Hz");
        DBG ("Block size: "     + String (device->getCurrentBufferSizeSamples()) + " samples");
        DBG ("Bit depth: "      + String (m_iBitDepth));
    }
    else
    {
        DBG ("No audio device open");
    }

    m_pPlayer->addActionListener(this);
    addActionListener(&m_transportBar);


    setSize (1000, 500);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    delete m_pSequenceInSec;
    removeAllActionListeners();
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    m_pPlayer->prepareToPlay(samplesPerBlockExpected, sampleRate);
    m_fSampleRate = sampleRate;
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    m_pPlayer->getNextAudioBlock(bufferToFill);
    if (m_bExporting)
        m_pAudioExporter->write(bufferToFill);
//    AudioBuffer<float>* buffer = bufferToFill.buffer;
//    for (int i=0; i<bufferToFill.numSamples; i++) {
//        std::cout << buffer->getSample(0, i) << " ";
//    }
//    std::cout << std::endl;

}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    removeAllActionListeners();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
//    g.setFont (Font (16.0f));
//    g.setColour (Colours::white);
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.

    Rectangle<int> bounds = getLocalBounds();
    int height = bounds.getHeight();
    bounds.setHeight(64);
    bounds.setY(height - 64);
    m_transportBar.setBounds(bounds);

    bounds = getLocalBounds();
    bounds.removeFromBottom(m_transportBar.getBounds().getHeight());
    m_pTrackView->setBounds(bounds);
}

bool MainComponent::fileCallback(CommandID commandID) {
    switch (commandID)
    {
        case MenuComponent::fileOpen:
            handleFileOpen();
            break;

        case MenuComponent::fileExportAudio:
            handleExportAudio();
            break;

//        case MenuComponent::fileExportMIDI:
//            break;

        default:
            return false;
    }
    return true;
}

void MainComponent::handleFileOpen() {
    FileChooser chooser ("Select a MIDI file to play...", {}, "*.mid");

    if (chooser.browseForFileToOpen()) {
        auto file = chooser.getResult();
        FileInputStream* stream = file.createInputStream();
        bool err = m_midiFile.readFrom(*stream);
        if (!err) {
            std::cerr << "Error readFrom MidiFile" << std::endl;
            return;
        }

        // clean stuff in the last track
        delete m_pSequenceInSec;
        m_pPlayer->clearTempoEvents();

        // Set time format (ticks per quarter note if +ve)
        int timeFormat = m_midiFile.getTimeFormat();
        m_pTrackView->setTimeFormat(timeFormat);
        m_pPlayer->setTimeFormat(timeFormat);

        // Create sequence with timestamps in ticks
        auto pSequenceInTicks = new MidiMessageSequence();
        for (int i=0; i < m_midiFile.getNumTracks(); i++) {
            pSequenceInTicks->addSequence(*m_midiFile.getTrack(i), 0);
            pSequenceInTicks->updateMatchedPairs();
        }

        int numTimeStampsForPianoRoll = jmax(Globals::PianoRoll::initTimeStamps, static_cast<int>(pSequenceInTicks->getEndTime()/timeFormat) + 10);
        m_pTrackView->addTrack(numTimeStampsForPianoRoll);


        // init m_TempoEvents in PlayerComponent
        m_midiFile.findAllTempoEvents(m_pPlayer->getTempoEventsInTicks());


        // ----------Timestamp conversion----------
        m_midiFile.convertTimestampTicksToSeconds();


        // init m_TempoEventsInSecs in PlayerComponent
        m_midiFile.findAllTempoEvents(m_pPlayer->getTempoEventsInSecs());

        // Create sequence with timestamps in seconds
        m_pSequenceInSec = new MidiMessageSequence();
        for (int i=0; i < m_midiFile.getNumTracks(); i++) {
            m_pSequenceInSec->addSequence(*m_midiFile.getTrack(i), 0);
            m_pSequenceInSec->updateMatchedPairs();
        }

        // Pass sequence in Seconds to Player
        m_pPlayer->setMidiMessageSequence(m_pSequenceInSec);

        // Pass sequence in Ticks to TrackView
        m_pTrackView->convertMidiMessageSequence(0, pSequenceInTicks);


        toFront(true);

        delete pSequenceInTicks;
        delete stream;
    }
}

void MainComponent::handleExportAudio() {
    FileChooser chooser("Select path to save WAV file...");

    if (chooser.browseForFileToSave(true)) {
        auto file = chooser.getResult();

        if (!m_pAudioExporter)
            m_pAudioExporter = std::make_unique<AudioExportComponent>(m_fSampleRate, m_iNumChannels, 16); // hard coded bitdepth for now

        m_pAudioExporter->startThread();
        m_pAudioExporter->init(file);
        sendActionMessage(Globals::ActionMessage::PlayForExport);
        m_bExporting = true;
    }
}

void MainComponent::handleExportMidi() {
    FileChooser chooser("Select path to save MIDI file...");

    if (chooser.browseForFileToSave(true)) {
        auto file = chooser.getResult();
        if (auto* stream = file.createOutputStream()) {
        // TODO
        }
    }

}

void MainComponent::actionListenerCallback (const String& message) {
    if ((message == Globals::ActionMessage::PlayForExport) && m_bExporting) {
        m_pAudioExporter->finish();
        m_bExporting = false;
    }

    if ((message == Globals::ActionMessage::Stop) && m_bExporting) {
        m_pAudioExporter->finish();
        m_bExporting = false;
    }

    if ((message == Globals::ActionMessage::EnableTransport)) {
        sendActionMessage(message);
    }
}

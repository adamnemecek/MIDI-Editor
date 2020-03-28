/*
  ==============================================================================

    MenuComponent.cpp
    Created: 12 Jan 2020 12:28:18pm
    Author:  Raghavasimhan Sankaranarayanan

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "MenuComponent.h"

//==============================================================================
MenuComponent::MenuComponent()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    menuBar.reset(new MenuBarComponent(this));
    addAndMakeVisible(menuBar.get());
    setApplicationCommandManagerToWatch(&commandManager);
    commandManager.registerAllCommandsForTarget(this);
    addKeyListener(commandManager.getKeyMappings());
    addAndMakeVisible(editCommandTarget);
    MenuBarModel::setMacMainMenu(this);
    commandManager.setFirstCommandTarget(this);
}

MenuComponent::~MenuComponent()
{
    MenuBarModel::setMacMainMenu(nullptr);
}

void MenuComponent::paint (Graphics& g) {}

void MenuComponent::resized() {}

StringArray MenuComponent::getMenuBarNames() {
    return {"File", "Edit", "Help"};
}

PopupMenu MenuComponent::getMenuForIndex(int menuIndex, const String& name) {
    PopupMenu menu;
    if (menuIndex == 0) {
        menu.addCommandItem(&commandManager, fileOpen);
        menu.addCommandItem(&commandManager, fileExportAudio);
        menu.addCommandItem(&commandManager, fileExportMIDI);
    }
    else if (menuIndex == 1) {
        menu.addCommandItem(&commandManager, editUndo);
        menu.addCommandItem(&commandManager, editRedo);
        menu.addCommandItem(&commandManager, editCut);
        menu.addCommandItem(&commandManager, editCopy);
        menu.addCommandItem(&commandManager, editPaste);
    }
    else if (menuIndex == 2) {
        menu.addCommandItem(&commandManager, helpDocumentation);
    }
    return menu;
}

ApplicationCommandTarget* MenuComponent::getNextCommandTarget() {
    return &editCommandTarget;
}

void MenuComponent::getAllCommands (Array<CommandID>& c) {
    Array<CommandID> commands { fileOpen, fileExportAudio, fileExportMIDI };
    c.addArray (commands);
}

void MenuComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case fileOpen:
            result.setInfo ("Open", "Open MIDI file", "File", 0);
            result.addDefaultKeypress ('i', ModifierKeys::commandModifier);
            break;
        case fileExportAudio:
            result.setInfo ("Export Audio", "Export Audio", "File", 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
            break;
        case fileExportMIDI:
            result.setInfo ("Export MIDI", "Export MIDI", "File", 0);
            result.addDefaultKeypress ('b', ModifierKeys::shiftModifier | ModifierKeys::commandModifier);
            break;
        default:
            break;
    }
}

void MenuComponent::menuItemSelected (int /*menuItemID*/, int /*topLevelMenuIndex*/) {}

bool MenuComponent::perform (const InvocationInfo& info)
{
    return callbackFunc(static_cast<CommandIDs>(info.commandID));
}

void MenuComponent::setCallback(cbfunc func) {
    callbackFunc = func;
}

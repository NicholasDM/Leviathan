// Common function for updating the game setup screen //

// Function for detecting applied settings //
PLAYERCONTROLS DetectPlayerControlsFromOptionName(string option){
    if(option == "")
        return PLAYERCONTROLS_NONE;
    if(option == "AI")
        return PLAYERCONTROLS_AI;
    if(option == "WASD")
        return PLAYERCONTROLS_WASD;
    if(option == "ARROWS")
        return PLAYERCONTROLS_ARROWS;
    if(option == "IJKL")
        return PLAYERCONTROLS_IJKL;
    if(option == "NUMPAD")
        return PLAYERCONTROLS_NUMPAD;
    if(option == "CONTROLLER")
        return PLAYERCONTROLS_CONTROLLER;

    return PLAYERCONTROLS_NONE;
}

string GetPlayerControlsOptionNameFromType(PLAYERCONTROLS type){
    if(type == PLAYERCONTROLS_NONE)
        return "";
    if(type == PLAYERCONTROLS_AI)
        return "AI";
    if(type == PLAYERCONTROLS_WASD)
        return "WASD";
    if(type == PLAYERCONTROLS_ARROWS)
        return "ARROWS";
    if(type == PLAYERCONTROLS_IJKL)
        return "IJKL";
    if(type == PLAYERCONTROLS_NUMPAD)
        return "NUMPAD";
    if(type == PLAYERCONTROLS_CONTROLLER)
        return "CONTROLLER";
    // didn't match any //
    return "";
}

// This function prints the configuration table inside the provided element with the added name prefixes //
void UpdateGameSetupScreen(BaseGuiObject@ Instance, string nameprefix){
    // Construct the internal rml code //
    string code = "";
    
    // Loop through players and print the settings //
    for(int i = 0; i < 4; i++){
        if(i != 0){
            // Some limiting lines //
            code += "<hr></hr>";
        }
        PlayerSlot@ slot = GetPongGame().GetSlot(i);
        bool teamprinted = false;
        // Identifier for the object names //
        int ControlIdentifier = i;
        // for looping through split slots //
        while(true){
            if(!teamprinted){
                teamprinted = true;
                code += "<p>Team "+i+"</p>";
            }
            
            // Check for closed slots //
            if(!slot.IsActive()){
                // Add a join button //
                code += "<p class='MenuOption' id='"+nameprefix+"Join_"+ControlIdentifier+"'>Click here to join!</p>";
                
            } else {
                // Print info and controls //
                code += "<p>Player "+slot.GetPlayerNumber();
                // Options //
                code += "<br></br>";
                code += "<form>";
                code += "<dataselect source='GameConfiguration.Colours' fields='Name,Colour' valuefield='Name'"
                    "id='"+nameprefix+"Colour_"+ControlIdentifier+"'></dataselect>";
                code += "<br></br>";
                // TODO: the following really needs a formatter!
                
                string chosenctrl = GetPlayerControlsOptionNameFromType(slot.GetControlType());
                code += "<dataselect source='GameConfiguration.Controls' fields='Type,ID' valuefield='Type'"
                    "id='"+nameprefix+"Controls_"+ControlIdentifier+"' value='"+chosenctrl+"'></dataselect>";
                code += "<br></br>";
                code += "</form>";
                code += "<p class='MenuOption DangerousMenu' id='"+nameprefix+"Quit_"+ControlIdentifier+"'>Close</p>";
                code += "</p>";
                code += "<br></br>";
            }
            @slot = slot.GetSplit();
            if(slot is null){
                // Add a split option
                code += "<p class='MenuOption' id='"+nameprefix+"Split_"+ControlIdentifier+"'>Split this slot</p>";
                break;
            }
            // We need a unique name for the split slot
            ControlIdentifier += i*10;
        }
    }
    
    Instance.SetInternalElementRML(code);

    // We have added objects with IDs so we want to make gui check if everything is fine //
    Instance.GetOwningManager().GUIObjectsCheckRocketLinkage();
}

PlayerSlot@ GetPlayerSlotFromControlIdentifier(int identifier){
    // Create identifiers until matching one is found //
    for(int i = 0; i < 4; i++){

        PlayerSlot@ slot = GetPongGame().GetSlot(i);
        // Identifier for the object names //
        int ControlIdentifier = i;
        // for looping through split slots //
        while(true){
            
            if(ControlIdentifier == identifier)
                return slot;
            
            @slot = slot.GetSplit();
            if(slot is null){
                // Add a split option
                break;
            }
            // We need a unique name for the split slot
            ControlIdentifier += i*10;
        }
    }
    return null;
}

// ------------------ Setting update handling ------------------ //

// Call when a colour is changed //
void ColourChangeUpdated(int controlidentifier, string newvalue){

    Print("Setting "+controlidentifier+" colour to: "+newvalue);
    // Get the slot from the identifier //
    PlayerSlot@ slot = GetPlayerSlotFromControlIdentifier(controlidentifier);
    
    // Do inverse table lookup //
    string colourvalue = GetPongGame().GetGameDatabase()
        .GetValueOnRow("Colours", "Name", ScriptSafeVariableBlock("Key", newvalue), 
        "Colour").ConvertAndReturnString();
    
    // Set the colour //
    slot.SetColourFromRML(colourvalue);
}




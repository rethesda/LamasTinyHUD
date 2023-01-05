ScriptName LamasTinyHUD_MCM Extends MCM_ConfigBase

Bool Property bMagicSelected Auto
Bool Property bPowerSelected Auto
Bool Property bWeaponSelected Auto
Bool Property bBothHands Auto

Bool Property bMagicSelectedLeft Auto
Bool Property bPowerSelectedLeft Auto

Event OnConfigClose() native
string[] function GetSelectedOptions(int a_id, bool a_both) native
int function GetFormIdForSelection(int a_index) native
string function GetResolutionWidth() native
string function GetResolutionHeight() native

function SetToggles()
    bMagicSelected = GetModSettingInt("uTopType:TopPage") == 1
    bPowerSelected = GetModSettingInt("uTopType:TopPage") == 3
    bWeaponSelected = GetModSettingInt("uTopType:TopPage") == 0
    bBothHands = GetModSettingInt("uTopHandSelection:TopPage") == 0
    
    bMagicSelectedLeft = GetModSettingInt("uTopTypeLeft:TopPage") == 1
    bPowerSelectedLeft = GetModSettingInt("uTopTypeLeft:TopPage") == 3
endfunction

function RefreshItemsMain(string a_type, string a_selection, string a_action)
    SetToggles()
    string[] menu_list = GetSelectedOptions(GetModSettingInt(a_type), bBothHands)
    SetMenuOptions(a_selection, menu_list, menu_list)
    SetModSettingInt(a_selection, 0)

    SetModSettingInt(a_action, 0)
    RefreshMenu()
endFunction

function RefreshItems()
    RefreshItemsMain("uTopType:TopPage", "uTopSelectedItem:TopPage", "uTopSlotAction:TopPage")
endfunction

function RefreshItemsLeft()
    RefreshItemsMain("uTopTypeLeft:TopPage", "uTopSelectedItemLeft:TopPage", "uTopSlotActionLeft:TopPage")
endfunction

Event OnSettingChange(String a_ID)
    if (a_ID == "uTopSelectedItem:TopPage")
        SetModSettingString("sSelectedTopItemForm:TopPage", GetFormIdForSelection(GetModSettingInt(a_ID)))
        RefreshMenu()
    elseIf ( a_ID == "uTopSelectedItemLeft:TopPage")
        SetModSettingString("sSelectedTopItemFormLeft:TopPage", GetFormIdForSelection(GetModSettingInt(a_ID)))
        RefreshMenu()
    endif
EndEvent

Event OnPageSelect(string a_page)
    if (a_page == "$LamasTinyHUD_TopPage")
        SetToggles()
        ;no need to set uTopSelectedItem:TopPage here
        string[] menu_list = GetSelectedOptions(GetModSettingInt("uTopType:TopPage"), bBothHands)
        SetMenuOptions("uTopSelectedItem:TopPage", menu_list, menu_list)

        menu_list = GetSelectedOptions(GetModSettingInt("uTopTypeLeft:TopPage"), bBothHands)
        SetMenuOptions("sSelectedTopItemFormLeft:TopPage", menu_list, menu_list)

        RefreshMenu()
    elseif ( a_page == "$LamasTinyHUD_HudSetting" )
        SetModSettingString("sDisplayResolutionWidth:HudSetting",GetResolutionWidth())
        SetModSettingString("sDisplayResolutionHeight:HudSetting",GetResolutionHeight())
        RefreshMenu()
    endIf
EndEvent
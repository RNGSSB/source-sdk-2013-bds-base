"GameMenu"
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	}
	"2"
	{
		"label" "#GameUI_GameMenu_ReturnToMainMenu"
		"command" "engine disconnect;map_background background01"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	}
	"3"
	{
		"label" "#GameUI_GameMenu_Disconnect"
		"command" "Disconnect"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	}
	"4"
	{
		"label" "#GameUI_GameMenu_PlayerList"
		"command" "OpenPlayerListDialog"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	} 
	"5"
	{
		"label" "------------------------"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	}
	"6"
	{
		"label" "#GameUI_GameMenu_QuickSearch" 
		"command" "engine serverfinderdialog"
		"OnlyInLegacyMode"	"1"
	} 
	"7"
	{
		"label" "#GameUI_GameMenu_FindServers" 
		"command" "OpenServerBrowser"
		"OnlyInLegacyMode"	"1"
	} 
	"8"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "OpenCreateMultiplayerGameDialog"
		"OnlyInLegacyMode"	"1"
	}
	"9"
	{
		"label"	"#GameUI_GameMenu_OfflinePractice"
		"command" "engine training_showdlg"
		"OnlyInLegacyMode"	"1"
	}
	"10"
	{
		"label"	"#GameUI_LoadCommentary"
		"command" "OpenLoadSingleplayerCommentaryDialog"
		"OnlyInLegacyMode"	"1"
	}
	"11"
	{
		"label" "#GameUI_Controller"
		"command" "OpenControllerDialog"
		"ConsoleOnly" "1"
		"OnlyInLegacyMode"	"1"
	}
	"12"
	{
		"label" "------------------------"
		"OnlyInLegacyMode"	"1"
	}
	"13"
	{
		"label" "#GameUI_GameMenu_CharacterSetup"
		"command" "engine open_charinfo"
		"OnlyInLegacyMode"	"1"
	}
	"14"
	{
		"label" "#GameUI_GameMenu_Achievements"
		"command" "OpenAchievementsDialog"
		"OnlyInLegacyMode"	"1"
	}
	"15"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
		"OnlyInLegacyMode"	"1"
	}
	"16"
	{
		"label" "#GameUI_GameMenu_AdvancedOptions"
		"command" "engine opentf2options"
		"OnlyInLegacyMode"	"1"
	}
	"17"
	{
		"label" "------------------------"
		"OnlyInLegacyMode"	"1"
	}
	"18"
	{
		"label" "#GameUI_GameMenu_CallVote"
		"command" "engine open_vote"
		"OnlyInGame" "1"
		"OnlyInLegacyMode"	"1"
	} 
	"19"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "Quit"
		"OnlyInLegacyMode"	"1"
	}
}

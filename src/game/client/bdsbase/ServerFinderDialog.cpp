//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ServerFinderDialog.h"

#include <stdio.h>

using namespace vgui;

#include <vgui/ILocalize.h>

#include "FileSystem.h"
#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/convar.h"
#include "ServerBrowser/blacklisted_server_manager.h"
#include "ServerBrowser/IServerBrowser.h"
#include "tier1/netadr.h"
#include "vgui_controls/MessageBox.h"

// for SRC
#include <vstdlib/random.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#ifdef BDSBASE_ALLOW_SERVERFINDER

#define RANDOM_MAP "#GameUI_RandomMap"
#define SERVERFINDER_CONFIG "ServerFinderConfig.vdf"

extern const char* COM_GetModDirectory();

void LoadCommand(void)
{
	CServerFinderDialog* pCServerFinderDialog = new CServerFinderDialog(NULL);
	pCServerFinderDialog->Activate();
}

ConCommand serverfinderdialog("serverfinderdialog", LoadCommand, "", FCVAR_NONE);

static void AddFilter(CUtlVector<MatchMakingKeyValuePair_t>& vecServerFilters, const char* pchKey, const char* pchValue)
{
	// @note Tom Bui: this is to get around the linker error where we don't like strncpy in MatchMakingKeyValuePair_t's constructor!
	int idx = vecServerFilters.AddToTail();
	MatchMakingKeyValuePair_t& keyvalue = vecServerFilters[idx];
	Q_strncpy(keyvalue.m_szKey, pchKey, sizeof(keyvalue.m_szKey));
	Q_strncpy(keyvalue.m_szValue, pchValue, sizeof(keyvalue.m_szValue));
}

static bool BHasTag(const CUtlStringList& TagList, const char* tag)
{
	for (int i = 0; i < TagList.Count(); i++)
	{
		if (!Q_stricmp(TagList[i], tag))
		{
			return true;
		}
	}
	return false;
}

bool HasAppropriateTags(const CUtlStringList& TagList, const CUtlStringList& Req, const CUtlStringList& Rej)
{
	FOR_EACH_VEC(Req, idx)
	{
		if (!BHasTag(TagList, Req[idx]))
		{
			Msg("Cannot find required tag %s\n", Req[idx]);
			return false;
		}
	}

	FOR_EACH_VEC(Rej, idx)
	{
		if (BHasTag(TagList, Rej[idx]))
		{
			Msg("Found rejected tag %s\n", Rej[idx]);
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CServerFinderDialog::CServerFinderDialog(vgui::Panel *parent) : BaseClass(NULL, "ServerFinderConfig")
{
	m_hServerListRequest = NULL;

	SetSize(348, 460);
	//SetOKButtonText("#GameUI_Start");

	m_pOptions = new ServerFinderOptions_t();
	
	m_pMapList = new ComboBox(this, "MapList", 12, false);
	m_maxPlayers = new TextEntry( this, "MaxPlayers" );
#if defined(TF_CLIENT_DLL)
	m_pRandCrits = new ComboBox(this, "RandCrits", 12, false);
#if defined(QUIVER_DLL)
	m_pRandCrits->AddItem("#TF_Quickplay_DamageSpread_Default", NULL);
	m_pRandCrits->AddItem("#TF_Quickplay_DamageSpread_Enabled", NULL);
#else
	m_pRandCrits->AddItem("#TF_Quickplay_RandomCrits_Default", NULL);
	m_pRandCrits->AddItem("#TF_Quickplay_RandomCrits_Disabled", NULL);
#endif
	m_pRandCrits->AddItem("#TF_Quickplay_RandomCrits_DontCare", NULL);

	m_pDmgSpread = new ComboBox(this, "DmgSpread", 12, false);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_Default", NULL);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_Enabled", NULL);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_DontCare", NULL);
#endif

	m_pRespawnTimes = new ComboBox(this, "RespawnTimes", 12, false);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_Default", NULL);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_Instant", NULL);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_DontCare", NULL);

	LoadMapList();

	// create KeyValues object to load/save config options
	m_pSavedData = new KeyValues("ServerFinderConfig");

	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile(g_pFullFileSystem, SERVERFINDER_CONFIG, "GAME"); // this is game-specific data, so it should live in GAME, not CONFIG
		SetParams();
	}

	GetOptions();

	LoadControlSettings("Resource/ServerFinderConfig.res");

	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CServerFinderDialog::~CServerFinderDialog()
{
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

void CServerFinderDialog::SaveOptionSelection(bool reload)
{
	char szMapName[64];
	Q_strncpy(szMapName, GetMapName(), sizeof(szMapName));

	char szMaxBots[256];
	m_maxPlayers->GetText(szMaxBots, sizeof(szMaxBots));

	// save the config data
	if (m_pSavedData)
	{
		// even if we have a random map selected, always save the map so we can queue for games with this map.
		m_pSavedData->SetString("map", szMapName);
		// if random map is a thing though, add a boolean to tell the ui to select the random map when it loads.
		m_pSavedData->SetBool("israndommap", IsRandomMapSelected());
		m_pSavedData->SetString("MaxPlayers", szMaxBots);
#if defined(TF_CLIENT_DLL)
		m_pSavedData->SetInt("RandomCrits", (ERandCritsOption)m_pRandCrits->GetItemIDFromRow(m_pRandCrits->GetActiveItem()));
		m_pSavedData->SetInt("DamageSpread", (EDamageSpreadOption)m_pDmgSpread->GetItemIDFromRow(m_pDmgSpread->GetActiveItem()));
#endif
		m_pSavedData->SetInt("RespawnTimes", (ERespawnTimes)m_pRespawnTimes->GetItemIDFromRow(m_pRespawnTimes->GetActiveItem()));

		// save config to a file
		m_pSavedData->SaveToFile(g_pFullFileSystem, SERVERFINDER_CONFIG, "GAME");

		if (reload)
		{
			// reload the saved file.
			SetParams();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CServerFinderDialog::OnClose()
{
	BaseClass::OnClose();
	
	SaveOptionSelection(false);

	MarkForDeletion();
}

//
// ISteamMatchmakingServerListResponse overrides
//

void CServerFinderDialog::ServerResponded(HServerListRequest hRequest, int iServer)
{
	gameserveritem_t* server = steamapicontext->SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);
	if (server)
	{
		ServerResponded(*server);
	}
}

void CServerFinderDialog::ServerFailedToRespond(HServerListRequest hRequest, int iServer)
{
}

void CServerFinderDialog::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	// we failed, men
	if (response == eNoServersListedOnMasterServer)
	{
		// create a server with neuro-samas in it
		Msg("Failed to find any server, starting bot server.\n");
		OnSearchFailure();
	}

	DestroyServerListRequest();
}

void CServerFinderDialog::ServerResponded(gameserveritem_t& server)
{
	Msg("SERVER FOUND! Validating.\n");

	bool invalid = false;

	if (!server.m_bHadSuccessfulResponse)
	{
		invalid = true;
		Msg("SERVER INVALID: Response wasn't successful\n");
	}

	// Ignore servers with bogus address. Is this even possible?
	if (!server.m_NetAdr.GetIP() || !server.m_NetAdr.GetConnectionPort())
	{
		Assert(server.m_NetAdr.GetIP() && server.m_NetAdr.GetConnectionPort());
		invalid = true;
		Msg("SERVER INVALID: Response was invalid\n");
	}

	if (server.m_bPassword)
	{
		invalid = true;
		Msg("SERVER INVALID: Server is password protected\n");
	}

	//first, are the max players invalid?
	if (server.m_nMaxPlayers > m_pOptions->m_iMaxPlayers)
	{
		invalid = true;
		Msg("SERVER %s: Max players are too high\n", server.GetName());
	}

	CUtlStringList requiredTags;
	CUtlStringList illegalTags;

	//next, parse the tags.
	switch (m_pOptions->m_eRespawnTimes)
	{
		case eRespawnTimesDefault:
			illegalTags.CopyAndAddToTail("norespawntime");
			break;

		case eRespawnTimesInstant:
			requiredTags.CopyAndAddToTail("norespawntime");
			break;

		default:
			Assert(false);
		case eRespawnTimesDontCare:
			break;
	}

#if defined(TF_CLIENT_DLL)
	switch (m_pOptions->m_eRandomCrits)
	{
		case eRandCritsYes:
			requiredTags.CopyAndAddToTail("nocrits");
			break;

		case eRandCritsNo:
			illegalTags.CopyAndAddToTail("nocrits");
			break;

		default:
			Assert(false);
		case eRandCritsDontCare:
			break;
	}

	switch (m_pOptions->m_eDamageSpread)
	{
		case eDamageSpreadNo:
			illegalTags.CopyAndAddToTail("dmgspread");
			break;

		case eDamageSpreadYes:
			requiredTags.CopyAndAddToTail("dmgspread");
			break;

		default:
			Assert(false);
		case eDamageSpreadDontCare:
			break;
	}
#endif

	CUtlStringList TagList; // auto-deletes strings on scope exit
	if (server.m_szGameTags && server.m_szGameTags[0])
	{
		V_SplitString(server.m_szGameTags, ",", TagList);
	}

	if (!HasAppropriateTags(TagList, requiredTags, illegalTags))
	{
		Msg("SERVER %s: Server has illegal tags or is missing required tags!\n", server.GetName());
		invalid = true;
	}

	// we join the first valid server regardless of ping.
	if (!invalid)
	{
		Msg("SERVER %s: Server validation succeeded. Attempting to join %s!\n", server.GetName(), server.m_NetAdr.GetConnectionAddressString());
		// if this server is valid, join it.
		char szJoinCommand[1024];
		// create the command to execute
		Q_snprintf(szJoinCommand, sizeof(szJoinCommand), "disconnect\nconnect %s", server.m_NetAdr.GetConnectionAddressString());
		// exec
		engine->ClientCmd_Unrestricted(szJoinCommand);
		OnClose();
	}
	else
	{
		Msg("SERVER: Server validation failed. Starting a bot server.\n");
		OnSearchFailure();
	}
}

void CServerFinderDialog::BeginSearch()
{ 
	// Setup search filters
	CUtlVector<MatchMakingKeyValuePair_t> vecServerFilters;
	AddFilter(vecServerFilters, "gamedir", COM_GetModDirectory());
	// source sdk 2013 has no secure servers......
	/*if (GetUniverse() == k_EUniversePublic)
	{
		AddFilter(vecServerFilters, "secure", "1");
		AddFilter(vecServerFilters, "dedicated", "1");
	}*/
	AddFilter(vecServerFilters, "full", "1"); // actually means "not full"
	AddFilter(vecServerFilters, "map", GetMapName());

	Msg("Beginning server search...\n");

	CUtlString sFilter;
	FOR_EACH_VEC(vecServerFilters, idx)
	{
		sFilter.Append(vecServerFilters[idx].m_szKey);
		sFilter.Append(" - ");
		sFilter.Append(vecServerFilters[idx].m_szValue);
		sFilter.Append("\n");
	}
	Msg("Using filter: %s\n", sFilter.String());

	MatchMakingKeyValuePair_t* pFilters = vecServerFilters.Base(); // <<<< Note, this is weird, but correct.
	m_hServerListRequest = steamapicontext->SteamMatchmakingServers()->RequestInternetServerList(
		engine->GetAppID(),
		&pFilters,
		vecServerFilters.Count(),
		this);
}

void CServerFinderDialog::OnSearchFailure()
{
	char szMapCommand[1024];
	// create the command to execute
#if defined(TF_CLIENT_DLL)
	Q_snprintf(szMapCommand, sizeof(szMapCommand),
		"exec serverfinder_fail.cfg\ntf_bot_quota %i\nmaxplayers %i\nmp_disable_respawn_times %i\ntf_weapon_criticals %i\ntf_damage_disablespread %i\nprogress_enable\nmap %s\n",
		(m_pOptions->m_iMaxPlayers - 1),
		m_pOptions->m_iMaxPlayers,
		(m_pOptions->m_eRespawnTimes == eRespawnTimesInstant ? 1 : (m_pOptions->m_eRespawnTimes == eRespawnTimesDontCare ? random->RandomInt(0,1) : 0)),
		(m_pOptions->m_eRandomCrits == eRandCritsYes ? 1 : (m_pOptions->m_eRandomCrits == eRandCritsDontCare ? random->RandomInt(0, 1) : 0)),
		(m_pOptions->m_eDamageSpread == eDamageSpreadNo ? 1 : (m_pOptions->m_eDamageSpread == eDamageSpreadDontCare ? random->RandomInt(0, 1) : 0)),
		GetMapName());
#elif defined(HL2MP)
	Q_snprintf(szMapCommand, sizeof(szMapCommand),
		"exec serverfinder_fail.cfg\ntf_bot_quota %i\nmaxplayers %i\nmp_disable_respawn_times %i\nprogress_enable\nmap %s\n",
		(m_pOptions->m_iMaxPlayers - 1),
		m_pOptions->m_iMaxPlayers,
		(m_pOptions->m_eRespawnTimes == eRespawnTimesInstant ? 1 : (m_pOptions->m_eRespawnTimes == eRespawnTimesDontCare ? random->RandomInt(0, 1) : 0)),
		GetMapName());
#endif

	// exec
	engine->ClientCmd_Unrestricted(szMapCommand);
	OnClose();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CServerFinderDialog::OnCommand(const char *command)
{
	if ( !stricmp( command, "Ok" ) )
	{
		SaveOptionSelection();

		if (engine->IsInGame())
		{
			Msg("Already connected to server.\n");
			vgui::MessageBox* pMessageBox = new vgui::MessageBox("#Serverfinder_Title", "#Serverfinder_Error_CannotJoinConnected", GetParent());
			pMessageBox->DoModal();
			OnClose();
			return;
		}

		// SEARCH HERE.
		BeginSearch();
		
		return;
	}

	BaseClass::OnCommand( command );
}

void CServerFinderDialog::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CServerFinderDialog::LoadMaps(const char *pszPathID)
{
	FileFindHandle_t findHandle = NULL;

	const char *pszFilename = g_pFullFileSystem->FindFirst( "maps/*.bsp", &findHandle );
	while ( pszFilename )
	{
		char mapname[256];

		// FindFirst ignores the pszPathID, so check it here
		// TODO: this doesn't find maps in fallback dirs
		Q_snprintf( mapname, sizeof(mapname), "maps/%s", pszFilename );
		if ( !g_pFullFileSystem->FileExists( mapname, pszPathID ) )
		{
			goto nextFile;
		}

		// remove the text 'maps/' and '.bsp' from the file name to get the map name
		
		const char *str = Q_strstr( pszFilename, "maps" );
		if ( str )
		{
			Q_strncpy( mapname, str + 5, sizeof(mapname) - 1 );	// maps + \\ = 5
		}
		else
		{
			Q_strncpy( mapname, pszFilename, sizeof(mapname) - 1 );
		}
		char *ext = Q_strstr( mapname, ".bsp" );
		if ( ext )
		{
			*ext = 0;
		}

		// add to the map list
		m_pMapList->AddItem( mapname, new KeyValues( "data", "mapname", mapname ) );

		// get the next file
	nextFile:
		pszFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );
}



//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CServerFinderDialog::LoadMapList()
{
	// clear the current list (if any)
	m_pMapList->DeleteAllItems();

	// add special "name" to represent loading a randomly selected map
	m_pMapList->AddItem( RANDOM_MAP, new KeyValues( "data", "mapname", RANDOM_MAP ) );

	// iterate the filesystem getting the list of all the files
	// UNDONE: steam wants this done in a special way, need to support that
	const char *pathID = "GAME";

	// Load the GameDir maps
	LoadMaps( pathID ); 

	// set the first item to be selected
	m_pMapList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CServerFinderDialog::IsRandomMapSelected()
{
	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!stricmp( mapname, RANDOM_MAP ))
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CServerFinderDialog::GetMapName()
{
	int count = m_pMapList->GetItemCount();

	// if there is only one entry it's the special "select random map" entry
	if( count <= 1 )
		return NULL;

	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!strcmp( mapname, RANDOM_MAP ))
	{
		int which = RandomInt( 1, count - 1 );
		mapname = m_pMapList->GetItemUserData( which )->GetString("mapname");
	}

	return mapname;
}

//-----------------------------------------------------------------------------
// Purpose: Sets currently selected map in the map combobox
//-----------------------------------------------------------------------------
void CServerFinderDialog::SetMap(const char *mapName)
{
	for (int i = 0; i < m_pMapList->GetItemCount(); i++)
	{
		if (!m_pMapList->IsItemIDValid(i))
			continue;

		if (!stricmp(m_pMapList->GetItemUserData(i)->GetString("mapname"), mapName))
		{
			m_pMapList->ActivateItem(i);
			break;
		}
	}
}

int clampToPlayerCount(int violator)
{
	return clamp(violator, MIN_PLAYERS, (MAX_PLAYERS - 1));
}

void CServerFinderDialog::SetParams()
{
	const char* startMap = m_pSavedData->GetString("map", "");
	if (startMap[0])
	{
		// select the map if its non-random.
		bool bIsRandom = m_pSavedData->GetBool("israndommap", false);
		if (!bIsRandom)
		{
			SetMap(startMap);
		}
	}

	const char* startMax = m_pSavedData->GetString("MaxPlayers", "");
	if (startMax[0])
	{
		m_pOptions->m_iMaxPlayers = clampToPlayerCount(atoi(startMax));
	}

	const char* startRandomCrits = m_pSavedData->GetString("RandomCrits", "");
	if (startRandomCrits[0])
	{
		m_pOptions->m_eRandomCrits = (ERandCritsOption)atoi(startRandomCrits);
	}

	const char* startDamageSpread = m_pSavedData->GetString("DamageSpread", "");
	if (startDamageSpread[0])
	{
		m_pOptions->m_eDamageSpread = (EDamageSpreadOption)atoi(startDamageSpread);
	}

	const char* startRespawnTimes = m_pSavedData->GetString("RespawnTimes", "");
	if (startRespawnTimes[0])
	{
		m_pOptions->m_eRespawnTimes = (ERespawnTimes)atoi(startRespawnTimes);
	}
}

void CServerFinderDialog::GetOptions()
{
	int maxPlayers = clampToPlayerCount(m_pOptions->m_iMaxPlayers);
	if (maxPlayers > 0)
	{
		char szCount[16];
		Q_snprintf(szCount, sizeof(szCount), "%i", maxPlayers);
		m_maxPlayers->SetText(szCount);
	}

	m_pRandCrits->ActivateItem((int)m_pOptions->m_eRandomCrits);
	m_pDmgSpread->ActivateItem((int)m_pOptions->m_eDamageSpread);
	m_pRespawnTimes->ActivateItem((int)m_pOptions->m_eRespawnTimes);
}
#endif

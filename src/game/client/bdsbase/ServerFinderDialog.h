//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INSTANTACTIONGAMEDIALOG_H
#define INSTANTACTIONGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>

#ifdef BDSBASE_ALLOW_SERVERFINDER

//TF prefers a higher min player count. cap it to 6 so at least 3v3 is possible.
#if defined(TF_CLIENT_DLL)
#define MIN_PLAYERS 6
#else
#define MIN_PLAYERS 2
#endif

#if defined(TF_CLIENT_DLL)
enum ERandCritsOption
{
#if defined(QUIVER_DLL)
	eRandCritsNo,
	eRandCritsYes,
#elif defined(TF_CLIENT_DLL)
	eRandCritsYes,
	eRandCritsNo,
#endif
	eRandCritsDontCare
};
#endif

enum EDamageSpreadOption
{
	eDamageSpreadNo,
	eDamageSpreadYes,
	eDamageSpreadDontCare
};

enum ERespawnTimes
{
	eRespawnTimesDefault,
	eRespawnTimesInstant,
	eRespawnTimesDontCare
};

class ServerFinderOptions_t
{
public:
	ServerFinderOptions_t()
	{
#if defined(QUIVER_DLL)
		m_eRandomCrits = eRandCritsNo;
#elif defined(TF_CLIENT_DLL)
		m_eRandomCrits = eRandCritsYes;
#endif

#if defined(TF_CLIENT_DLL)
		m_eDamageSpread = eDamageSpreadNo;
#endif

		m_eRespawnTimes = eRespawnTimesDefault;
		m_iMaxPlayers = 16;
	}

#if defined(TF_CLIENT_DLL)
	ERandCritsOption m_eRandomCrits;
	EDamageSpreadOption m_eDamageSpread;
#endif

	ERespawnTimes m_eRespawnTimes;

	int m_iMaxPlayers;
};

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CServerFinderDialog : public vgui::Frame, public ISteamMatchmakingServerListResponse
{
	DECLARE_CLASS_SIMPLE( CServerFinderDialog,  vgui::Frame );

public:
	CServerFinderDialog(vgui::Panel *parent);
	~CServerFinderDialog();
	
	// returns currently entered information about the server
	void SetMap(const char *name);
	void GetOptions();
	void SetParams();
	bool IsRandomMapSelected();
	const char *GetMapName();

	//
	// ISteamMatchmakingServerListResponse overrides
	//

	virtual void ServerResponded(HServerListRequest hRequest, int iServer);
	virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);

	virtual void ServerResponded(gameserveritem_t& server);

private:
	virtual void OnCommand( const char *command );
	virtual void OnClose();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	
	void BeginSearch();
	void OnSearchFailure();
	void SaveOptionSelection(bool reload = true);
	void LoadMapList();
	void LoadMaps( const char *pszPathID );

	void DestroyServerListRequest()
	{
		if (m_hServerListRequest)
		{
			steamapicontext->SteamMatchmakingServers()->ReleaseRequest(m_hServerListRequest);
			m_hServerListRequest = NULL;
		}
	}

	HServerListRequest m_hServerListRequest;

	vgui::ComboBox *m_pMapList;
	vgui::TextEntry* m_maxPlayers;
#if defined(TF_CLIENT_DLL)
	vgui::ComboBox *m_pRandCrits;
	vgui::ComboBox* m_pDmgSpread;
#endif
	vgui::ComboBox* m_pRespawnTimes;
	ServerFinderOptions_t *m_pOptions;

	// for loading/saving game config
	KeyValues *m_pSavedData;
};

#endif
#endif

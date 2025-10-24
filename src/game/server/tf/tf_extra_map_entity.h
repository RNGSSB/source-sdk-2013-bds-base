//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#ifndef EXTRA_MAP_ENTITY_H
#define EXTRA_MAP_ENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

#define ENTITYROCKET_DEFAULT_MODEL	"models/props_skybox/grocket_001.mdl"
#define ENTITYCARRIER_DEFAULT_MODEL	"models/props_skybox/skybox_carrier.mdl"
#define ENTITYSIGN_DEFAULT_MODEL	"models/props_teaser/update_billboard001.mdl"
#define ENTITYSAUCER_DEFAULT_MODEL	"models/props_teaser/saucer.mdl"

DECLARE_AUTO_LIST( IExtraMapEntityAutoList );
class CExtraMapEntity : public CBaseAnimating, public IExtraMapEntityAutoList
{
	DECLARE_CLASS( CExtraMapEntity, CBaseAnimating );
public:
	DECLARE_DATADESC();

	virtual void	Spawn( void ) OVERRIDE;
	virtual void	Precache( void ) OVERRIDE;
	virtual const char *GetDefaultModel( void ) = 0;
	virtual void	PrepareModelName( const char *szModelName );

	static void		SpawnExtraModel( void );

	virtual bool	ShouldAnimate( void ){ return false; }
	void			AnimThink( void );

#ifdef BDSBASE
	virtual bool	ShouldSpawn(void) { return true; }
#endif

protected:
	virtual void	Precache_Internal( void );

private:
	static const char *ValidateKeyName( const char *pszEntName );
};

#ifdef BDSBASE
extern ConVar tf_spawn_extra_models_rocket;
extern ConVar tf_spawn_extra_models_carrier;
extern ConVar tf_spawn_extra_models_sign;
extern ConVar tf_spawn_extra_models_saucer;
#endif

class CExtraMapEntity_Rocket : public CExtraMapEntity
{
	DECLARE_CLASS( CExtraMapEntity_Rocket, CExtraMapEntity );
public:
	virtual void	Spawn( void ) OVERRIDE;
	virtual const char *GetDefaultModel( void ) OVERRIDE { return ENTITYROCKET_DEFAULT_MODEL; }
#ifdef BDSBASE
	virtual bool	ShouldSpawn(void) { return tf_spawn_extra_models_rocket.GetBool(); }
#endif

protected:
	virtual void	Precache_Internal( void ) OVERRIDE;
};

class CExtraMapEntity_Carrier : public CExtraMapEntity
{
	DECLARE_CLASS( CExtraMapEntity_Carrier, CExtraMapEntity );
public:
	virtual void	Spawn( void ) OVERRIDE;
	virtual const char *GetDefaultModel( void ) OVERRIDE { return ENTITYCARRIER_DEFAULT_MODEL; }
#ifdef BDSBASE
	virtual bool	ShouldSpawn(void) { return tf_spawn_extra_models_carrier.GetBool(); }
#endif
};

class CExtraMapEntity_Sign : public CExtraMapEntity
{
	DECLARE_CLASS( CExtraMapEntity_Sign, CExtraMapEntity );
public:
	virtual void	Spawn( void ) OVERRIDE;
	virtual const char *GetDefaultModel( void ) OVERRIDE { return ENTITYSIGN_DEFAULT_MODEL; }
#ifdef BDSBASE
	virtual bool	ShouldSpawn(void) { return tf_spawn_extra_models_sign.GetBool(); }
#endif
};

class CExtraMapEntity_Saucer : public CExtraMapEntity
{
	DECLARE_CLASS( CExtraMapEntity_Saucer, CExtraMapEntity );
public:
	virtual void	Spawn( void ) OVERRIDE;
	virtual const char *GetDefaultModel( void ) OVERRIDE{ return ENTITYSAUCER_DEFAULT_MODEL; }
	virtual bool	ShouldAnimate( void ){ return true; }
#ifdef BDSBASE
	virtual bool	ShouldSpawn(void) { return tf_spawn_extra_models_saucer.GetBool(); }
#endif
};

#endif // EXTRA_MAP_ENTITY_H



//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#ifndef TF_WEAPON_BONESAW_H
#define TF_WEAPON_BONESAW_H
#ifdef _WIN32
#pragma once
#endif

#include "tf_weaponbase_melee.h"
#if defined(QUIVER_DLL)
#include "tf_point_manager.h"
#endif

#ifdef CLIENT_DLL
#include "tf_weapon_medigun.h"

#define CTFBonesaw C_TFBonesaw
#endif

enum bonesaw_weapontypes_t
{
	BONESAW_DEFAULT = 0,
	BONESAW_UBER_ONHIT,
	BONESAW_UBER_SAVEDONDEATH,
	BONESAW_RADIUSHEAL,
	BONESAW_TONGS,
#if defined(QUIVER_DLL)
	BONESAW_DISEASE,
#endif
};

//=============================================================================
//
// Bonesaw class.
//
class CTFBonesaw : public CTFWeaponBaseMelee
{
public:
	DECLARE_CLASS( CTFBonesaw, CTFWeaponBaseMelee );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CTFBonesaw() {}
	virtual void		Activate( void );
	virtual int			GetWeaponID( void ) const { return TF_WEAPON_BONESAW; }

	virtual void		SecondaryAttack();

#if defined(QUIVER_DLL)
	virtual void		Precache() OVERRIDE;
#endif

	virtual bool		DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt );
	int					GetBonesawType( void ) const		{ int iMode = 0; CALL_ATTRIB_HOOK_INT( iMode, set_weapon_mode ); return iMode; };

	virtual void		DoMeleeDamage( CBaseEntity* ent, trace_t& trace ) OVERRIDE;
	
	float				GetProgress( void ) { return 0.f; }
	const char*			GetEffectLabelText( void ) { return "#TF_ORGANS"; }

	float				GetBoneSawSpeedMod( void );

#ifdef GAME_DLL
	virtual void		OnPlayerKill( CTFPlayer *pVictim, const CTakeDamageInfo &info ) OVERRIDE;
#else
	virtual void		OnDataChanged( DataUpdateType_t updateType );
	void				UpdateChargePoseParam( void );
	virtual void		GetPoseParameters( CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM] );
	virtual void		UpdateAttachmentModels( void );
#endif

private:

#ifdef CLIENT_DLL
	int			m_iUberChargePoseParam;
	float		m_flChargeLevel;
#endif

	CTFBonesaw( const CTFBonesaw & ) {}
};

#if defined(QUIVER_DLL)
#define TF_DISEASE_LIFETIME	5.f
#define TF_DISEASE_POINT_RADIUS	35.f
#define TF_DISEASE_BLEED_TIME 15.f
#define TF_DISEASE_INFECTION_DELAY 15.f
#define TF_DISEASE_FALLRATE 5.f
#define TF_DISEASE_MOVE_DISTANCE 10.f
#define TF_DISEASE_BLEED_BASE_DMG_PERCENTAGE 0.025f

#ifdef CLIENT_DLL
#define CTFDiseaseManager C_TFDiseaseManager

struct disease_particle_t
{
	disease_particle_t();
	~disease_particle_t();

	int								m_nUniqueID;
	CSmartPtr<CNewParticleEffect>	m_hParticleEffect;
	EHANDLE							m_hParticleOwner;
};

typedef CUtlVector< disease_particle_t > DiseaseParticle_t;
#endif // CLIENT_DLL

class CTFDiseaseManager : public CTFPointManager
{
	DECLARE_CLASS(CTFDiseaseManager, CTFPointManager);
public:
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CTFDiseaseManager();
	virtual void UpdateOnRemove(void) OVERRIDE;

#ifdef GAME_DLL
	static CTFDiseaseManager* Create(CBaseEntity* pOwner, CBaseEntity* pVictim);
	void AddDisease();
	virtual void OnCollide(CBaseEntity* pEnt, int iPointIndex) OVERRIDE;
#else
	virtual void PostDataUpdate(DataUpdateType_t updateType) OVERRIDE;
#endif // GAME_DLL

	virtual void Update() OVERRIDE;

protected:
	virtual Vector	GetInitialPosition() const OVERRIDE;

	virtual float	GetLifeTime() const OVERRIDE { return TF_DISEASE_LIFETIME; }
	virtual Vector	GetAdditionalVelocity(const tf_point_t* pPoint) const OVERRIDE { return vec3_origin; }
	virtual float	GetRadius(const tf_point_t* pPoint) const OVERRIDE { return TF_DISEASE_POINT_RADIUS; }
	virtual int		GetMaxPoints() const OVERRIDE { return 20; }
	virtual bool	ShouldIgnoreStartSolid(void) OVERRIDE { return true; }
	virtual bool	OnPointHitWall(tf_point_t* pPoint, Vector& vecNewPos, Vector& vecNewVelocity, const trace_t& tr, float flDT) OVERRIDE;

#ifdef GAME_DLL
	virtual bool	ShouldCollide(CBaseEntity* pEnt) const OVERRIDE;
#endif // GAME_DLL

private:
#ifdef GAME_DLL
	CUtlVector< EHANDLE > m_Touched;
#else
	DiseaseParticle_t m_hDiseaseParticleEffects;
#endif // GAME_DLL

	float m_flLastMoveUpdate;
	bool m_bKeepMovingPoints;
};
#endif

#endif // TF_WEAPON_BONESAW_H

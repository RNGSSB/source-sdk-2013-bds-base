
//========= Copyright Valve Corporation, All rights reserved. ============//
//
// TF Rocket
//
//=============================================================================
#include "cbase.h"
#include "tf_weaponbase.h"
#include "tf_projectile_rocket.h"
#include "tf_player.h"
#if defined(QUIVER_DLL)
#include "tf_gamerules.h"
#include "soundent.h"
#include "tf_fx.h"
#endif

//=============================================================================
//
// TF Rocket functions (Server specific).
//
#define ROCKET_MODEL "models/weapons/w_models/w_rocket.mdl"

LINK_ENTITY_TO_CLASS( tf_projectile_rocket, CTFProjectile_Rocket );
PRECACHE_REGISTER( tf_projectile_rocket );

IMPLEMENT_NETWORKCLASS_ALIASED( TFProjectile_Rocket, DT_TFProjectile_Rocket )

BEGIN_NETWORK_TABLE( CTFProjectile_Rocket, DT_TFProjectile_Rocket )
	SendPropBool( SENDINFO( m_bCritical ) ),
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_Rocket *CTFProjectile_Rocket::Create( CBaseEntity *pLauncher, const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pOwner, CBaseEntity *pScorer )
{
	CTFProjectile_Rocket *pRocket = static_cast<CTFProjectile_Rocket*>( CTFBaseRocket::Create( pLauncher, "tf_projectile_rocket", vecOrigin, vecAngles, pOwner ) );

	if ( pRocket )
	{
		pRocket->SetScorer( pScorer );
		pRocket->SetEyeBallRocket( false );
		pRocket->SetSpell( false );

		CTFWeaponBase *pWeapon = dynamic_cast< CTFWeaponBase * >( pLauncher );
		bool bDirectHit = pWeapon ? ( pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT ) : false;
		pRocket->SetDirectHit( bDirectHit );
	}

	return pRocket;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::Spawn()
{
	SetModel( ROCKET_MODEL );
	BaseClass::Spawn();
#ifdef BDSBASE
	CBaseEntity* pTFOwner = GetOwnerEntity();
	if (pTFOwner)
	{
		m_nSkin = (pTFOwner->GetTeamNumber() == TF_TEAM_BLUE) ? 1 : 0;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::Precache()
{
	int iModel = PrecacheModel( ROCKET_MODEL );
	PrecacheGibsForModel( iModel );
	PrecacheParticleSystem( "critical_rocket_blue" );
	PrecacheParticleSystem( "critical_rocket_red" );
	PrecacheParticleSystem( "eyeboss_projectile" );
	PrecacheParticleSystem( "rockettrail" );
	PrecacheParticleSystem( "rockettrail_RocketJumper" );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::SetScorer( CBaseEntity *pScorer )
{
	m_Scorer = pScorer;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CBasePlayer *CTFProjectile_Rocket::GetScorer( void )
{
	return dynamic_cast<CBasePlayer *>( m_Scorer.Get() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_Rocket::GetDamageType() 
{ 
	int iDmgType = BaseClass::GetDamageType();
	if ( m_bCritical )
	{
		iDmgType |= DMG_CRITICAL;
	}

	return iDmgType;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CTFProjectile_Rocket::GetDamageCustom()
{
	if ( m_bDirectHit )
	{
		return TF_DMG_CUSTOM_ROCKET_DIRECTHIT;
	}
	else if ( m_bEyeBallRocket )
	{
		return TF_DMG_CUSTOM_EYEBALL_ROCKET;
	}
	else if ( m_bSpell )
	{
		return TF_DMG_CUSTOM_SPELL_MONOCULUS;
	}
	else
		return BaseClass::GetDamageCustom();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::RocketTouch( CBaseEntity *pOther )
{
	BaseClass::RocketTouch( pOther );
		
	if (m_bCritical && pOther && pOther->IsPlayer())
	{		
		CTFPlayer *pHitPlayer = ToTFPlayer( pOther );
		int iHitPlayerTeamNumber = pHitPlayer->GetTeamNumber();
		int iRocketTeamNumber = BaseClass::GetTeamNumber();

		if (pHitPlayer->IsPlayerClass(TF_CLASS_HEAVYWEAPONS) && !pHitPlayer->m_Shared.InCond( TF_COND_INVULNERABLE)
			&& pHitPlayer->IsAlive() && iHitPlayerTeamNumber != iRocketTeamNumber)
		{
			pHitPlayer->AwardAchievement( ACHIEVEMENT_TF_HEAVY_SURVIVE_CROCKET );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Rocket was deflected.
//-----------------------------------------------------------------------------
void CTFProjectile_Rocket::Deflected( CBaseEntity *pDeflectedBy, Vector &vecDir )
{
	CTFPlayer *pTFDeflector = ToTFPlayer( pDeflectedBy );
	if ( !pTFDeflector )
		return;

	ChangeTeam( pTFDeflector->GetTeamNumber() );
	SetLauncher( pTFDeflector->GetActiveWeapon() );

#ifdef BDSBASE
	CTFPlayer* pOldOwner = ToTFPlayer(GetOwnerEntity());
	if (pOldOwner == nullptr)
	{
		CBaseObject* pBaseObject = dynamic_cast<CBaseObject*>(GetOwnerEntity());
		if (pBaseObject && pBaseObject->GetOwner())
		{
			pOldOwner = ToTFPlayer(pBaseObject->GetOwner());
		}
	}
#else
	CTFPlayer* pOldOwner = ToTFPlayer( GetOwnerEntity() );
#endif
	SetOwnerEntity( pTFDeflector );

	if ( pOldOwner )
	{
		pOldOwner->SpeakConceptIfAllowed( MP_CONCEPT_DEFLECTED, "projectile:1,victim:1" );
	}

	if ( pTFDeflector->m_Shared.IsCritBoosted() )
	{
		SetCritical( true );
	}

	CTFWeaponBase::SendObjectDeflectedEvent( pTFDeflector, pOldOwner, GetWeaponID(), this );

	IncrementDeflected();
	m_nSkin = ( GetTeamNumber() == TF_TEAM_BLUE ) ? 1 : 0;
}

#if defined(QUIVER_DLL)
#define MINI_ROCKETS_MODEL					"models/weapons/w_models/w_rocket_airstrike/w_rocket_airstrike.mdl"

BEGIN_DATADESC(CTFProjectile_RocketCluster)
DEFINE_THINKFUNC(ClusterThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(qf_projectile_rocketcluster, CTFProjectile_RocketCluster);
PRECACHE_REGISTER(qf_projectile_rocketcluster);

IMPLEMENT_NETWORKCLASS_ALIASED(TFProjectile_RocketCluster, DT_TFProjectile_RocketCluster)

BEGIN_NETWORK_TABLE(CTFProjectile_RocketCluster, DT_TFProjectile_RocketCluster)
END_NETWORK_TABLE()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFProjectile_RocketCluster* CTFProjectile_RocketCluster::Create(CBaseEntity* pLauncher, const Vector& vecOrigin, const QAngle& vecAngles, CBaseEntity* pOwner, CBaseEntity* pScorer)
{
	CTFProjectile_RocketCluster* pRocket = static_cast<CTFProjectile_RocketCluster*>(CTFBaseRocket::Create(pLauncher, "qf_projectile_rocketcluster", vecOrigin, vecAngles, pOwner));

	if (pRocket)
	{
		pRocket->SetScorer(pScorer);
		pRocket->SetEyeBallRocket(false);
		pRocket->SetSpell(false);

		CTFWeaponBase* pWeapon = dynamic_cast<CTFWeaponBase*>(pLauncher);
		bool bDirectHit = pWeapon ? (pWeapon->GetWeaponID() == TF_WEAPON_ROCKETLAUNCHER_DIRECTHIT) : false;
		pRocket->SetDirectHit(bDirectHit);
	}

	return pRocket;
}

void CTFProjectile_RocketCluster::Spawn()
{
	BaseClass::Spawn();
	ResetClusterTime();
}

void CTFProjectile_RocketCluster::ResetClusterTime()
{
	m_flCreationTime = gpGlobals->curtime;

	// Setup the think and touch functions (see CBaseEntity).
	SetThink(&CTFProjectile_RocketCluster::ClusterThink);
	SetNextThink(gpGlobals->curtime + 0.2);
}

void CTFProjectile_RocketCluster::Precache()
{
	BaseClass::Precache();

	UTIL_PrecacheOther("tf_projectile_rocket");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFProjectile_RocketCluster::ClusterThink(void)
{
	if (!IsInWorld())
	{
		Remove();
		return;
	}

	if (gpGlobals->curtime > (m_flCreationTime + 0.25f))
	{
		Cluster();
		return;
	}

	SetNextThink(gpGlobals->curtime + 0.2);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_RocketCluster::ExplodeMainRocket(trace_t* pTrace, int bitsDamageType)
{
	if (ShouldNotDetonate())
	{
		Destroy(true);
		return;
	}

	// Invisible.
	SetModelName(NULL_STRING);
	AddSolidFlags(FSOLID_NOT_SOLID);
	m_takedamage = DAMAGE_NO;

	// Pull out a bit.
	if (pTrace->fraction != 1.0)
	{
		SetAbsOrigin(pTrace->endpos + (pTrace->plane.normal * 1.0f));
	}

	// Play explosion sound and effect.
	Vector vecOrigin = GetAbsOrigin();
	CPVSFilter filter(vecOrigin);

	// Halloween Spell Effect Check
	int iHalloweenSpell = 0;
	int iCustomParticleIndex = INVALID_STRING_INDEX;
	item_definition_index_t ownerWeaponDefIndex = INVALID_ITEM_DEF_INDEX;
	// if the owner is a Sentry, Check its owner
	CBaseEntity* pPlayerOwner = GetOwnerPlayer();

	if (TF_IsHolidayActive(kHoliday_HalloweenOrFullMoon))
	{
		CALL_ATTRIB_HOOK_INT_ON_OTHER(pPlayerOwner, iHalloweenSpell, halloween_pumpkin_explosions);
		if (iHalloweenSpell > 0)
		{
			iCustomParticleIndex = GetParticleSystemIndex("halloween_explosion");
		}
	}

	int iNoSelfBlastDamage = 0;
	CTFWeaponBase* pWeapon = dynamic_cast<CTFWeaponBase*>(GetOriginalLauncher());
	if (pWeapon)
	{
		ownerWeaponDefIndex = pWeapon->GetAttributeContainer()->GetItem()->GetItemDefIndex();

		CALL_ATTRIB_HOOK_INT_ON_OTHER(pWeapon, iNoSelfBlastDamage, no_self_blast_dmg);
		if (iNoSelfBlastDamage)
		{
			iCustomParticleIndex = GetParticleSystemIndex("ExplosionCore_Wall_Jumper");
		}
	}

	int iLargeExplosion = 0;
	CALL_ATTRIB_HOOK_INT_ON_OTHER(pPlayerOwner, iLargeExplosion, use_large_smoke_explosion);
	if (iLargeExplosion > 0)
	{
		DispatchParticleEffect("explosionTrail_seeds_mvm", GetAbsOrigin(), GetAbsAngles());
		DispatchParticleEffect("fluidSmokeExpl_ring_mvm", GetAbsOrigin(), GetAbsAngles());
	}

	TE_TFExplosion(filter, 0.0f, vecOrigin, pTrace->plane.normal, GetWeaponID(), kInvalidEHandleExplosion, ownerWeaponDefIndex, SPECIAL3, iCustomParticleIndex);

	CSoundEnt::InsertSound(SOUND_COMBAT, vecOrigin, 1024, 3.0);

	// Damage.
	CBaseEntity* pAttacker = GetOwnerEntity();
	IScorer* pScorerInterface = dynamic_cast<IScorer*>(pAttacker);

	if (pScorerInterface && pScorerInterface->GetScorer())
	{
		pAttacker = pScorerInterface->GetScorer();
	}
	else if (pAttacker && pAttacker->GetOwnerEntity())
	{
		pAttacker = pAttacker->GetOwnerEntity();
	}

	float flRadius = GetRadius() * GetRadiusScale();

	if (pAttacker) // No attacker, deal no damage. Otherwise we could potentially kill teammates.
	{
		CTFPlayer* pTarget = ToTFPlayer(GetEnemy());
		if (pTarget)
		{
			// Rocket Specialist
			CheckForStunOnImpact(pTarget);

			RecordEnemyPlayerHit(pTarget, true);
		}

		CTakeDamageInfo info(this, pAttacker, GetOriginalLauncher(), vec3_origin, vecOrigin, GetDamage(), bitsDamageType, GetDamageCustom());
		CTFRadiusDamageInfo radiusinfo(&info, vecOrigin, flRadius, NULL, TF_ROCKET_RADIUS_FOR_RJS, GetDamageForceScale());
		TFGameRules()->RadiusDamage(radiusinfo);
	}

	// Don't decal players with scorch.
	if (pTrace->m_pEnt && !pTrace->m_pEnt->IsPlayer() && (iNoSelfBlastDamage == 0))
	{
		UTIL_DecalTrace(pTrace, "Scorch");
	}

	// Remove the rocket.
	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFProjectile_RocketCluster::Detonate(void)
{
	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	SetThink(NULL);

	vecSpot = GetAbsOrigin() + Vector(0, 0, 8);
	UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -32), MASK_SHOT_HULL, this, COLLISION_GROUP_NONE, &tr);

	ExplodeMainRocket(&tr, GetDamageType());
}

Vector g_vecFixedRktSpreadPellets[] =
{
	Vector(32,0,0),
	Vector(32,48,0),
	Vector(-32,0,0),
	Vector(-32,48,0)
};

void CTFProjectile_RocketCluster::Cluster()
{
	CTFWeaponBase* pWeapon = dynamic_cast<CTFWeaponBase*>(GetOriginalLauncher());

	if (pWeapon)
	{
		Vector vecOrigin = GetAbsOrigin();

		// Damage.
		CBaseEntity* pAttacker = GetOwnerEntity();
		IScorer* pScorerInterface = dynamic_cast<IScorer*>(pAttacker);

		if (pScorerInterface && pScorerInterface->GetScorer())
		{
			pAttacker = pScorerInterface->GetScorer();
		}
		else if (pAttacker && pAttacker->GetOwnerEntity())
		{
			pAttacker = pAttacker->GetOwnerEntity();
		}

		//after we explode, spawn 4 sentry rockets from our position in a spread.
		//each rocket that shoots out has a portion of our damage, which includes any damage bonuses/penalties

		for (int i = 0; i < 4; i++)
		{
			// Get the shooting angles.
			Vector vecShootForward, vecShootRight, vecShootUp;
			AngleVectors(GetAbsAngles(), &vecShootForward, &vecShootRight, &vecShootUp);

			float x = g_vecFixedRktSpreadPellets[i].x;
			float y = g_vecFixedRktSpreadPellets[i].y;

			Vector offset = ((x * vecShootRight) + (y * vecShootUp));
			Vector pos = (vecOrigin + offset);
			pos.z -= 16;

			CTFProjectile_Rocket* pProjectile = CTFProjectile_Rocket::Create(pWeapon, pos, GetAbsAngles(), pAttacker, pAttacker);

			if (pProjectile)
			{
				pProjectile->SetModel(MINI_ROCKETS_MODEL);
				pProjectile->SetCritical(IsCritical());
				pProjectile->SetRadiusScale(0.5f);
				pProjectile->SetScorer(pAttacker);
				pProjectile->SetDamage(GetDamage() / 2);
			}
		}
	}

	Detonate();
}

void CTFProjectile_RocketCluster::Deflected(CBaseEntity* pDeflectedBy, Vector& vecDir)
{
	SetThink(NULL);
	ResetClusterTime();
	BaseClass::Deflected(pDeflectedBy, vecDir);
}
#endif

//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "tf_weapon_bonesaw.h"
#include "tf_weapon_medigun.h"
#include "tf_gamerules.h"
#ifdef GAME_DLL
#include "tf_player.h"
#ifdef QUIVER_DLL
#include "func_respawnroom.h"
#endif
#else
#include "c_tf_player.h"
#endif

#define UBERSAW_CHARGE_POSEPARAM		"syringe_charge_level"
#define VITASAW_CHARGE_PER_HIT 0.15f

//=============================================================================
//
// Weapon Bonesaw tables.
//

IMPLEMENT_NETWORKCLASS_ALIASED( TFBonesaw, DT_TFWeaponBonesaw )

BEGIN_NETWORK_TABLE( CTFBonesaw, DT_TFWeaponBonesaw )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFBonesaw )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_bonesaw, CTFBonesaw );
PRECACHE_WEAPON_REGISTER( tf_weapon_bonesaw );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::Activate( void )
{
	BaseClass::Activate();
}
//-----------------------------------------------------------------------------
void CTFBonesaw::SecondaryAttack( void )
{
	CTFPlayer *pPlayer = GetTFPlayerOwner();
	if ( !pPlayer )
		return;

#ifdef GAME_DLL
	int iSpecialTaunt = 0;
	CALL_ATTRIB_HOOK_INT( iSpecialTaunt, special_taunt );
	if ( iSpecialTaunt )
	{
		pPlayer->Taunt( TAUNT_BASE_WEAPON );
		return;
	}
#endif
	BaseClass::SecondaryAttack();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFBonesaw::DefaultDeploy( char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt )
{
	if ( BaseClass::DefaultDeploy( szViewModel, szWeaponModel, iActivity, szAnimExt ) )
	{
#ifdef CLIENT_DLL
		UpdateChargePoseParam();
#endif
		return true;
	}

	return false;
}

#if defined(QUIVER_DLL)
#ifdef GAME_DLL
void Disease(CTFPlayer* pTFOwner, CTFPlayer* pVictimPlayer)
{
	if (pVictimPlayer && !pVictimPlayer->m_Shared.InCond(QF_COND_INFECTED) && !pVictimPlayer->m_Shared.IsInvulnerable() && !pVictimPlayer->m_Shared.InCond(TF_COND_PHASE) && !pVictimPlayer->m_Shared.InCond(TF_COND_PASSTIME_INTERCEPTION))
	{
		float flBleedDmg = TF_DISEASE_BLEED_BASE_DMG_PERCENTAGE * pVictimPlayer->GetMaxHealth();
		CTFWeaponBase* pWeapon = pTFOwner->GetActiveTFWeapon();
		if (pWeapon)
		{
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flBleedDmg, mult_dmg);
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flBleedDmg, mult_dmg_infection_bleeding);
		}

		float flBleedTime = TF_DISEASE_BLEED_TIME;

		if (pWeapon)
		{
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flBleedTime, infection_bleeding_duration);
		}

		pVictimPlayer->m_Shared.MakeBleed(pTFOwner, NULL, flBleedTime, flBleedDmg);
		CTFDiseaseManager* pChildDisease = CTFDiseaseManager::Create(pTFOwner, pVictimPlayer);
		if (pChildDisease)
		{
			pChildDisease->AddDisease();
		}

		float flInfectionDelay = TF_DISEASE_INFECTION_DELAY;

		if (pWeapon)
		{
			CALL_ATTRIB_HOOK_FLOAT_ON_OTHER(pWeapon, flInfectionDelay, infection_delay);
		}

		pVictimPlayer->m_Shared.AddCond(QF_COND_INFECTED, flInfectionDelay);
	}
}
#endif
#endif

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::DoMeleeDamage( CBaseEntity* ent, trace_t& trace )
{
	if ( !TFGameRules() || !TFGameRules()->IsTruceActive() )
	{
		if ( ent && ent->IsPlayer() )
		{
			CTFPlayer *pTFOwner = ToTFPlayer( GetOwnerEntity() );
			if ( pTFOwner && pTFOwner->GetTeamNumber() != ent->GetTeamNumber() )
			{
#ifdef BDSBASE
				float MeleeDamage = 0.f;
#ifndef CLIENT_DLL
				int iCustomDamage = GetDamageCustom();
				int iDmgType = DMG_MELEE | DMG_NEVERGIB | DMG_CLUB;
				MeleeDamage = GetMeleeDamage(ent, &iDmgType, &iCustomDamage);
#endif
				if (!ToTFPlayer(ent)->m_Shared.InCond(TF_COND_DISGUISED) ||
					(ent->GetHealth() - MeleeDamage <= 0.f))
				{
					int iDecaps = pTFOwner->m_Shared.GetDecapitations() + 1;

					int iTakeHeads = 0;
					CALL_ATTRIB_HOOK_INT(iTakeHeads, add_head_on_hit);
					if (iTakeHeads)
					{
						// We hit a target, take a head
						pTFOwner->m_Shared.SetDecapitations(iDecaps);
						pTFOwner->TeamFortress_SetSpeed();
					}

					float flPreserveUber = 0.f;
					CALL_ATTRIB_HOOK_FLOAT(flPreserveUber, ubercharge_preserved_on_spawn_max);
					if (flPreserveUber)
					{
						pTFOwner->m_Shared.SetDecapitations(iDecaps);

						CWeaponMedigun* pMedigun = dynamic_cast<CWeaponMedigun*>(pTFOwner->Weapon_OwnsThisID(TF_WEAPON_MEDIGUN));
						if (pMedigun)
						{
							pMedigun->SetChargeLevelToPreserve((iDecaps * VITASAW_CHARGE_PER_HIT));
						}
					}

#if defined(QUIVER_DLL)
#ifdef GAME_DLL
					int iTHEDISEASE = 0;
					CALL_ATTRIB_HOOK_INT(iTHEDISEASE, theres_this_disease_going_around_killing_people_and_i_think_i_have_it_uh_oh);
					if (iTHEDISEASE)
					{
						CTFPlayer* pVictimPlayer = ToTFPlayer(ent);

						if (pVictimPlayer)
						{
							Disease(pTFOwner, pVictimPlayer);

							// if the dude is healing, spread the infection to the patient too!
							CTFPlayer* pPatient = ToTFPlayer(pVictimPlayer->MedicGetHealTarget());

							if (pPatient)
							{
								Disease(pTFOwner, pPatient);
							}
						}
					}
#endif
#endif
				}
#else
				int iDecaps = pTFOwner->m_Shared.GetDecapitations() + 1;

				int iTakeHeads = 0;
				CALL_ATTRIB_HOOK_INT( iTakeHeads, add_head_on_hit );
				if ( iTakeHeads )
				{
					// We hit a target, take a head
					pTFOwner->m_Shared.SetDecapitations( iDecaps );
					pTFOwner->TeamFortress_SetSpeed();
				}

				float flPreserveUber = 0.f;
				CALL_ATTRIB_HOOK_FLOAT( flPreserveUber, ubercharge_preserved_on_spawn_max );
				if ( flPreserveUber )
				{
					pTFOwner->m_Shared.SetDecapitations( iDecaps );

					CWeaponMedigun *pMedigun = dynamic_cast< CWeaponMedigun* >( pTFOwner->Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) );
					if ( pMedigun )
					{
						pMedigun->SetChargeLevelToPreserve( ( iDecaps * VITASAW_CHARGE_PER_HIT ) );
					}
				}
#endif
			}
		}
	}

	BaseClass::DoMeleeDamage( ent, trace );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFBonesaw::GetBoneSawSpeedMod( void ) 
{ 
	const int MAX_HEADS_FOR_SPEED = 10;
	// Calculate Speed based on heads
	CTFPlayer *pPlayer = ToTFPlayer( GetOwnerEntity() );

	int iTakeHeads = 0;
	CALL_ATTRIB_HOOK_INT( iTakeHeads, add_head_on_hit );
	if ( pPlayer && iTakeHeads )
	{
		int iDecaps = Min( MAX_HEADS_FOR_SPEED, pPlayer->m_Shared.GetDecapitations() );
		return 1.f + (iDecaps * 0.05f);
	}
	return 1.f; 
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::OnPlayerKill( CTFPlayer *pVictim, const CTakeDamageInfo &info )
{
	BaseClass::OnPlayerKill( pVictim, info );

	CTFPlayer *pTFOwner = ToTFPlayer( GetOwnerEntity() );
	if ( !pTFOwner )
		return;

	int iTakeHeads = 0;
	CALL_ATTRIB_HOOK_INT( iTakeHeads, add_head_on_kill );
	if ( iTakeHeads )
	{
		int nOrgans = pTFOwner->m_Shared.GetDecapitations() + 1;
		pTFOwner->m_Shared.SetDecapitations( nOrgans );

		CWeaponMedigun *pMedigun = dynamic_cast< CWeaponMedigun* >( pTFOwner->Weapon_OwnsThisID( TF_WEAPON_MEDIGUN ) );
		if ( pMedigun )
		{
			pMedigun->SetChargeLevelToPreserve( ( nOrgans * VITASAW_CHARGE_PER_HIT ) );
		}
	}
}
#endif

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	UpdateChargePoseParam(); 
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::UpdateAttachmentModels( void )
{
	BaseClass::UpdateAttachmentModels();

	if ( m_hViewmodelAttachment )
	{
		m_iUberChargePoseParam = m_hViewmodelAttachment->LookupPoseParameter( m_hViewmodelAttachment->GetModelPtr(), UBERSAW_CHARGE_POSEPARAM );
	}
	else
	{
		m_iUberChargePoseParam = LookupPoseParameter( GetModelPtr(), UBERSAW_CHARGE_POSEPARAM );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::UpdateChargePoseParam( void )
{
	if ( m_iUberChargePoseParam >= 0 )
	{
		CTFPlayer *pTFPlayer = ToTFPlayer( GetOwner() );
		if ( pTFPlayer && pTFPlayer->IsPlayerClass( TF_CLASS_MEDIC ) )
		{
			CWeaponMedigun *pMedigun = (CWeaponMedigun *)pTFPlayer->Weapon_OwnsThisID( TF_WEAPON_MEDIGUN );
			if ( pMedigun )
			{
				m_flChargeLevel = pMedigun->GetChargeLevel();

				// On the local client, we push the pose parameters onto the attached model
				if ( m_hViewmodelAttachment )
				{
					m_hViewmodelAttachment->SetPoseParameter( m_iUberChargePoseParam, pMedigun->GetChargeLevel() );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFBonesaw::GetPoseParameters( CStudioHdr *pStudioHdr, float poseParameter[MAXSTUDIOPOSEPARAM] )
{
	if ( !pStudioHdr )
		return;

	BaseClass::GetPoseParameters( pStudioHdr, poseParameter );

	if ( m_iUberChargePoseParam >= 0 )
	{
		poseParameter[m_iUberChargePoseParam] = m_flChargeLevel;
	}
}

#endif

#if defined(QUIVER_DLL)

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFBonesaw::Precache()
{
	// TEMP
	PrecacheParticleSystem("infected_blue");
	PrecacheParticleSystem("infected_red");

	BaseClass::Precache();
}

IMPLEMENT_NETWORKCLASS_ALIASED(TFDiseaseManager, DT_TFDiseaseManager);

BEGIN_NETWORK_TABLE(CTFDiseaseManager, DT_TFDiseaseManager)
END_NETWORK_TABLE()

BEGIN_DATADESC(CTFDiseaseManager)
END_DATADESC()

LINK_ENTITY_TO_CLASS(qf_disease_manager_find_a_fucking_doctor_please, CTFDiseaseManager);

#ifdef CLIENT_DLL
disease_particle_t::disease_particle_t()
{
	m_nUniqueID = 0;
	m_hParticleEffect = NULL;
	m_hParticleOwner = NULL;
}

disease_particle_t::~disease_particle_t()
{
	Assert(m_hParticleOwner);
	if (m_hParticleEffect.GetObject() && m_hParticleOwner)
	{
		m_hParticleOwner->ParticleProp()->StopEmission(m_hParticleEffect.GetObject());
	}
}
#endif // CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFDiseaseManager::CTFDiseaseManager()
{
	m_flLastMoveUpdate = -1.f;
	m_bKeepMovingPoints = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFDiseaseManager::UpdateOnRemove(void)
{
#ifdef CLIENT_DLL
	m_hDiseaseParticleEffects.RemoveAll();
#endif // CLIENT_DLL

	BaseClass::UpdateOnRemove();
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CTFDiseaseManager* CTFDiseaseManager::Create(CBaseEntity* pOwner, CBaseEntity* pVictim)
{
	CTFDiseaseManager* pGasManager = static_cast<CTFDiseaseManager*>(CBaseEntity::Create("qf_disease_manager_find_a_fucking_doctor_please", pVictim->WorldSpaceCenter(), vec3_angle, pOwner));
	if (pGasManager)
	{
		// Initialize the owner.
		pGasManager->SetOwnerEntity(pOwner);

		// Set team.
		pGasManager->ChangeTeam(pOwner->GetTeamNumber());
	}

	return pGasManager;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFDiseaseManager::AddDisease()
{
	int iCurrentTick = TIME_TO_TICKS(gpGlobals->curtime);
	while (CanAddPoint())
	{
		if (!AddPoint(iCurrentTick))
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFDiseaseManager::ShouldCollide(CBaseEntity* pEnt) const
{
	if (!pEnt->IsPlayer())
		return false;

	if (pEnt->GetTeamNumber() == GetTeamNumber())
		return false;

	if (TFGameRules() && TFGameRules()->IsTruceActive())
		return false;

	if (m_Touched.Find(pEnt) != m_Touched.InvalidIndex())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFDiseaseManager::OnCollide(CBaseEntity* pEnt, int iPointIndex)
{
	CTFPlayer* pTFPlayer = assert_cast<CTFPlayer*>(pEnt);
	CTFPlayer* pOwner = ToTFPlayer(GetOwnerEntity());

	if (pOwner)
	{
		if (pTFPlayer)
		{
			Disease(pOwner, pTFPlayer);

			// if the dude is healing, spread the infection to the patient too!
			CTFPlayer* pPatient = ToTFPlayer(pTFPlayer->MedicGetHealTarget());

			if (pPatient)
			{
				Disease(pOwner, pPatient);
			}
		}
	}

	m_Touched.AddToTail(pTFPlayer);
}
#endif // GAME_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFDiseaseManager::Update()
{
	TM_ZONE_DEFAULT(TELEMETRY_LEVEL0)

	bool bUpdatePoints = (GetPointVec().Count() > 0);

	// remove any gas points that shouldn't be included anymore
	FOR_EACH_VEC_BACK(GetPointVec(), i)
	{
		bool bShouldRemove = false;
		// expired
		if (gpGlobals->curtime > GetPointVec()[i]->m_flSpawnTime + GetPointVec()[i]->m_flLifeTime)
		{
			bShouldRemove = true;
		}

		// in water?
		int nContents = UTIL_PointContents(GetPointVec()[i]->m_vecPosition);
		if ((nContents & MASK_WATER))
		{
			bShouldRemove = true;
		}

#ifdef GAME_DLL
		//in a spawnroom while in a pre-game state?
		bool bIsBeforeRound = (TFGameRules()->State_Get() == GR_STATE_PREGAME ||
			TFGameRules()->State_Get() == GR_STATE_PREROUND ||
			TFGameRules()->InSetup() ||
			TFGameRules()->IsInWaitingForPlayers());
		if (bIsBeforeRound)
		{
			if (PointsCrossRespawnRoomVisualizer(GetInitialPosition(), GetPointVec()[i]->m_vecPosition))
			{
				bShouldRemove = true;
			}

			if (PointInRespawnRoom(NULL, GetPointVec()[i]->m_vecPosition))
			{
				bShouldRemove = true;
			}
		}
#endif

		if (bShouldRemove)
		{
			RemovePoint(i);
		}
	}

	if (GetPointVec().Count() <= 0)
	{
		if (bUpdatePoints)
		{
			SetContextThink(&CTFDiseaseManager::SUB_Remove, gpGlobals->curtime, "RemoveThink");
		}

		return;
	}

	// store the previous position
	FOR_EACH_VEC(GetPointVec(), i)
	{
		GetPointVec()[i]->m_vecPrevPosition = GetPointVec()[i]->m_vecPosition;
	}

	float flMinDistanceApart = (TF_DISEASE_POINT_RADIUS * 1.9f);
	if (m_flLastMoveUpdate <= 1.f)
	{
		m_flLastMoveUpdate = gpGlobals->curtime;
	}
	float flDelta = gpGlobals->curtime - m_flLastMoveUpdate;
	m_flLastMoveUpdate = gpGlobals->curtime;

	bool bAnyoneMoved = false;

	if (m_bKeepMovingPoints)
	{
		// move them down
		FOR_EACH_VEC(GetPointVec(), i)
		{
			Vector vecDownDir = flDelta * Vector(0.f, 0.f, -TF_DISEASE_POINT_RADIUS);

			trace_t	tr;
			Vector vecPos = GetPointVec()[i]->m_vecPosition + Vector(0.f, 0.f, -TF_DISEASE_POINT_RADIUS); // bottom of the sphere
			UTIL_TraceLine(vecPos, vecPos + vecDownDir, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
			if (tr.fraction != 1.f)
				continue;

			GetPointVec()[i]->m_vecPosition += (vecDownDir * TF_DISEASE_FALLRATE);
			bAnyoneMoved = true;
		}

		// now have them separate
		FOR_EACH_VEC(GetPointVec(), point1)
		{
			int nNeighborCount = 0;
			Vector vecResult(0, 0, 0);

			FOR_EACH_VEC(GetPointVec(), point2)
			{
				if (point1 == point2)
					continue;

				Vector vecDist = GetPointVec()[point2]->m_vecPosition - GetPointVec()[point1]->m_vecPosition;
				if (vecDist.Length() < flMinDistanceApart)
				{
					vecResult.x += (GetPointVec()[point2]->m_vecPosition.x - GetPointVec()[point1]->m_vecPosition.x);
					vecResult.y += (GetPointVec()[point2]->m_vecPosition.y - GetPointVec()[point1]->m_vecPosition.y);
					vecResult.z += (GetPointVec()[point2]->m_vecPosition.z - GetPointVec()[point1]->m_vecPosition.z);

					nNeighborCount++;
				}
			}

			if (nNeighborCount > 0)
			{
				vecResult.x /= nNeighborCount;
				vecResult.y /= nNeighborCount;
				vecResult.z /= nNeighborCount;

				Vector vecNewDir = vecResult.Normalized();
				vecNewDir *= (TF_DISEASE_MOVE_DISTANCE * -1.f);

				// can we move there?
				trace_t	tr;
				UTIL_TraceLine(GetPointVec()[point1]->m_vecPosition, GetPointVec()[point1]->m_vecPosition + vecNewDir, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
				if (tr.fraction != 1.f)
					continue;

				GetPointVec()[point1]->m_vecPosition += vecNewDir;
				bAnyoneMoved = true;
			}
		}
	}

	// 	if ( bAnyoneMoved )
	// 	{
	// 		FOR_EACH_VEC( GetPointVec(), i )
	// 		{
	// #ifdef GAME_DLL
	// 			DevMsg( "SERVER MOVE: index [%d], pos[ %f %f %f ]\n", i, XYZ( GetPointVec()[i]->m_vecPosition ) );
	// #else
	// 			DevMsg( "CLIENT MOVE: index [%d], pos[ %f %f %f ]\n", i, XYZ( GetPointVec()[i]->m_vecPosition ) );
	// #endif
	// 		}
	// 	}

	m_bKeepMovingPoints = bAnyoneMoved;
#ifdef GAME_DLL
	// update bounds if necessary
	if (bAnyoneMoved)
	{
		Vector vHullMin(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
		Vector vHullMax(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
		FOR_EACH_VEC(GetPointVec(), i)
		{
			tf_point_t* pPoint = GetPointVec()[i];
			float flRadius = GetRadius(pPoint);
			Vector vExtent(flRadius, flRadius, flRadius);
			VectorMin(vHullMin, pPoint->m_vecPosition - vExtent, vHullMin);
			VectorMax(vHullMax, pPoint->m_vecPosition + vExtent, vHullMax);
		}

		Vector vExtent = 0.5f * (vHullMax - vHullMin);
		Vector vOrigin = vHullMin + vExtent;
		SetAbsOrigin(vOrigin);
		UTIL_SetSize(this, -vExtent, vExtent);
	}
#endif // GAME_DLL

#ifdef CLIENT_DLL
	// clean up gas that has gone away
	FOR_EACH_VEC_BACK(m_hDiseaseParticleEffects, nIndex)
	{
		int nUniqueID = m_hDiseaseParticleEffects[nIndex].m_nUniqueID;
		bool bFound = false;
		FOR_EACH_VEC(GetPointVec(), i)
		{
			if (GetPointVec()[i]->m_nPointIndex == nUniqueID)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			m_hDiseaseParticleEffects.Remove(nIndex);
		}
	}

	// add/update gas that's still around
	FOR_EACH_VEC(GetPointVec(), i)
	{
		int iFoundIndex = -1;
		FOR_EACH_VEC_BACK(m_hDiseaseParticleEffects, iIndex)
		{
			if (m_hDiseaseParticleEffects[iIndex].m_nUniqueID == GetPointVec()[i]->m_nPointIndex)
			{
				iFoundIndex = iIndex;
				break;
			}
		}

		if (iFoundIndex == -1)
		{
			// create a new effect
			CSmartPtr<CNewParticleEffect> pEffect = ParticleProp()->Create((GetTeamNumber() == TF_TEAM_BLUE) ? "infected_blue" : "infected_red", PATTACH_CUSTOMORIGIN, 0);
			if (pEffect.IsValid() && pEffect->IsValid())
			{
				pEffect->SetControlPoint(0, GetPointVec()[i]->m_vecPosition);

				int nNewIndex = m_hDiseaseParticleEffects.AddToTail();
				m_hDiseaseParticleEffects[nNewIndex].m_hParticleOwner = this;
				m_hDiseaseParticleEffects[nNewIndex].m_hParticleEffect = pEffect;
				m_hDiseaseParticleEffects[nNewIndex].m_nUniqueID = GetPointVec()[i]->m_nPointIndex;
			}
		}
		else
		{
			m_hDiseaseParticleEffects[iFoundIndex].m_hParticleEffect->SetControlPoint(0, GetPointVec()[i]->m_vecPosition);
		}
	}
#endif // CLIENT_DLL
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
Vector CTFDiseaseManager::GetInitialPosition() const
{
	float flDistance = TF_DISEASE_POINT_RADIUS;

#ifdef GAME_DLL
	Vector vecOrigin = GetAbsOrigin();
#else
	Vector vecOrigin = GetNetworkOrigin();
#endif

	return (vecOrigin + Vector(m_randomStream.RandomFloat(flDistance * -1.f, flDistance), m_randomStream.RandomFloat(flDistance * -1.f, flDistance), 0));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CTFDiseaseManager::OnPointHitWall(tf_point_t* pPoint, Vector& vecNewPos, Vector& vecNewVelocity, const trace_t& tr, float flDT)
{
	// set pos to some offset from the surface
	vecNewPos = tr.endpos + ((GetRadius(pPoint) + 5) * tr.plane.normal);
	vecNewVelocity = pPoint->m_vecVelocity;

	return false;
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CTFDiseaseManager::PostDataUpdate(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		BaseClass::PostDataUpdate(updateType);
		return;
	}

	// intentionally skip the BaseClass version after we're created
	CBaseEntity::PostDataUpdate(updateType);
}
#endif // CLIENT_DLL
#endif
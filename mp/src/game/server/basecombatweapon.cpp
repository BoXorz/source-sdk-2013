
#include "cbase.h"
#include "ai_basenpc.h"
#include "animation.h"
#include "basecombatweapon.h"
#include "player.h"			// For gEvilImpulse101 / CBasePlayer
#include "gamerules.h"		// For g_pGameRules
#include <KeyValues.h>
#include "ammodef.h"
#include "baseviewmodel.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "engine/IEngineSound.h"
#include "sendproxy.h"
#include "tier1/strtools.h"
#include "vphysics/constraints.h"
#include "npcevent.h"
#include "igamesystem.h"
#include "collisionutils.h"
#include "iservervehicle.h"
#include "func_break.h"

#include "MSS_gamerules.h"
#include "MSS_item_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern int	gEvilImpulse101;		// In Player.h

// -----------------------------------------
//	Sprite Index info
// -----------------------------------------
short		g_sModelIndexLaser;			// holds the index for the laser beam
const char	*g_pModelNameLaser = "sprites/laserbeam.vmt";
short		g_sModelIndexLaserDot;		// holds the index for the laser beam dot
short		g_sModelIndexFireball;		// holds the index for the fireball
short		g_sModelIndexSmoke;			// holds the index for the smoke cloud
short		g_sModelIndexWExplosion;	// holds the index for the underwater explosion
short		g_sModelIndexBubbles;		// holds the index for the bubbles model
short		g_sModelIndexBloodDrop;		// holds the sprite index for the initial blood
short		g_sModelIndexBloodSpray;	// holds the sprite index for splattered blood

//ConVar weapon_showproficiency( "weapon_showproficiency", "0" );
extern ConVar ai_debug_shoot_positions;

// BOXBOX Precache item script stuff
void W_Precache(void)
{
	PrecacheFileItemInfoDatabase( filesystem, g_pGameRules->GetEncryptionKey() );

/*
#ifdef HL1_DLL
	g_sModelIndexWExplosion = CBaseEntity::PrecacheModel ("sprites/WXplo1.vmt");// underwater fireball
	g_sModelIndexBloodSpray = CBaseEntity::PrecacheModel ("sprites/bloodspray.vmt"); // initial blood
	g_sModelIndexBloodDrop = CBaseEntity::PrecacheModel ("sprites/blood.vmt"); // splattered blood 
	g_sModelIndexLaserDot = CBaseEntity::PrecacheModel("sprites/laserdot.vmt");
#endif // HL1_DLL
*/

	g_sModelIndexFireball = CBaseEntity::PrecacheModel ("sprites/zerogxplode.vmt");// fireball

	g_sModelIndexSmoke = CBaseEntity::PrecacheModel ("sprites/steam1.vmt");// smoke
	g_sModelIndexBubbles = CBaseEntity::PrecacheModel ("sprites/bubble.vmt");//bubbles
	g_sModelIndexLaser = CBaseEntity::PrecacheModel( (char *)g_pModelNameLaser );

	PrecacheParticleSystem( "blood_impact_red_01" );
	PrecacheParticleSystem( "blood_impact_green_01" );
	PrecacheParticleSystem( "blood_impact_yellow_01" );

	CBaseEntity::PrecacheModel ("effects/bubble.vmt");//bubble trails

//	CBaseEntity::PrecacheModel("models/weapons/w_bullet.mdl");

//	CBaseEntity::PrecacheScriptSound( "BaseCombatWeapon.WeaponDrop" ); // BOXBOX TODO figure out item drop sounds
//	CBaseEntity::PrecacheScriptSound( "BaseCombatWeapon.WeaponMaterialize" ); // BOXBOX weapons don't respawn
}

//-----------------------------------------------------------------------------
// Purpose: Transmit weapon data
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::UpdateTransmitState( void)
{
	// If the weapon is being carried by a CBaseCombatCharacter, let the combat character do the logic
	// about whether or not to transmit it.
	if ( GetOwner() )
	{	
		return SetTransmitState( FL_EDICT_PVSCHECK );
	}
	else
	{
		// If it's just lying around, then use CBaseEntity's visibility test to see if it should be sent.
		return BaseClass::UpdateTransmitState();
	}
}


void CBaseCombatWeapon::Operator_FrameUpdate( CBaseCombatCharacter *pOperator )
{
	StudioFrameAdvance( ); // animate

	if ( IsSequenceFinished() )
	{
		if ( SequenceLoops() )
		{
			// animation does loop, which means we're playing subtle idle. Might need to fidget.
			int iSequence = SelectWeightedSequence( GetActivity() );
			if ( iSequence != ACTIVITY_NOT_AVAILABLE )
			{
				ResetSequence( iSequence );	// Set to new anim (if it's there)
			}
		}
#if 0
		else
		{
			// animation that just ended doesn't loop! That means we just finished a fidget
			// and should return to our heaviest weighted idle (the subtle one)
			SelectHeaviestSequence( GetActivity() );
		}
#endif
	}

	// Animation events are passed back to the weapon's owner/operator
	DispatchAnimEvents( pOperator );

	// Update and dispatch the viewmodel events
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	CBaseViewModel *vm = pOwner->GetViewModel( m_nViewModelIndex );
	
	if ( vm != NULL )
	{
		vm->StudioFrameAdvance();
		vm->DispatchAnimEvents( this );
	}
}


void CBaseCombatWeapon::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		if ( pEvent->event == EVENT_WEAPON_MELEE_HIT )
		{
			HandleAnimEventMeleeHit( pEvent, pOperator );
		}
		else if ( pEvent->event == AE_WPN_PLAYWPNSOUND )
		{
			int iSnd = GetWeaponSoundFromString(pEvent->options);
			if ( iSnd != -1 )
			{
				WeaponSound( (WeaponSound_t)iSnd );
			}
		}
	}

	DevWarning( 2, "Unhandled animation event %d from %s --> %s\n", pEvent->event, pOperator->GetClassname(), GetClassname() );
}

// NOTE: This should never be called when a character is operating the weapon.  Animation events should be
// routed through the character, and then back into CharacterAnimEvent() 
void CBaseCombatWeapon::HandleAnimEvent( animevent_t *pEvent )
{
	//If the player is receiving this message, pass it through
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner != NULL )
	{
		Operator_HandleAnimEvent( pEvent, pOwner );
	}
}

void CBaseCombatWeapon::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	Vector vecEnd;
	VectorMA( pOperator->Weapon_ShootPosition(), 50, vecDirection, vecEnd );
	CBaseEntity *pHurt = pOperator->CheckTraceHullAttack( pOperator->Weapon_ShootPosition(), vecEnd, 
		Vector(-16,-16,-16), Vector(36,36,36), GetDamageForActivity( GetActivity() ), DMG_CLUB, 0.75 );
	
	// did I hit someone?
	if ( pHurt )
	{
		// play sound
		WeaponSound( MELEE_HIT );

		// Fake a trace impact, so the effects work out like a player's crowbaw
		trace_t traceHit;
		UTIL_TraceLine( pOperator->Weapon_ShootPosition(), pHurt->GetAbsOrigin(), MASK_SHOT_HULL, pOperator, COLLISION_GROUP_NONE, &traceHit );
		ImpactEffect( traceHit );
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Weapons ignore other weapons when LOS tracing
//-----------------------------------------------------------------------------
class CWeaponLOSFilter : public CTraceFilterSkipTwoEntities
{
	DECLARE_CLASS( CWeaponLOSFilter, CTraceFilterSkipTwoEntities );
public:
	CWeaponLOSFilter( IHandleEntity *pHandleEntity, IHandleEntity *pHandleEntity2, int collisionGroup ) :
	  CTraceFilterSkipTwoEntities( pHandleEntity, pHandleEntity2, collisionGroup ), m_pVehicle( NULL )
	{
		// If the tracing entity is in a vehicle, then ignore it
		if ( pHandleEntity != NULL )
		{
			CBaseCombatCharacter *pBCC = ((CBaseEntity *)pHandleEntity)->MyCombatCharacterPointer();
			if ( pBCC != NULL )
			{
				m_pVehicle = pBCC->GetVehicleEntity();
			}
		}
	}
	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
	{
		CBaseEntity *pEntity = (CBaseEntity *)pServerEntity;

		if ( pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON )
			return false;

		// Don't collide with the tracing entity's vehicle (if it exists)
		if ( pServerEntity == m_pVehicle )
			return false;

		if ( pEntity->GetHealth() > 0 )
		{
			CBreakable *pBreakable = dynamic_cast<CBreakable *>(pEntity);
			if ( pBreakable  && pBreakable->IsBreakable() && pBreakable->GetMaterialType() == matGlass)
			{
				return false;
			}
		}

		return BaseClass::ShouldHitEntity( pServerEntity, contentsMask );
	}

private:
	CBaseEntity *m_pVehicle;
};

//-----------------------------------------------------------------------------
// Purpose: Check the weapon LOS for an owner at an arbitrary position
//			If bSetConditions is true, LOS related conditions will also be set
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	// --------------------
	// Check for occlusion
	// --------------------
	CAI_BaseNPC* npcOwner = m_hOwner.Get()->MyNPCPointer();

	// Find its relative shoot position
	Vector vecRelativeShootPosition;
	VectorSubtract( npcOwner->Weapon_ShootPosition(), npcOwner->GetAbsOrigin(), vecRelativeShootPosition );
	Vector barrelPos = ownerPos + vecRelativeShootPosition;

	// FIXME: If we're in a vehicle, we need some sort of way to handle shooting out of them

	// Use the custom LOS trace filter
	CWeaponLOSFilter traceFilter( m_hOwner.Get(), npcOwner->GetEnemy(), COLLISION_GROUP_BREAKABLE_GLASS );
	trace_t tr;
	UTIL_TraceLine( barrelPos, targetPos, MASK_SHOT, &traceFilter, &tr );

	// See if we completed the trace without interruption
	if ( tr.fraction == 1.0 )
	{
		if ( ai_debug_shoot_positions.GetBool() )
		{
			NDebugOverlay::Line( barrelPos, targetPos, 0, 255, 0, false, 1.0 );
		}

		return true;
	}

	CBaseEntity	*pHitEnt = tr.m_pEnt;

	CBasePlayer *pEnemyPlayer = ToBasePlayer( npcOwner->GetEnemy() );

	// is player in a vehicle? if so, verify vehicle is target and return if so (so npc shoots at vehicle)
	if ( pEnemyPlayer && pEnemyPlayer->IsInAVehicle() )
	{
		// Ok, player in vehicle, check if vehicle is target we're looking at, fire if it is
		// Also, check to see if the owner of the entity is the vehicle, in which case it's valid too.
		// This catches vehicles that use bone followers.
		CBaseEntity	*pVehicle  = pEnemyPlayer->GetVehicle()->GetVehicleEnt();
		if ( pHitEnt == pVehicle || pHitEnt->GetOwnerEntity() == pVehicle )
			return true;
	}

	// Hitting our enemy is a success case
	if ( pHitEnt == npcOwner->GetEnemy() )
	{
		if ( ai_debug_shoot_positions.GetBool() )
		{
			NDebugOverlay::Line( barrelPos, targetPos, 0, 255, 0, false, 1.0 );
		}

		return true;
	}

	// If a vehicle is blocking the view, grab its driver and use that as the combat character
	CBaseCombatCharacter *pBCC;
	IServerVehicle *pVehicle = pHitEnt->GetServerVehicle();
	if ( pVehicle )
	{
		pBCC = pVehicle->GetPassenger( );
	}
	else
	{
		pBCC = ToBaseCombatCharacter( pHitEnt );
	}

	if ( pBCC ) 
	{
		if ( npcOwner->IRelationType( pBCC ) == D_HT )
			return true;

		if ( bSetConditions )
		{
			npcOwner->SetCondition( COND_WEAPON_BLOCKED_BY_FRIEND );
		}
	}
	else if ( bSetConditions )
	{
		npcOwner->SetCondition( COND_WEAPON_SIGHT_OCCLUDED );
		npcOwner->SetEnemyOccluder( pHitEnt );

		if( ai_debug_shoot_positions.GetBool() )
		{
			NDebugOverlay::Line( tr.startpos, tr.endpos, 255, 0, 0, false, 1.0 );
		}
	}

	return false;
}

/* BOXBOX removing
int CBaseCombatWeapon::WeaponRangeAttack1Condition( float flDot, float flDist )
{
 	if ( UsesPrimaryAmmo() && !HasPrimaryAmmo() )
 	{
 		return COND_NO_PRIMARY_AMMO;
 	}
 	else if ( flDist < m_fMinRange1) 
 	{
 		return COND_TOO_CLOSE_TO_ATTACK;
 	}
 	else if (flDist > m_fMaxRange1) 
 	{
 		return COND_TOO_FAR_TO_ATTACK;
 	}
 	else if (flDot < 0.5) 	// UNDONE: Why check this here? Isn't the AI checking this already?
 	{
 		return COND_NOT_FACING_ATTACK;
 	}

 	return COND_CAN_RANGE_ATTACK1;
}

//-----------------------------------------------------------------------------
// Purpose: Base class always returns not bits
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	// currently disabled
	return COND_NONE;

	if ( m_bReloadsSingly )
	{
		if (m_iClip2 <=0)
		{
			return COND_NO_SECONDARY_AMMO;
		}
		else if ( flDist < m_fMinRange2) 
		{
			return COND_TOO_CLOSE_TO_ATTACK;
		}
		else if (flDist > m_fMaxRange2) 
		{
			return COND_TOO_FAR_TO_ATTACK;
		}
		else if (flDot < 0.5) 
		{
			return COND_NOT_FACING_ATTACK;
		}
		return COND_CAN_RANGE_ATTACK2;
	}

	return COND_NONE;
}
*/

int CBaseCombatWeapon::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	return COND_NONE;
}

int CBaseCombatWeapon::WeaponMeleeAttack2Condition( float flDot, float flDist )
{
	return COND_NONE;
}

/* BOXBOX removing
void CBaseCombatWeapon::Delete( void )
{
	SetTouch( NULL );
	// FIXME: why doesn't this just remove itself now?
	SetThink(&CBaseCombatWeapon::SUB_Remove);
	SetNextThink( gpGlobals->curtime + 0.1f );
}
*/
void CBaseCombatWeapon::DestroyItem( void )
{
	CBaseCombatCharacter *pOwner = m_hOwner.Get();

	if ( pOwner )
	{
		// if attached to a player, remove. 
		pOwner->RemovePlayerItem( this );
	}

	Kill( );
}

void CBaseCombatWeapon::Kill( void )
{
	SetTouch( NULL );	// FIXME: why doesn't this just remove itself now?
	SetThink(&CBaseCombatWeapon::SUB_Remove);
	SetNextThink( gpGlobals->curtime + 0.1f );
}

// BOXBOX Setup for falling to ground
void CBaseCombatWeapon::FallInit( void )
{
	SetModel( GetWorldModel() );
	VPhysicsDestroyObject();

	if ( !VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false ) )
	{
		SetMoveType( MOVETYPE_FLYGRAVITY );
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_TRIGGER );
	}
	else
	{
#if !defined( CLIENT_DLL )
		// Constrained start?
		if ( HasSpawnFlags( SF_WEAPON_START_CONSTRAINED ) )
		{
			//Constrain the weapon in place
			IPhysicsObject *pReferenceObject, *pAttachedObject;
			
			pReferenceObject = g_PhysWorldObject;
			pAttachedObject = VPhysicsGetObject();

			if ( pReferenceObject && pAttachedObject )
			{
				constraint_fixedparams_t fixed;
				fixed.Defaults();
				fixed.InitWithCurrentObjectState( pReferenceObject, pAttachedObject );
				
				fixed.constraint.forceLimit	= lbs2kg( 10000 );
				fixed.constraint.torqueLimit = lbs2kg( 10000 );

				m_pConstraint = physenv->CreateFixedConstraint( pReferenceObject, pAttachedObject, NULL, fixed );

				m_pConstraint->SetGameData( (void *) this );
			}
		}
#endif //CLIENT_DLL
	}	

	SetPickupTouch();
	
	SetThink( &CBaseCombatWeapon::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Items that have just spawned run this think to catch them when 
//			they hit the ground. Once we're sure that the object is grounded, 
//			we change its solid type to trigger and set it in a large box that 
//			helps the player get it.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FallThink ( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	bool shouldMaterialize = false;
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( pPhysics )
	{
		shouldMaterialize = pPhysics->IsAsleep();
	}
	else
	{
		shouldMaterialize = (GetFlags() & FL_ONGROUND) ? true : false;
	}

	if ( shouldMaterialize )
	{
/* BOXBOX removing
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if ( GetOwnerEntity() )
		{
			EmitSound( "BaseCombatWeapon.WeaponDrop" );
		}
*/
		Materialize(); 
	}

}


void CBaseCombatWeapon::Materialize( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		RemoveEffects( EF_NODRAW );
//		DoMuzzleFlash();
	}

//	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
//	{
		VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );
		SetMoveType( MOVETYPE_VPHYSICS );
//	}

	SetPickupTouch();

	SetThink (NULL);
}

/* BOXBOX removing
void CBaseCombatWeapon::AttemptToMaterialize( void ) // BOXBOX changing this
{
//	float time = g_pGameRules->FlWeaponTryRespawn( this );

//	if ( time == 0 )
//	{
		Materialize();
//		return;
//	}

//	SetNextThink( gpGlobals->curtime + time );
}

//-----------------------------------------------------------------------------
// Purpose: Weapon has been picked up, should it respawn?
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::CheckRespawn( void ) // BOXBOX no weapon respawning in MSS
{
	switch ( g_pGameRules->WeaponShouldRespawn( this ) )
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}

}
*/
class CWeaponList : public CAutoGameSystem
{
public:
	CWeaponList( char const *name ) : CAutoGameSystem( name )
	{
	}


	virtual void LevelShutdownPostEntity()  
	{ 
		m_list.Purge();
	}

	void AddWeapon( CBaseCombatWeapon *pWeapon )
	{
		m_list.AddToTail( pWeapon );
	}

	void RemoveWeapon( CBaseCombatWeapon *pWeapon )
	{
		m_list.FindAndRemove( pWeapon );
	}
	CUtlLinkedList< CBaseCombatWeapon * > m_list;
};

CWeaponList g_WeaponList( "CWeaponList" );

void OnBaseCombatWeaponCreated( CBaseCombatWeapon *pWeapon )
{
	g_WeaponList.AddWeapon( pWeapon );
}

void OnBaseCombatWeaponDestroyed( CBaseCombatWeapon *pWeapon )
{
	g_WeaponList.RemoveWeapon( pWeapon );
}

int CBaseCombatWeapon::GetAvailableWeaponsInBox( CBaseCombatWeapon **pList, int listMax, const Vector &mins, const Vector &maxs )
{
	// linear search all weapons
	int count = 0;
	int index = g_WeaponList.m_list.Head();
	while ( index != g_WeaponList.m_list.InvalidIndex() )
	{
		CBaseCombatWeapon *pWeapon = g_WeaponList.m_list[index];
		// skip any held weapon
		if ( !pWeapon->GetOwner() )
		{
			// restrict to mins/maxs
			if ( IsPointInBox( pWeapon->GetAbsOrigin(), mins, maxs ) )
			{
				if ( count < listMax )
				{
					pList[count] = pWeapon;
					count++;
				}
			}
		}
		index = g_WeaponList.m_list.Next( index );
	}

	return count;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBaseCombatWeapon::ObjectCaps( void )
{ 
	int caps = BaseClass::ObjectCaps();
	if ( !IsFollowingEntity() && !HasSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP) )
	{
		caps |= FCAP_IMPULSE_USE;
	}

	return caps;
}

// BOXBOX TODO eliminate bumpweapon, and implement the item pickup system(viewmodel reaches out, use button needed to grab item)
void CBaseCombatWeapon::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	
	if ( pPlayer )
	{
		m_OnPlayerUse.FireOutput( pActivator, pCaller );

		//
		// Bump the weapon to try equipping it before picking it up physically. This is
		// important in a few spots in the game where the player could potentially +use pickup
		// and then THROW AWAY a vital weapon, rendering them unable to continue the game.
		//
		if ( pPlayer->BumpWeapon( this ) )
		{
			OnPickedUp( pPlayer );
		}
		else
		{
			pPlayer->PickupObject( this );
		}
	}
}


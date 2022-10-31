//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Used at the bottom of maps where objects should fall away to infinity
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "triggers.h"
#include "asw_shareddefs.h"
#include "asw_triggers.h"
#include "asw_inhabitable_npc.h"

BEGIN_DATADESC( CASW_Trigger_Fall )

	DEFINE_KEYFIELD( m_nFlatDamage, FIELD_INTEGER, "FlatDamage" ),
	DEFINE_KEYFIELD( m_flVelocityDamageScale, FIELD_FLOAT, "VelocityDamageScale" ),
	DEFINE_KEYFIELD( m_bApplyDamageOnLanding, FIELD_BOOLEAN, "ApplyDamageOnLanding" ),

	// Function Pointers
	DEFINE_FUNCTION( FallTouch ),

	// Outputs
	DEFINE_OUTPUT( m_OnFallingObject, "OnFallingObject" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( asw_trigger_fall, CASW_Trigger_Fall ); // old and busted
LINK_ENTITY_TO_CLASS( trigger_asw_fall, CASW_Trigger_Fall ); // the new hotness


CASW_Trigger_Fall::CASW_Trigger_Fall()
{
	m_nFlatDamage = 0;
	m_flVelocityDamageScale = 1.0f;
	m_bApplyDamageOnLanding = true;
}


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CASW_Trigger_Fall::Spawn( void )
{
	BaseClass::Spawn();

	if ( ClassMatches( "asw_trigger_fall" ) )
	{
		// for compatibility, ignore which spawn flags and keyvalues are set in Hammer
		AddSpawnFlags( SF_TRIGGER_ALLOW_ALL );
		m_nFlatDamage = 500;
		m_flVelocityDamageScale = 0.0f;
		m_bApplyDamageOnLanding = false;
	}

	InitTrigger();
	SetTouch( &CASW_Trigger_Fall::FallTouch );
}

//-----------------------------------------------------------------------------
// Purpose: Make the object fall away
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CASW_Trigger_Fall::FallTouch( CBaseEntity *pOther )
{
	if ( !PassesTriggerFilters( pOther ) )
	{
		return;
	}

	if ( CASW_Inhabitable_NPC *pNPC = dynamic_cast<CASW_Inhabitable_NPC *>( pOther ) )
	{
		if ( pNPC->IsAlive() == false )
			return;

		pNPC->SetFallTrigger( this );
	}

	// Fire our output
	m_OnFallingObject.FireOutput( pOther, this );
}

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list
 **/

// Trace against Actors/Components which provide interactions.
#define ZODIAC_TRACE_CHANNEL_INTERACTION					ECC_GameTraceChannel1

// Trace used by weapons, will hit physics assets instead of capsules
#define ZODIAC_TRACE_CHANNEL_WEAPON						ECC_GameTraceChannel2

// Trace used by weapons, will hit pawn capsules instead of physics assets
#define ZODIAC_TRACE_CHANNEL_WEAPON_CAPSULE				ECC_GameTraceChannel3

// Trace used by weapons, will trace through multiple pawns rather than stopping on the first hit
#define ZODIAC_TRACE_CHANNEL_WEAPON_MULTI					ECC_GameTraceChannel4

#define ZODIAC_TRACE_CHANNEL_TRAVERSAL_PAWN_IGNORED					ECC_GameTraceChannel6
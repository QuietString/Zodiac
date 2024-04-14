// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacPawnData.h"
#include "GameFramework/Pawn.h"
//#include "AbilitySystem/ZodiacAbilitySet.h"

UZodiacPawnData::UZodiacPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnClass = nullptr;
	InputConfig = nullptr;
	//DefaultCameraMode = nullptr;
}

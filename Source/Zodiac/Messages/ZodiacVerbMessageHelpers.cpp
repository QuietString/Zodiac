// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacVerbMessageHelpers.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameplayEffectTypes.h"
#include "Messages/ZodiacMessageTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacVerbMessageHelpers)

//////////////////////////////////////////////////////////////////////
// FZodiacVerbMessage

FString FZodiacVerbMessage::ToString() const
{
	FString HumanReadableMessage;
	FZodiacVerbMessage::StaticStruct()->ExportText(HumanReadableMessage, this, nullptr, nullptr, PPF_None, nullptr);
	return HumanReadableMessage;
}

//////////////////////////////////////////////////////////////////////
// 

APlayerState* UZodiacVerbMessageHelpers::GetPlayerStateFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC->PlayerState;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS;
	}
	
	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		if (APlayerState* TargetPS = TargetPawn->GetPlayerState())
		{
			return TargetPS;
		}
	}
	return nullptr;
}

APlayerController* UZodiacVerbMessageHelpers::GetPlayerControllerFromObject(UObject* Object)
{
	if (APlayerController* PC = Cast<APlayerController>(Object))
	{
		return PC;
	}

	if (APlayerState* TargetPS = Cast<APlayerState>(Object))
	{
		return TargetPS->GetPlayerController();
	}

	if (APawn* TargetPawn = Cast<APawn>(Object))
	{
		return Cast<APlayerController>(TargetPawn->GetController());
	}

	return nullptr;
}

FGameplayCueParameters UZodiacVerbMessageHelpers::VerbMessageToCueParameters(const FZodiacVerbMessage& Message)
{
	FGameplayCueParameters Result;

	Result.OriginalTag = Message.Channel;
	Result.Instigator = Cast<AActor>(Message.Instigator);
	Result.EffectCauser = Cast<AActor>(Message.Target);
	Result.AggregatedSourceTags = Message.InstigatorTags;
	Result.AggregatedTargetTags = Message.TargetTags;
	Result.RawMagnitude = Message.Magnitude;

	return Result;
}

FZodiacVerbMessage UZodiacVerbMessageHelpers::CueParametersToVerbMessage(const FGameplayCueParameters& Params)
{
	FZodiacVerbMessage Result;
	
	Result.Channel = Params.OriginalTag;
	Result.Instigator = Params.Instigator.Get();
	Result.Target = Params.EffectCauser.Get();
	Result.InstigatorTags = Params.AggregatedSourceTags;
	Result.TargetTags = Params.AggregatedTargetTags;
	Result.Magnitude = Params.RawMagnitude;

	return Result;
}


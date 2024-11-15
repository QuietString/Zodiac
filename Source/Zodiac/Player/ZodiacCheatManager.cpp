// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacCheatManager.h"

#include "GameplayTagContainer.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacPlayerController.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Development/ZodiacDeveloperSettings.h"
#include "Engine/Console.h"
#include "System/ZodiacAssetManager.h"
#include "System/ZodiacGameData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCheatManager)

DEFINE_LOG_CATEGORY(LogZodiacCheat);

namespace ZodiacCheat
{
	static const FName NAME_Fixed = FName(TEXT("Fixed"));
	
	static bool bEnableDebugCameraCycling = false;
	static FAutoConsoleVariableRef CVarEnableDebugCameraCycling(
		TEXT("ZodiacCheat.EnableDebugCameraCycling"),
		bEnableDebugCameraCycling,
		TEXT("If true then you can cycle the debug camera while running the game."),
		ECVF_Cheat);
};

UZodiacCheatManager::UZodiacCheatManager()
{
}

void UZodiacCheatManager::InitCheatManager()
{
	Super::InitCheatManager();

#if WITH_EDITOR
	if (GIsEditor)
	{
		APlayerController* PC = GetOuterAPlayerController();
		
		for (const FZodiacCheatToRun& CheatRow : GetDefault<UZodiacDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnCheatManagerCreated)
			{
				PC->ConsoleCommand(CheatRow.Cheat, true);
			}
		}
	}
#endif
}

void UZodiacCheatManager::CheatOutputText(const FString& TextToOutput)
{
#if USING_CHEAT_MANAGER
	// Output to the console.
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->ViewportConsole)
	{
		GEngine->GameViewport->ViewportConsole->OutputText(TextToOutput);
	}

	// Output to log.
	UE_LOG(LogZodiacCheat, Display, TEXT("%s"), *TextToOutput);
#endif
}

void UZodiacCheatManager::Cheat(const FString& Msg)
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		ZodiacPC->ServerCheat(Msg.Left(128));
	}
}

void UZodiacCheatManager::AddTagToSelf(FString TagName)
{
	FGameplayTag Tag = ZodiacGameplayTags::FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = GetPlayerAbilitySystemComponent())
		{
			ZodiacASC->AddDynamicTagGameplayEffect(Tag);
		}
	}
	else
	{
		UE_LOG(LogZodiacCheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

void UZodiacCheatManager::DamageSelf(float DamageAmount)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetPlayerAbilitySystemComponent())
	{
		ApplySetByCallerDamage(ZodiacASC, DamageAmount);
	}
}

void UZodiacCheatManager::God()
{
	auto ApplyDynamicTagForGodMode = [](UZodiacAbilitySystemComponent* ZodiacASC)
	{
		if (ZodiacASC)
		{
			const FGameplayTag Tag = ZodiacGameplayTags::Cheat_GodMode;
			const bool bHasTag = ZodiacASC->HasMatchingGameplayTag(Tag);

			if (bHasTag)
			{
				ZodiacASC->RemoveDynamicTagGameplayEffect(Tag);
			}
			else
			{
				ZodiacASC->AddDynamicTagGameplayEffect(Tag);
			}
		}
	};

	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(ZodiacPC->GetPawn()))
		{
			for (AZodiacHeroCharacter*& Hero : HostCharacter->GetHeroes())
			{
				if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
				{
					ApplyDynamicTagForGodMode(ZodiacASC);
				}
			}
		}
		else if (UZodiacAbilitySystemComponent* ZodiacASC = ZodiacPC->GetZodiacAbilitySystemComponent())
		{
			ApplyDynamicTagForGodMode(ZodiacASC);
		}
	}
}

void UZodiacCheatManager::ApplySetByCallerDamage(UZodiacAbilitySystemComponent* ZodiacASC, float DamageAmount)
{
	check(ZodiacASC);

	TSubclassOf<UGameplayEffect> DamageGE = UZodiacAssetManager::GetSubclass(UZodiacGameData::Get().DamageGameplayEffect_SetByCaller);
	FGameplayEffectSpecHandle SpecHandle = ZodiacASC->MakeOutgoingSpec(DamageGE, 1.0f, ZodiacASC->MakeEffectContext());

	if (SpecHandle.IsValid())
	{
		SpecHandle.Data->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_Damage, DamageAmount);
		ZodiacASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}

UZodiacAbilitySystemComponent* UZodiacCheatManager::GetPlayerAbilitySystemComponent() const
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		return ZodiacPC->GetZodiacAbilitySystemComponent();
	}
	
	return nullptr;
}

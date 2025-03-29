// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com

#include "ZodiacCheatManager.h"

#include "AIController.h"
#include "EngineUtils.h"
#include "GameplayTagContainer.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacPlayerController.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "AI/ZodiacAIPawnSubsystem.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Character/ZodiacMonster.h"
#include "Development/ZodiacDeveloperSettings.h"
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

void UZodiacCheatManager::Cheat(const FString& Msg)
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		ZodiacPC->ServerCheat(Msg.Left(128));
	}
}

void UZodiacCheatManager::CheatAll(const FString& Msg)
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		ZodiacPC->ServerCheatAll(Msg.Left(128));
	}
}

void UZodiacCheatManager::AddTagToSelf(FString TagName)
{
	FGameplayTag Tag = ZodiacGameplayTags::FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		if (AZodiacHostCharacter* HostCharacter = GetHostCharacter())
		{
			for (auto& Hero : HostCharacter->GetHeroes())
			{
				if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
				{
					ToggleDynamicTag(ZodiacASC, Tag);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogZodiacCheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

void UZodiacCheatManager::DamageSelf(float DamageAmount)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetHeroAbilitySystemComponent())
	{
		ApplySetByCallerDamage(ZodiacASC, DamageAmount);
	}
}

void UZodiacCheatManager::InfiniteAmmo()
{
	if (AZodiacHostCharacter* HostCharacter = GetHostCharacter())
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
			{
				ToggleDynamicTag(ZodiacASC, ZodiacGameplayTags::Cheat_InfiniteAmmo);
			}
		}
	}
}

void UZodiacCheatManager::InfiniteUltimate()
{
	if (AZodiacHostCharacter* HostCharacter = GetHostCharacter())
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
			{
				ToggleDynamicTag(ZodiacASC, ZodiacGameplayTags::Cheat_InfiniteUltimate);
			}
		}
	}
}

void UZodiacCheatManager::God()
{
	if (AZodiacHostCharacter* HostCharacter = GetHostCharacter())
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
			{
				ToggleDynamicTag(ZodiacASC, ZodiacGameplayTags::Cheat_GodMode);
			}
		}
	}
}

void UZodiacCheatManager::PauseZombies()
{
	UWorld* World = GetWorld();
	check(World);
	UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);
	
	UZodiacAIPawnSubsystem* AISubsystem = GameInstance->GetSubsystem<UZodiacAIPawnSubsystem>();
	if (AISubsystem)
	{
		AISubsystem->PauseAllMonsters();
	}
}

void UZodiacCheatManager::ResumeZombies()
{
	UWorld* World = GetWorld();
	check(World);
	UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);
	
	UZodiacAIPawnSubsystem* AISubsystem = GameInstance->GetSubsystem<UZodiacAIPawnSubsystem>();
	if (AISubsystem)
	{
		AISubsystem->ResumeAllMonsters();
	}
}

void UZodiacCheatManager::AddTagToAllMonsters(FString TagName)
{
	FGameplayTag Tag = ZodiacGameplayTags::FindTagByString(TagName, true);
	if (Tag.IsValid())
	{
		for (TActorIterator<AZodiacMonster> It(GetWorld()); It; ++It)
		{
			if (AZodiacMonster* Monster = *It)
			{
				if (UZodiacAbilitySystemComponent* ZodiacASC = Monster->GetZodiacAbilitySystemComponent())
				{
					ZodiacASC->AddDynamicTagGameplayEffect(Tag);
				}
			}
		}
	}
	else
	{
		UE_LOG(LogZodiacCheat, Display, TEXT("AddTagToSelf: Could not find any tag matching [%s]."), *TagName);
	}
}

void UZodiacCheatManager::AllMonstersInvincible()
{
	for (TActorIterator<AZodiacMonster> It(GetWorld()); It; ++It)
	{
		if (AZodiacMonster* Monster = *It)
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = Monster->GetZodiacAbilitySystemComponent())
			{
				ZodiacASC->AddDynamicTagGameplayEffect(ZodiacGameplayTags::Status_Invincible);
			}
		}
	}
}

void UZodiacCheatManager::MonstersImmortal()
{
	for (TActorIterator<AZodiacMonster> It(GetWorld()); It; ++It)
	{
		if (AZodiacMonster* Monster = *It)
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = Monster->GetZodiacAbilitySystemComponent())
			{
				ToggleDynamicTag(ZodiacASC, ZodiacGameplayTags::Status_Immortal);
			}
		}
	}
}

void UZodiacCheatManager::ToggleMonsterAI()
{
	for (TActorIterator<AAIController> It(GetWorld()); It; ++It)
	{
		AAIController* AICon = *It;
		if (AICon && AICon->BrainComponent)
		{
			if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(AICon->BrainComponent))
			{
				if (BTComp->IsPaused())
				{
					BTComp->ResumeLogic(TEXT("CheatManager ToggleMonsterAI"));	
				}
				else
				{
					BTComp->PauseLogic(TEXT("CheatManager ToggleMonsterAI"));
				}
			}
		}
	}

	UE_LOG(LogZodiacCheat, Warning, TEXT("All AI Behavior Trees stopped."));
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

UZodiacAbilitySystemComponent* UZodiacCheatManager::GetHeroAbilitySystemComponent() const
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		return ZodiacPC->GetHeroAbilitySystemComponent();
	}
	
	return nullptr;
}

AZodiacHostCharacter* UZodiacCheatManager::GetHostCharacter() const
{
	if (AZodiacPlayerController* ZodiacPC = Cast<AZodiacPlayerController>(GetOuterAPlayerController()))
	{
		return ZodiacPC->GetHostCharacter();
	}
	
	return nullptr;
}

void UZodiacCheatManager::ToggleDynamicTag(UZodiacAbilitySystemComponent* ZodiacASC, FGameplayTag TagToToggle)
{
	const bool bHasTag = ZodiacASC->HasMatchingGameplayTag(TagToToggle);

	if (bHasTag)
	{
		ZodiacASC->RemoveDynamicTagGameplayEffect(TagToToggle);
	}
	else
	{
		ZodiacASC->AddDynamicTagGameplayEffect(TagToToggle);
	}
}
// the.quiet.string@gmail.com

#include "ZodiacPlayerState.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerState)

AZodiacPlayerState::AZodiacPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EZodiacPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHostAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AZodiacPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Params;
	Params.bIsPushBased = true;
	Params.RepNotifyCondition = REPNOTIFY_OnChanged;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, Params);

	DOREPLIFETIME(ThisClass, StatTags);
}

void AZodiacPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetPlayerController())
	{
		if (PC->IsLocalPlayerController())
		{
			ServerNotifyClientIsReady();	
		}
	}
}

void AZodiacPlayerState::ServerNotifyClientIsReady_Implementation()
{
	AbilitySystemComponent->AddLooseGameplayTag(ZodiacGameplayTags::Player_PlayReady);

	if (AZodiacHostCharacter* HostCharacter = GetPawn<AZodiacHostCharacter>())
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (UAbilitySystemComponent* ASC = Hero->GetAbilitySystemComponent())
			{
				ASC->AddLooseGameplayTag(ZodiacGameplayTags::Player_PlayReady);
			}
		}	
	}
}

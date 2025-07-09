// the.quiet.string@gmail.com

#include "ZodiacPlayerState.h"

#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Character/ZodiacPawnExtensionComponent.h"
#include "Character/Hero/ZodiacHeroData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerState)

const FName AZodiacPlayerState::NAME_ZodiacAbilityReady("ZodiacAbilitiesReady");

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

UZodiacAbilitySystemComponent* AZodiacPlayerState::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);
}

void AZodiacPlayerState::SetPawnData(const UZodiacHeroData* InPawnData)
{
	check(InPawnData);

	if (!HasAuthority())
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogZodiac, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	for (const UZodiacAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_ZodiacAbilityReady);

	ForceNetUpdate();
}

void AZodiacPlayerState::Reset()
{
	Super::Reset();
}

void AZodiacPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (UZodiacPawnExtensionComponent* PawnExtensionComponent = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(GetPawn()))
	{
		UE_LOG(LogZodiacFramework, Warning, TEXT("client init"));
		PawnExtensionComponent->CheckDefaultInitialization();
	}
}

void AZodiacPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
}

void AZodiacPlayerState::OnDeactivated()
{
	Super::OnDeactivated();
}

void AZodiacPlayerState::OnReactivated()
{
	Super::OnReactivated();
}

// the.quiet.string@gmail.com


#include "ZodiacPlayerState.h"

#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Net/UnrealNetwork.h"

AZodiacPlayerState::AZodiacPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EZodiacPlayerConnectionType::Player)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	UZodiacHealthSet* HealthSet = CreateDefaultSubobject<UZodiacHealthSet>(TEXT("HealthSet"));
	AbilitySystemComponent->AddAttributeSetSubobject(HealthSet);
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

UAbilitySystemComponent* AZodiacPlayerState::GetAbilitySystemComponent() const
{
	return GetZodiacAbilitySystemComponent();
}

void AZodiacPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
	
}

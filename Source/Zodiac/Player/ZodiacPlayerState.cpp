// the.quiet.string@gmail.com


#include "ZodiacPlayerState.h"

#include "Character/ZodiacPlayerCharacter.h"
#include "Net/UnrealNetwork.h"

AZodiacPlayerState::AZodiacPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EZodiacPlayerConnectionType::Player)
{

}

UAbilitySystemComponent* AZodiacPlayerState::GetAbilitySystemComponent() const
{
	if (AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetPawn()))
	{
		return PlayerCharacter->GetAbilitySystemComponent();
	}

	return nullptr;
}

void AZodiacPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	if (AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetPawn()))
	{
		PlayerCharacter->CheckReadyAndPlay();
	}
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

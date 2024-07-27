// the.quiet.string@gmail.com


#include "ZodiacPlayerState.h"

#include "Character/ZodiacHostCharacter.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerState)

AZodiacPlayerState::AZodiacPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MyPlayerConnectionType(EZodiacPlayerConnectionType::Player)
{

}

UAbilitySystemComponent* AZodiacPlayerState::GetAbilitySystemComponent() const
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetPawn()))
	{
		return HostCharacter->GetAbilitySystemComponent();
	}

	return nullptr;
}

void AZodiacPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
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

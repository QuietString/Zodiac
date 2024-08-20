// the.quiet.string@gmail.com


#include "ZodiacAIController.h"

#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAIController)

AZodiacAIController::AZodiacAIController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AZodiacAIController::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacAbilitySystemComponent* AZodiacAIController::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

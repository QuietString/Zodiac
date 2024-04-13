// the.quiet.string@gmail.com


#include "Character/ZodiacHealthComponent.h"

#include "ZodiacLogChannels.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"

UZodiacHealthComponent::UZodiacHealthComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)	
{
	HealthSet = nullptr;
}

void UZodiacHealthComponent::InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InASC)
{
	UE_LOG(LogTemp, Warning, TEXT("health asc initialized"));
	AActor* Owner = GetOwner();
	check(Owner);
	
	HealthSet = InASC->GetSet<UZodiacHealthSet>();
	if (!HealthSet)
	{
		UE_LOG(LogZodiac, Error, TEXT("ZodiacHealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}
	
	InASC->GetGameplayAttributeValueChangeDelegate(UZodiacHealthSet::GetHealthAttribute()).AddUObject(this, &ThisClass::OnHealthChanged);
}

void UZodiacHealthComponent::OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("health changed from %.1f to %.1f"), OldValue, NewValue);
}

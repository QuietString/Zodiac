// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Components/GameFrameworkComponent.h"
#include "ZodiacHealthComponent.generated.h"

class UZodiacHealthSet;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHealthComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UZodiacHealthComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeWithAbilitySystem(UZodiacAbilitySystemComponent* InASC);

protected:
	void OnHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
private:
	UPROPERTY()
	const UZodiacHealthSet* HealthSet;
};

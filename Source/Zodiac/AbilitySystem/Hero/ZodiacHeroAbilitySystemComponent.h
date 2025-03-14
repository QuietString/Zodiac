// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHeroAbilitySystemComponent.generated.h"


UCLASS()
class ZODIAC_API UZodiacHeroAbilitySystemComponent : public UZodiacAbilitySystemComponent
{
	GENERATED_BODY()

public:
	UZodiacAbilitySystemComponent* GetHostAbilitySystemComponent() const { return HostAbilitySystemComponent; }
	void SetHostAbilitySystemComponent(UZodiacAbilitySystemComponent* InASC);
	
protected:
	UPROPERTY()
	TObjectPtr<UZodiacAbilitySystemComponent> HostAbilitySystemComponent;
};

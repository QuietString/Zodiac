// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacHeroWidgetBase.h"
#include "ZodiacHealthBarWidget.generated.h"

class AZodiacHostCharacter;
class UZodiacHealthComponent;

UCLASS()
class ZODIAC_API UZodiacHealthBarWidget : public UZodiacHeroWidgetBase
{
	GENERATED_BODY()

protected:

	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn) override;
	
	//virtual void HandleHeroChanged(UZodiacHeroComponent* HeroComponent) override;

	UFUNCTION(BlueprintImplementableEvent)
	void OnHeroChanged(float NewValue, float MaxHealth);
	
	//UFUNCTION()
	//void HandleHealthChanged(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION(BlueprintImplementableEvent)
	void OnHealthChanged(float OldValue, float NewValue, float MaxHealth);

protected:
	
	float CurrentMaxHealth;

};

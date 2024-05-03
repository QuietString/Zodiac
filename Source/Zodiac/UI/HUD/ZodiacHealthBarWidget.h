// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Character/ZodiacHeroComponent.h"
#include "ZodiacHealthBarWidget.generated.h"

class AZodiacPlayerCharacter;
class UZodiacHealthComponent;
/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	
	virtual void NativeOnInitialized() override;

protected:
	
	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void HandleHeroChanged(UZodiacHeroComponent* HeroComponent);

	UFUNCTION(BlueprintNativeEvent)
	void OnHeroChanged(float NewValue, float MaxHealth);
	
	UFUNCTION()
	void HandleHealthChanged(UZodiacHealthComponent* HealthComponent, float OldValue, float NewValue, AActor* Instigator);

	UFUNCTION(BlueprintNativeEvent)
	void OnHealthChanged(float OldValue, float NewValue, float MaxHealth);

	float CurrentMaxHealth;
	
protected:

	UPROPERTY(BlueprintReadOnly)
	AZodiacPlayerCharacter* PlayerCharacter;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZodiacHeroWidgetBase.generated.h"

class UZodiacReticleWidgetBase;
class UZodiacHeroComponent;
class AZodiacHostCharacter;

/**
 * Widgets that can react to HeroComponent change
 */
UCLASS()
class ZODIAC_API UZodiacHeroWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeOnInitialized() override;

protected:

	// bind OnHeroChanged delegate to HandleHeroChanged().
	UFUNCTION()
	virtual void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	// Called when HeroComponent has changed
	//UFUNCTION()
	//virtual void HandleHeroChanged(UZodiacHeroComponent* HeroComponent);

protected:

	UPROPERTY(BlueprintReadOnly)
	AZodiacHostCharacter* HostCharacter;
};

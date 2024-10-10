// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterMovementComponent.h"
#include "Hero/ZodiacHeroAbilityDefinition.h"

#include "ZodiacHeroData.generated.h"

struct FZodiacHeroAbilityDefinition;
class UZodiacAbilitySet;
struct FAttributeDefaults;
class UZodiacReticleWidgetBase;

UCLASS(BlueprintType, Const)
class UZodiacHeroData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, Category = "Display")
	FName HeroName = TEXT("NoHeroName");
	
	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Movement")
	TEnumAsByte<EZodiacCustomMovementMode> DefaultMovementMode;

	// X: max speed, Y: mid speed, Z: min speed
	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector WalkSpeeds = FVector(200.0f, 175.0f, 150.0f);
	
	// X: max speed, Y: mid speed, Z: min speed
	UPROPERTY(EditAnywhere, Category = "Movement")
	FVector RunSpeeds = FVector(500.0f, 350.0f, 300.0f);
	
	UPROPERTY(EditAnywhere, Category = "Display")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;

	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<FZodiacHeroAbilityDefinition> AbilitySlots;
};
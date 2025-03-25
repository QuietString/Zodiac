// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterType.h"

#include "ZodiacHeroData.generated.h"

class UZodiacReticleWidgetBase;
class UZodiacHeroAbilitySlotDefinition;
class UZodiacAbilitySet;

UCLASS(BlueprintType, Const)
class UZodiacHeroData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<TObjectPtr<UZodiacHeroAbilitySlotDefinition>> AbilitySlots;

	UPROPERTY(editAnywhere, Category = "Widget")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> GlobalReticles;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	FZodiacExtendedMovementConfig ExtendedMovementConfig;
};
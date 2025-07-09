// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacterType.h"

#include "ZodiacHeroData.generated.h"

class AZodiacHeroActor;
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

	UPROPERTY(EditAnywhere, Category = "Widget")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> GlobalReticles;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	FZodiacExtendedMovementConfig ExtendedMovementConfig;
};
// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacCharacterType.h"
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
	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;

	UPROPERTY(EditAnywhere, Category = "Ability")
	TArray<FZodiacHeroAbilityDefinition> AbilitySlots;
	
	UPROPERTY(EditAnywhere, Category = "Movement")
	FZodiacExtendedMovementConfig ExtendedMovementConfig;
};
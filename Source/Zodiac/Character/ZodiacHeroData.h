// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZodiacCharacterMovementComponent.h"
#include "Item/ZodiacHeroItemSlot.h"

#include "ZodiacHeroData.generated.h"

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
	
	UPROPERTY(EditAnywhere, Category = "Display")
	TArray<TSubclassOf<UZodiacReticleWidgetBase>> ReticleWidgets;

	UPROPERTY(EditAnywhere, Category = "Item")
	FZodiacHeroItemDefinition WeaponDefinition;
	
	UPROPERTY(EditAnywhere, Category = "Item")
	TArray<FZodiacHeroItemDefinition> SkillSlots;
};
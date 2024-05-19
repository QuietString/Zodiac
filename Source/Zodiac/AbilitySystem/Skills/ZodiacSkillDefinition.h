// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ZodiacSkillDefinition.generated.h"

class UZodiacSkillAbility;
class UZodiacAbilitySystemComponent;

/**
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FZodiacSkillAbilityGrantData
{
	GENERATED_BODY()

public:

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UZodiacSkillAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
};

UCLASS(Const)
class ZODIAC_API UZodiacSkillDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	void GiveToAbilitySystemComponent(UZodiacAbilitySystemComponent* ZodiacASC);
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Tags, meta=(Categories = "Skill"))
	FGameplayTag SkillID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Tags, meta=(Categories = "Ability.Type.Skill.Slot"))
	FGameplayTag SlotType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FZodiacSkillAbilityGrantData AbilityToGrant;
	
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Sockets)
	// TArray<FName> TraceSockets;
	//
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Sockets)
	// TArray<FName> EffectSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
};

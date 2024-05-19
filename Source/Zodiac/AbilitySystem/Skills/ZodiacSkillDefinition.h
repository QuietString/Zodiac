// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ZodiacSkillDefinition.generated.h"

class UZodiacAbilitySystemComponent;

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
	FZodiacAbilitySet_GameplayAbility AbilityToGrant;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Sockets)
	TArray<FName> TraceSockets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Sockets)
	TArray<FName> EffectSockets;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
};

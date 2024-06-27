// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "System/GameplayTagStack.h"
#include "ZodiacSkillSet.generated.h"


class UZodiacSkillAbility;

/**
 *	Data used by the skill set to grant skill abilities.
 */
USTRUCT(BlueprintType)
struct FZodiacSkillSet_GrantedSkillAbility : public  FZodiacAbilitySet_GameplayAbility
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Tags, meta=(Categories = "Skill"))
	FGameplayTag SkillID;
};

UCLASS()
class ZODIAC_API UZodiacSkillSet : public UZodiacAbilitySet
{
	GENERATED_BODY()

public:
	UZodiacSkillSet(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	virtual void GiveToAbilitySystem(UZodiacAbilitySystemComponent* ZodiacASC, FZodiacAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject) const override;

protected:
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, meta=(TitleProperty=Skill))
	FZodiacSkillSet_GrantedSkillAbility PrimarySkillAbility;
	
	// Gameplay abilities to grant when this ability set is granted.
	UPROPERTY(EditDefaultsOnly, meta=(TitleProperty=Skill))
	TArray<FZodiacSkillSet_GrantedSkillAbility> GrantedSkillAbilities;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Components/PawnComponent.h"

#include "ZodiacSkillManagerComponent.generated.h"

struct FHeroChangedMessage_SkillSlot;
struct FZodiacSkillSetWithHandle;
class UAbilitySystemComponent;
struct FGameplayTag;
struct FGameplayAbilitySpecHandle;

UCLASS()
class ZODIAC_API UZodiacSkillManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	
	void RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData);

	void HandleSkillChanged(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpecHandle>& Handles);
	
protected:
	
	void GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage, UAbilitySystemComponent* InASC, FZodiacSkillSet* Skill);
	void GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, UAbilitySystemComponent* InASC, FZodiacSkillSet* Skill);
	
	FGameplayTag GetCooldownExtendedTag(const FGameplayTag& SkillTag);

protected:

	UPROPERTY()
	TMap<FGameplayAbilitySpecHandle, FZodiacSkillSet> SkillMap;
};

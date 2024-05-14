// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Components/PawnComponent.h"
#include "UI/HUD/ZodiacHealthBarWidget.h"

#include "ZodiacSkillManagerComponent.generated.h"

struct FZodiacSkillSetWithHandle;
class UAbilitySystemComponent;
struct FGameplayTag;
struct FGameplayAbilitySpecHandle;

USTRUCT(BlueprintType)
struct FHeroChangedMessage_SkillSlot
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> Instigator = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FText HeroName = FText();
	
	UPROPERTY(BlueprintReadWrite)
	FSlateBrush Brush;

	UPROPERTY(BlueprintReadOnly)
	bool bIsReady;

	UPROPERTY(BlueprintReadWrite)
	float CurrentValue;

	UPROPERTY(BlueprintReadWrite)
	float MaxValue;

	UPROPERTY(BlueprintReadWrite)
	float OptionalValue;
};

// Data struct for storing skill display data.
USTRUCT()
struct FSkillDataForDisplay
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FText SkillName = FText();
	
	UPROPERTY()
	FSlateBrush Brush;

	UPROPERTY()
	FGameplayTag SkillTag;
	
	UPROPERTY()
	FGameplayTag CooldownTag;
};

UCLASS()
class ZODIAC_API UZodiacSkillManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	
	void RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData);

	//void HandleHeroChanged(UZodiacHeroComponent* ZodiacHeroComponent);
	void HandleSkillChanged(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpecHandle>& Handles);
	
protected:
	
	void GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage, UAbilitySystemComponent* InASC, FZodiacSkillSet* Skill);
	void GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, UAbilitySystemComponent* InASC, FZodiacSkillSet* Skill);
	
	FGameplayTag GetCooldownExtendedTag(const FGameplayTag& SkillTag);

protected:

	UPROPERTY()
	TMap<FGameplayAbilitySpecHandle, FSkillDataForDisplay> DisplayDataMap;

	UPROPERTY()
	TMap<FGameplayAbilitySpecHandle, FZodiacSkillSet> SkillMap;
};

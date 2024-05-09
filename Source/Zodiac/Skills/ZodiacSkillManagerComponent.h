// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayTagContainer.h"
#include "Components/PawnComponent.h"

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
	bool HaveCooldown;
	
	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Duration = 0;
	
	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Remaining = 0;
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
	
	void HandleSkillChanged(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpecHandle>& Handles);
	
	void OnSkillChanged(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle& SpecHandle, const FGameplayTag& SkillType);

protected:

	bool GetCooldown(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle Handle, OUT float& CooldownRemaining, OUT float& CooldownDuration);

	FGameplayTag GetCooldownExtendedTag(const FGameplayTag& SkillTag);
	
protected:

	UPROPERTY()
	TMap<FGameplayAbilitySpecHandle, FSkillDataForDisplay> DisplayDataMap;
};

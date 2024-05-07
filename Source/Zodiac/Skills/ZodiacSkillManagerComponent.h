// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "Components/PawnComponent.h"

#include "ZodiacSkillManagerComponent.generated.h"

struct FZodiacSkillSetWithHandle;
class UZodiacHeroComponent;
class UAbilitySystemComponent;
struct FGameplayTag;
struct FGameplayAbilitySpecHandle;

// Data struct for storing skill display data.
USTRUCT()
struct FSkillDisplayData
{
	GENERATED_BODY()

public:

	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
	
	UPROPERTY()
	FSlateBrush Brush;

	bool operator==(const FSkillDisplayData& Other) const
	{
		return Handle == Other.Handle;
	}

	bool operator==(const FGameplayAbilitySpecHandle& Other) const
	{
		return Handle == Other;
	}
};

USTRUCT(BlueprintType)
struct FHeroChangedMessage_SkillSlot
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<APawn> PlayerPawn = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	FSlateBrush Brush;
	
	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Duration = 0;
	
	UPROPERTY(BlueprintReadWrite)
	float Cooldown_Remaining = 0;
};

UCLASS()
class ZODIAC_API UZodiacSkillManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	
	void InitializeWithHeroComponent(UZodiacHeroComponent* HeroComponent);

	void RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData);
	
	void HandleSkillChanged(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpecHandle>& Handles);
	
	void OnSkillChanged(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle& SpecHandle);

	
protected:

	bool GetCooldown(UAbilitySystemComponent* InASC, const FGameplayAbilitySpecHandle Handle, OUT float& CooldownRemaining, OUT float& CooldownDuration);
public:

protected:

	UPROPERTY()
	TArray<FSkillDisplayData> SkillDisplayDataList;
};

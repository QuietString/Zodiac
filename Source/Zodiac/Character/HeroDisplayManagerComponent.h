// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Components/PawnComponent.h"
#include "HeroDisplayManagerComponent.generated.h"

struct FHeroChangedMessage_SkillSlot;
struct FZodiacSkillSetWithHandle;
class UZodiacUltimateSet;
class UZodiacCombatSet;
class UZodiacHealthSet;
class UAbilitySystemComponent;


UCLASS(Transient)
class UHeroDisplayData : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	const UZodiacHealthSet* HealthSet;

	UPROPERTY()
	const UZodiacCombatSet* CombatSet;

	UPROPERTY()
	const UZodiacUltimateSet* UltimateSet;
};

UCLASS()
class ZODIAC_API UHeroDisplayManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UHeroDisplayManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void InitializeHeroData(const int32 InSlotIndex, UAbilitySystemComponent* InASC);
	
	void RegisterSkillDisplayData(const FZodiacSkillSetWithHandle& SkillData);

	void OnHeroChanged();
	
	void OnSkillChanged(UAbilitySystemComponent* InASC, const TArray<FGameplayAbilitySpecHandle>& Handles);

	void HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

protected:

	void SendHealthBarHeroChangedMessage();
	void SendSkillChangedMessages();
	
	void GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage, FZodiacSkillSet* Skill);
	void GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FZodiacSkillSet* Skill);

protected:

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(Transient)
	UAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY()
	TMap<FGameplayAbilitySpecHandle, FZodiacSkillSet> SkillMap;

	UPROPERTY()
	const UZodiacHealthSet* HealthSet;

	// UPROPERTY()
	// const UZodiacCombatSet* CombatSet;

	UPROPERTY()
	const UZodiacUltimateSet* UltimateSet;
};

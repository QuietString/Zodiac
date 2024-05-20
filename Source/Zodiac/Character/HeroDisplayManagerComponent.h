// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Skills/SkillHandle.h"
#include "Components/PawnComponent.h"
#include "HeroDisplayManagerComponent.generated.h"

struct FSkillHandleDataContainer;
class UZodiacSkillDefinition;
struct FHeroChangedMessage_SkillSlot;
class UZodiacUltimateSet;
class UZodiacCombatSet;
class UZodiacHealthSet;


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

	void InitializeHeroData(const int32 InSlotIndex, UZodiacAbilitySystemComponent* InZodiacASC, const TArray<UZodiacSkillDefinition*>& InSkillDefinitions, FSimpleMulticastDelegate
	                        & OnHeroChanged);
	
	void OnHeroChanged();
	
	void HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
	virtual void BeginPlay() override;
	
protected:
	void SendResetMessages();
	
	void SendHealthBarHeroChangedMessage();
	void SendSkillChangedMessages();
	
	void GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage);
	void GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SkillID);

protected:

	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(Transient)
	UZodiacAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	const UZodiacHealthSet* HealthSet;
	// UPROPERTY()
	// const UZodiacCombatSet* CombatSet;
	UPROPERTY()
	const UZodiacUltimateSet* UltimateSet;

	UPROPERTY()
	TArray<UZodiacSkillDefinition*> SkillDefinitions;

	TMap<FGameplayTag, float> RequiredCostAmounts;
	TMap<FGameplayTag, FGameplayTag> SkillCostTypeMap;

	float RequiredUltimateCostAmount = 0.0f;
};

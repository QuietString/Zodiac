// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"
#include "Components/PawnComponent.h"
#include "ZodiacSkillManagerComponent.generated.h"

class UZodiacSkillManagerComponent;
class UZodiacSkillSlotDefinition;
class UZodiacSkillSlot;
class UZodiacHeroComponent;
struct FSkillHandleDataContainer;
class UZodiacSkillDefinition;
struct FHeroChangedMessage_SkillSlot;
class UZodiacUltimateSet;
class UZodiacCombatSet;
class UZodiacHealthSet;


UCLASS()
class ZODIAC_API UZodiacSkillManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacSkillManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void ReadyForReplication() override;
	virtual  void BeginPlay() override;
	//~End of UActorComponent interface
	
	void InitializeSlots(UZodiacHeroComponent* HeroComponent, TMap<FGameplayTag, TObjectPtr<UZodiacSkillSlotDefinition>>SlotDefinitions);
	void OnHeroChanged();
	
	void HandleHealthChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	void HandleUltimateGaugeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

protected:
	void SendResetMessages();
	
	void SendHealthBarHeroChangedMessage();
	void SendSlotStatTagChangedMessage(UZodiacSkillSlot* Slot);
	void SendSkillSlotChangedMessages();
	
	void GetUltimateGauge(FHeroChangedMessage_SkillSlot& OutMessage);
	void GetCooldown(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SlotType);
	void GetCooldown2(FHeroChangedMessage_SkillSlot& OutMessage, FGameplayTag SlotType);

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

	TMap<FGameplayTag, float> RequiredCostAmounts;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<UZodiacSkillSlot>> Slots;
	
	float RequiredUltimateCostAmount = 0.0f;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/Skills/ZodiacSkillSlot.h"
#include "Components/PawnComponent.h"
#include "ZodiacSkillManagerComponent.generated.h"

struct FZodiacSkillSlotList;
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

USTRUCT(BlueprintType)
struct FZodiacSkillSlotEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FZodiacSkillSlotEntry()
	{}

	FString GetDebugString() const;

private:
	friend UZodiacSkillManagerComponent;
	friend FZodiacSkillSlotList;

	UPROPERTY()
	TObjectPtr<UZodiacSkillSlotDefinition> SlotDefinition;
	
	UPROPERTY()
	TObjectPtr<UZodiacSkillSlot> Slot = nullptr;

	UPROPERTY()
	FZodiacAbilitySet_GrantedHandles GrantedHandles;
};

/** List of skill slot */
USTRUCT(BlueprintType)
struct FZodiacSkillSlotList : public FFastArraySerializer
{
	GENERATED_BODY()

	FZodiacSkillSlotList()
		: OwnerComponent(nullptr)
	{
	}

	FZodiacSkillSlotList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

public:
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FZodiacSkillSlotEntry, FZodiacSkillSlotList>(Entries, DeltaParms, *this);
	}

	UZodiacSkillSlot* AddEntry(TObjectPtr<UZodiacSkillSlotDefinition> SlotDefinition, FGameplayTag
	                           SlotType, TObjectPtr<UZodiacAbilitySystemComponent> ZodiacASC);

private:
	friend UZodiacSkillManagerComponent;

private:
	// Replicated list of skill slot entries
	UPROPERTY()
	TArray<FZodiacSkillSlotEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

UCLASS()
class ZODIAC_API UZodiacSkillManagerComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacSkillManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UActorComponent interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
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

	UFUNCTION()
	void OnRep_SkillSlotList(const FZodiacSkillSlotList& OldList);

protected:
	UPROPERTY()
	int32 SlotIndex = INDEX_NONE;

	UPROPERTY(Transient)
	UZodiacAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(ReplicatedUsing = OnRep_SkillSlotList)
	FZodiacSkillSlotList SkillSlotList;
	
	UPROPERTY()
	const UZodiacHealthSet* HealthSet;
	// UPROPERTY()
	// const UZodiacCombatSet* CombatSet;
	
	UPROPERTY()
	const UZodiacUltimateSet* UltimateSet;

	TMap<FGameplayTag, float> RequiredCostAmounts;

	float RequiredUltimateCostAmount = 0.0f;
};

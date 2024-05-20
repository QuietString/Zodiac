#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameplayAbilitySpecHandle.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "SkillHandle.generated.h"

class UZodiacSkillDefinition;
class UZodiacAbilitySet;
struct FSkillHandleDataContainer;

USTRUCT(BlueprintType)
struct FSkillHandleData : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FSkillHandleData()
	{}

private:
	friend FSkillHandleDataContainer;
	friend UZodiacAbilitySet;
	friend UZodiacSkillDefinition;
	
	UPROPERTY()
	FGameplayTag SkillID;
	
	UPROPERTY()
	FGameplayTag SlotType;

	UPROPERTY()
	FGameplayTag CostType;
	
	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;
};

USTRUCT()
struct FSkillHandleDataContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FSkillHandleDataContainer()
	{}

public:
	FGameplayTag FindSkillIdByHandle(FGameplayAbilitySpecHandle Handle) const;
	
	// Return true if matching tag is found.
	bool FindSlotType(FGameplayTag SkillID, FGameplayTag& OutSlotType) const;
	bool FindCostType(FGameplayTag SkillID, FGameplayTag& OutCostType) const;

	// Return true if matching ability handle is found.
	bool GetAbilitySpecHandle(FGameplayTag SkillID, FGameplayAbilitySpecHandle& OutHandle) const;

	void AddSkillHandle(FSkillHandleData Data);
	
	//~FFastArraySerializer contract
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSkillHandleData, FSkillHandleDataContainer>( SkillHandles, DeltaParms, *this );
	}

private:
	UPROPERTY()
	TArray<FSkillHandleData> SkillHandles;
	
	TArray<FGameplayAbilitySpecHandle> SpecHandles;
	
	TMap<FGameplayTag, FSkillHandleData> SkillTagToHandle;
};

template<>
struct TStructOpsTypeTraits< FSkillHandleDataContainer > : public TStructOpsTypeTraitsBase2< FSkillHandleDataContainer >
{
	enum 
	{
		WithNetDeltaSerializer = true,
   };
};
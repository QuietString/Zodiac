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
	FGameplayAbilitySpecHandle Handle;
};

USTRUCT()
struct FSkillHandleDataContainer : public FFastArraySerializer
{
	GENERATED_BODY()

	FSkillHandleDataContainer()
	{}

public:
	void AddSkillHandle(FSkillHandleData Data);
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo & DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FSkillHandleData, FSkillHandleDataContainer>( SkillHandles, DeltaParms, *this );
	}

	//~FFastArraySerializer contract
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	// Return true if matching SlotType tag is found.
	bool GetSlotType(FGameplayTag SkillID, FGameplayTag& OutSlotType) const;

	// Return true if matching ability handle is found.
	bool GetAbilitySpecHandle(FGameplayTag SkillID, FGameplayAbilitySpecHandle& OutHandle) const;

private:
	UPROPERTY()
	TArray<FSkillHandleData> SkillHandles;
	
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
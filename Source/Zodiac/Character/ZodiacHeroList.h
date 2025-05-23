﻿// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ZodiacHeroList.generated.h"

struct FZodiacHeroList;
class AZodiacHeroCharacter;

/** A single entry of HeroList */
USTRUCT(BlueprintType)
struct FZodiacHeroEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FZodiacHeroEntry()
	{}

private:
	friend FZodiacHeroList;

	UPROPERTY()
	TObjectPtr<AZodiacHeroCharacter> Actor = nullptr;
};

/** List of Heroes */
USTRUCT(BlueprintType)
struct FZodiacHeroList : public FFastArraySerializer
{
	GENERATED_BODY()

	FZodiacHeroList()
		: Owner(nullptr)
	{
	}

	FZodiacHeroList(AActor* InOwner)
		: Owner(InOwner)
	{
	}
	
public:
	AZodiacHeroCharacter* AddEntry(UWorld* World, TSubclassOf<AZodiacHeroCharacter> HeroClass, int Index);
	void RemoveEntry(AZodiacHeroCharacter* Instance);

	AZodiacHeroCharacter* GetHero(const int32 Index) const;
	TArray<AZodiacHeroCharacter*> GetHeroes() const;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FZodiacHeroEntry, FZodiacHeroList>(Heroes, DeltaParms, *this);
	}
	
private:
	// Replicated list of heroes
	UPROPERTY()
	TArray<FZodiacHeroEntry> Heroes;
	
	UPROPERTY()
	TObjectPtr<AActor> Owner;
};

template<>
struct TStructOpsTypeTraits<FZodiacHeroList> : public TStructOpsTypeTraitsBase2<FZodiacHeroList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
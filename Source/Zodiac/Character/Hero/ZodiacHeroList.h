// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ZodiacHeroList.generated.h"

class AZodiacHostCharacter;
class AZodiacHeroActor;
struct FZodiacHeroList2;
class AZodiacHeroCharacter;

/** A single entry of HeroList */
USTRUCT(BlueprintType)
struct FZodiacHeroEntry2 : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FZodiacHeroEntry2()
	{}

private:
	friend FZodiacHeroList2;

	UPROPERTY()
	TObjectPtr<AZodiacHeroActor> Actor = nullptr;
};

/** List of Heroes */
USTRUCT(BlueprintType)
struct FZodiacHeroList2 : public FFastArraySerializer
{
	GENERATED_BODY()

	FZodiacHeroList2()
		: Owner(nullptr)
	{
	}

	FZodiacHeroList2(AActor* InOwner)
		: Owner(InOwner)
	{
	}
	
public:
	void SetOwner(AActor* InOwner) { Owner = InOwner; }

	AZodiacHeroActor* AddEntry(UWorld* World, AZodiacHostCharacter* InHostCharacter, TSubclassOf<AZodiacHeroActor> HeroClass, int Index);
	void RemoveEntry(AZodiacHeroActor* Instance);

	AZodiacHeroActor* GetHero(const int32 Index) const;
	TArray<AZodiacHeroActor*> GetHeroes() const;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FZodiacHeroEntry2, FZodiacHeroList2>(Heroes, DeltaParms, *this);
	}
	
private:
	// Replicated list of heroes
	UPROPERTY()
	TArray<FZodiacHeroEntry2> Heroes;
	
	UPROPERTY()
	TObjectPtr<AActor> Owner;
};

template<>
struct TStructOpsTypeTraits<FZodiacHeroList2> : public TStructOpsTypeTraitsBase2<FZodiacHeroList2>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
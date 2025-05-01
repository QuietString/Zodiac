// the.quiet.string@gmail.com

#include "ZodiacHeroList.h"

#include "ZodiacHeroCharacter.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroList)

AZodiacHeroCharacter* FZodiacHeroList::AddEntry(UWorld* World, TSubclassOf<AZodiacHeroCharacter> HeroClass, int Index)
{
	AZodiacHeroCharacter* Result = nullptr;
	check(HeroClass);
	check(World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Owner;

	AZodiacHeroCharacter* Hero = World->SpawnActor<AZodiacHeroCharacter>(HeroClass, Params);
	
	FZodiacHeroEntry& NewEntry = Heroes.AddDefaulted_GetRef();
	NewEntry.Actor = Hero;
	
	Result = NewEntry.Actor;
	MarkItemDirty(NewEntry);

	return Result;
}

void FZodiacHeroList::RemoveEntry(AZodiacHeroCharacter* Instance)
{
}

AZodiacHeroCharacter* FZodiacHeroList::GetHero(const int32 Index) const
{
	if (Heroes.IsValidIndex(Index))
	{
		return Heroes[Index].Actor;	
	}

	return nullptr;
}

TArray<AZodiacHeroCharacter*> FZodiacHeroList::GetHeroes() const
{
	TArray<AZodiacHeroCharacter*> Results;
	Results.Reserve(Heroes.Num());
	
	for (const FZodiacHeroEntry& Entry : Heroes)
	{
		if (Entry.Actor)
		{
			Results.Add(Entry.Actor);	
		}
	}
	
	return Results;
}

void FZodiacHeroList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FZodiacHeroList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FZodiacHeroList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}
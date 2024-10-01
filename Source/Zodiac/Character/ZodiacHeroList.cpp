// the.quiet.string@gmail.com

#include "ZodiacHeroList.h"
#include "ZodiacHeroActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroList)

AZodiacHeroActor* FZodiacHeroList::AddEntry(UWorld* World, TSubclassOf<AZodiacHeroActor> HeroClass, int Index)
{
	AZodiacHeroActor* Result = nullptr;
	check(HeroClass);
	check(World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Owner;

	AZodiacHeroActor* Hero = World->SpawnActor<AZodiacHeroActor>(HeroClass, Params);
	Hero->Index = Index;
	
	FZodiacHeroEntry& NewEntry = Heroes.AddDefaulted_GetRef();
	NewEntry.Actor = Hero;
	
	Result = NewEntry.Actor;
	MarkItemDirty(NewEntry);

	return Result;
}

void FZodiacHeroList::RemoveEntry(AZodiacHeroActor* Instance)
{
}

AZodiacHeroActor* FZodiacHeroList::GetHero(const int32 Index) const
{
	if (Heroes.IsValidIndex(Index))
	{
		return Heroes[Index].Actor;	
	}

	return nullptr;
}

TArray<AZodiacHeroActor*> FZodiacHeroList::GetHeroes()
{
	TArray<AZodiacHeroActor*> Results;
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
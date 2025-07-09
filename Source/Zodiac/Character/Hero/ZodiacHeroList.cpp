// the.quiet.string@gmail.com

#include "ZodiacHeroList.h"

#include "Character/Hero/ZodiacHeroActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroList)

AZodiacHeroActor* FZodiacHeroList2::AddEntry(UWorld* World, AZodiacHostCharacter* InHostCharacter, TSubclassOf<AZodiacHeroActor> HeroClass, int Index)
{
	AZodiacHeroActor* Result = nullptr;
	check(HeroClass);
	check(World);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = Owner;
	
	AZodiacHeroActor* Hero = World->SpawnActor<AZodiacHeroActor>(HeroClass, Params);
	Hero->SetHostCharacter(InHostCharacter);
	
	FZodiacHeroEntry2& NewEntry = Heroes.AddDefaulted_GetRef();
	NewEntry.Actor = Hero;
	
	Result = NewEntry.Actor;
	MarkItemDirty(NewEntry);

	return Result;
}

void FZodiacHeroList2::RemoveEntry(AZodiacHeroActor* Instance)
{
}

AZodiacHeroActor* FZodiacHeroList2::GetHero(const int32 Index) const
{
	if (Heroes.IsValidIndex(Index))
	{
		return Heroes[Index].Actor;	
	}

	return nullptr;
}

TArray<AZodiacHeroActor*> FZodiacHeroList2::GetHeroes() const
{
	TArray<AZodiacHeroActor*> Results;
	Results.Reserve(Heroes.Num());
	
	for (const FZodiacHeroEntry2& Entry : Heroes)
	{
		if (Entry.Actor)
		{
			Results.Add(Entry.Actor);	
		}
	}
	
	return Results;
}

void FZodiacHeroList2::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
}

void FZodiacHeroList2::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
}

void FZodiacHeroList2::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
}

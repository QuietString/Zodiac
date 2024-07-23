// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameplayTagStack.h"

#include "UObject/Stack.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayTagStack)

//////////////////////////////////////////////////////////////////////
// FGameplayTagStack

FString FGameplayTagStack::GetDebugString() const
{
	return FString::Printf(TEXT("%sx%d"), *Tag.ToString(), StackCount);
}

//////////////////////////////////////////////////////////////////////
// FGameplayTagStackContainer

void FGameplayTagStackContainer::AddStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to AddStack"), ELogVerbosity::Warning);
		return;
	}

	if (StackCount > 0)
	{
		for (FGameplayTagStack& Stack : Stacks)
		{
			if (Stack.Tag == Tag)
			{
				const int32 OldCount = Stack.StackCount;
				const int32 NewCount = OldCount + StackCount;
				Stack.StackCount = NewCount;
				TagToCountMap[Tag] = NewCount;

				MarkItemDirty(Stack);
				OnStackChanged.ExecuteIfBound(Tag, OldCount, NewCount);
				return;
			}
		}

		FGameplayTagStack& NewStack = Stacks.Emplace_GetRef(Tag, StackCount);
		MarkItemDirty(NewStack);
		TagToCountMap.Add(Tag, StackCount);
	}
}

void FGameplayTagStackContainer::RemoveStack(FGameplayTag Tag, int32 StackCount)
{
	if (!Tag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("An invalid tag was passed to RemoveStack"), ELogVerbosity::Warning);
		return;
	}
	
	if (StackCount > 0)
	{
		for (auto It = Stacks.CreateIterator(); It; ++It)
		{
			FGameplayTagStack& Stack = *It;
			if (Stack.Tag == Tag)
			{
				if (Stack.StackCount <= StackCount)
				{
					const int32 OldCount = Stack.StackCount;
					const int32 NewCount = 0;
					
					It.RemoveCurrent();
					TagToCountMap.Remove(Tag);
					MarkArrayDirty();
					
					OnStackChanged.ExecuteIfBound(Tag, OldCount, NewCount);
				}
				else
				{
					const int32 OldCount = Stack.StackCount;
					const int32 NewCount = OldCount - StackCount;
					Stack.StackCount = NewCount;
					TagToCountMap[Tag] = NewCount;
					MarkItemDirty(Stack);
					
					OnStackChanged.ExecuteIfBound(Tag, OldCount, NewCount);
				}
				return;
			}
		}
	}
}

void FGameplayTagStackContainer::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		const int32 OldCount = Stacks[Index].StackCount;
		const int32 NewCount = 0;

		const FGameplayTag Tag = Stacks[Index].Tag;
		TagToCountMap.Remove(Tag);
		OnStackChanged.ExecuteIfBound(Tag, OldCount, NewCount);

	}
}

void FGameplayTagStackContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		const int32 OldCount = 0;
		const int32 NewCount = TagToCountMap.Add(Stack.Tag, Stack.StackCount);

		OnStackChanged.ExecuteIfBound(Stack.Tag, OldCount, NewCount);
		UE_LOG(LogTemp, Warning, TEXT("post replicated add tag: %s, count: %d"), *Stack.Tag.ToString(), Stack.StackCount);
	}
}

void FGameplayTagStackContainer::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FGameplayTagStack& Stack = Stacks[Index];
		const int32 OldCount = TagToCountMap[Stack.Tag];
		const int32 NewCount = TagToCountMap[Stack.Tag] = Stack.StackCount;

		OnStackChanged.ExecuteIfBound(Stack.Tag, OldCount, NewCount);
		UE_LOG(LogTemp, Warning, TEXT("post replicated changed tag: %s, count: %d, index: %d"), *Stack.Tag.ToString(), Stack.StackCount, Index);
	}
}
// the.quiet.string@gmail.com

#include "SkillHandle.h"

void FSkillHandleDataContainer::AddSkillHandle(FSkillHandleData Data)
{
	if (!Data.SkillID.IsValid())
	{
		return;
	}

	if (!SkillTagToHandle.Find(Data.SkillID))
	{
		FSkillHandleData& NewData = SkillHandles.Emplace_GetRef(Data);
		MarkItemDirty(NewData);
		SkillTagToHandle.Add(NewData.SkillID, NewData);
	}
}

void FSkillHandleDataContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FSkillHandleData& Data = SkillHandles[Index];
		SkillTagToHandle.Add(Data.SkillID, Data);
	}
}

bool FSkillHandleDataContainer::GetSlotType(FGameplayTag SkillID, FGameplayTag& OutSlotType) const
{
	if (SkillTagToHandle.Find(SkillID))
	{
		OutSlotType = SkillTagToHandle[SkillID].SlotType;
		return true;
	}

	return false;
}

bool FSkillHandleDataContainer::GetAbilitySpecHandle(FGameplayTag SkillID, FGameplayAbilitySpecHandle& OutHandle) const
{
	if (SkillTagToHandle.Find(SkillID))
	{
		OutHandle = SkillTagToHandle[SkillID].Handle;
		return true;
	}

	return false;
}

// the.quiet.string@gmail.com

#include "SkillHandle.h"


FGameplayTag FSkillHandleDataContainer::FindSkillIdByHandle(FGameplayAbilitySpecHandle Handle) const
{
	for (auto& [Key, Value] : SkillTagToHandle)
	{
		if (Value.Handle == Handle)
		{
			return Key;
		}
	}

	return FGameplayTag();
}


bool FSkillHandleDataContainer::FindSlotType(FGameplayTag SkillID, FGameplayTag& OutSlotType) const
{
	if (SkillTagToHandle.Find(SkillID))
	{
		OutSlotType = SkillTagToHandle[SkillID].SlotType;
		return true;
	}

	return false;
}

bool FSkillHandleDataContainer::FindCostType(FGameplayTag SkillID, FGameplayTag& OutCostType) const
{
	if (SkillTagToHandle.Find(SkillID))
	{
		OutCostType = SkillTagToHandle[SkillID].CostType;
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
		SpecHandles.Add(NewData.Handle);
	}
}

void FSkillHandleDataContainer::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FSkillHandleData& Data = SkillHandles[Index];
		SkillTagToHandle.Add(Data.SkillID, Data);
		SpecHandles.Add(Data.Handle);
	}
}
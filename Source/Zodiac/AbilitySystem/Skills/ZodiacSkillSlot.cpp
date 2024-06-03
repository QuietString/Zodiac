// the.quiet.string@gmail.com


#include "ZodiacSkillSlot.h"

#include "ZodiacSkillSet.h"
#include "ZodiacSkillSlotDefinition.h"
#include "Net/UnrealNetwork.h"


UZodiacSkillSlot::UZodiacSkillSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UZodiacSkillSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTag);
}

const UZodiacSkillSlotFragment* UZodiacSkillSlot::FindFragmentByClass(TSubclassOf<UZodiacSkillSlotFragment> FragmentClass) const
{
	if (FragmentClass && SlotDefinition)
	{
		for (UZodiacSkillSlotFragment* Fragment : SlotDefinition->Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

int32 UZodiacSkillSlot::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTag.GetStackCount(Tag);
}

void UZodiacSkillSlot::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.AddStack(Tag, StackCount);
}

void UZodiacSkillSlot::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.RemoveStack(Tag, StackCount);
}
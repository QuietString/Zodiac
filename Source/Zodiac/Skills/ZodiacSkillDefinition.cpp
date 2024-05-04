// the.quiet.string@gmail.com


#include "ZodiacSkillDefinition.h"

UZodiacSkillDefinition::UZodiacSkillDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

const UZodiacSkillFragment* UZodiacSkillDefinition::FindFragmentByClass(
	TSubclassOf<UZodiacSkillFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UZodiacSkillFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

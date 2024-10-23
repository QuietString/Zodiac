// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySlot.h"

#include "ZodiacHeroAbilityFragment_Reticle.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilitySlot)

UZodiacHeroAbilityFragment::UZodiacHeroAbilityFragment(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UZodiacHeroAbilitySlot::UZodiacHeroAbilitySlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacHeroAbilitySlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTag);
	DOREPLIFETIME_CONDITION(ThisClass, Definition, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, GrantedHandles, COND_InitialOnly);
}

void UZodiacHeroAbilitySlot::InitializeSlot(const FZodiacHeroAbilityDefinition& InDef)
{
	Definition = InDef;

	for (auto& Fragment : Definition.Fragments)
	{
		Fragment->OnSlotCreated(this);
	}
}

FGameplayTag UZodiacHeroAbilitySlot::GetSlotType() const
{
	return Definition.SlotType;
}

void UZodiacHeroAbilitySlot::UpdateActivationTime()
{
	UWorld* World = GetWorld();
	check(World);
	TimeLastFired = World->GetTimeSeconds();
}

void UZodiacHeroAbilitySlot::ChangeReticle()
{
	if (UZodiacHeroAbilityFragment_Reticle* Fragment_Reticle = FindFragmentByClass<UZodiacHeroAbilityFragment_Reticle>())
	{
		OnReticleApplied.ExecuteIfBound(Fragment_Reticle->ReticleWidgets, this);	
	}
}

void UZodiacHeroAbilitySlot::ClearReticle()
{
	if (UZodiacHeroAbilityFragment_Reticle* Fragment_Reticle = FindFragmentByClass<UZodiacHeroAbilityFragment_Reticle>())
	{
		OnReticleCleared.ExecuteIfBound();	
	}
}

APawn* UZodiacHeroAbilitySlot::GetPawn() const
{
	return Cast<APawn>(GetOuter());
}

const UZodiacHeroAbilityFragment* UZodiacHeroAbilitySlot::FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UZodiacHeroAbilityFragment* Fragment : Definition.Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

int32 UZodiacHeroAbilitySlot::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTag.GetStackCount(Tag);
}

void UZodiacHeroAbilitySlot::SetStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.SetStack(Tag, StackCount);
}

void UZodiacHeroAbilitySlot::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.AddStack(Tag, StackCount);
}

void UZodiacHeroAbilitySlot::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTag.RemoveStack(Tag, StackCount);
}
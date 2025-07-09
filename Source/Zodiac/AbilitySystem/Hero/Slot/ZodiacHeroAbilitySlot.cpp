// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySlot.h"

#include "ZodiacGameplayTags.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilityFragment_Reticle.h"
#include "ZodiacHeroAbilitySlotActor.h"
#include "ZodiacHeroAbilitySlotDefinition.h"
#include "Character/Hero/ZodiacHeroActor.h"
#include "Character/Host/ZodiacHostCharacter.h"
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
	DOREPLIFETIME_CONDITION(ThisClass, SpawnedActors, COND_InitialOnly);
}

void UZodiacHeroAbilitySlot::InitializeSlot(const UZodiacHeroAbilitySlotDefinition* InDef)
{
	AActor* OwnerActor = GetTypedOuter<AActor>();
	check(OwnerActor);
	
	Definition = InDef;
	
	TArray<AZodiacHeroAbilitySlotActor*> Actors;
	for (auto& Actor : InDef->ActorsToSpawn)
	{
		AZodiacHeroAbilitySlotActor* NewActor = GetWorld()->SpawnActorDeferred<AZodiacHeroAbilitySlotActor>(Actor, FTransform::Identity, OwnerActor, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		NewActor->FinishSpawning(FTransform::Identity, true);
		NewActor->AttachToComponent(OwnerActor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
		Actors.Add(NewActor);
	}
	SpawnedActors = Actors;
	
	for (auto& Fragment : Definition->Fragments)
	{
		Fragment->OnSlotCreated(this);
	}
}

FGameplayTag UZodiacHeroAbilitySlot::GetSlotType() const
{
	return  Definition->SlotType;
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

AZodiacHostCharacter* UZodiacHeroAbilitySlot::GetHostCharacter() const
{
	if (AZodiacHeroActor* HeroActor = Cast<AZodiacHeroActor>(GetOuter()))
	{
		return HeroActor->GetHostCharacter();
	}
		
	return nullptr;
}

const UZodiacHeroAbilityFragment* UZodiacHeroAbilitySlot::FindFragmentByClass(const TSubclassOf<UZodiacHeroAbilityFragment>& FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UZodiacHeroAbilityFragment* Fragment : Definition->Fragments)
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
#if !UE_BUILD_SHIPPING
	if (AZodiacHostCharacter* Host = GetHostCharacter())
	{
		if (Host->HasMatchingGameplayTag(ZodiacGameplayTags::Cheat_InfiniteAmmo) && Tag == ZodiacGameplayTags::Ability_Cost_Stack_MagazineAmmo)
		{
			return INT_MAX;
		}
	}
#endif
	
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
#if !UE_BUILD_SHIPPING
	if (AZodiacHostCharacter* Host = GetHostCharacter())
	{
		if (Host->HasMatchingGameplayTag(ZodiacGameplayTags::Cheat_InfiniteAmmo) && Tag == ZodiacGameplayTags::Ability_Cost_Stack_MagazineAmmo)
		{
			return;
		}
	}
#endif
	
	StatTag.RemoveStack(Tag, StackCount);
}

const UZodiacHeroAbilityFragment* UZodiacAbilitySlotFunctionLibrary::FindAbilityFragment(UZodiacHeroAbilitySlot* Slot, TSubclassOf<UZodiacHeroAbilityFragment> FragmentClass)
{
	if (Slot && FragmentClass)
	{
		if (const UZodiacHeroAbilitySlotDefinition* Definition = Slot->GetSlotDefinition())
		{
			return Definition->FindFragmentByClass(FragmentClass);	
		}
	}
	
	return nullptr;
}

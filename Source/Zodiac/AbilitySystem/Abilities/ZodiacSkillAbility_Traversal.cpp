// the.quiet.string@gmail.com


#include "ZodiacSkillAbility_Traversal.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Character/ZodiacCharacter.h"
#include "Traversal/ZodiacTraversalComponent.h"
#include "Traversal/ZodiacTraversalTypes.h"
#include "GameplayAbilities/Public/Abilities/Tasks/AbilityTask_WaitDelay.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbility_Traversal)

UZodiacSkillAbility_Traversal::UZodiacSkillAbility_Traversal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		// Add the ability trigger tag as default to the CDO.
		FAbilityTriggerData TriggerData;
		TriggerData.TriggerTag = ZodiacGameplayTags::Event_Traversal;
		TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
		AbilityTriggers.Add(TriggerData);
	}
}

bool UZodiacSkillAbility_Traversal::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                            const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			FText FailReason;
			bool Result = TraversalComponent->CanTraversalAction(FailReason);
#if WITH_EDITOR
			if (!Result && ZodiacConsoleVariables::CVarTraversalDrawDebug.GetValueOnAnyThread())
			{
				UE_LOG(LogZodiacTraversal, Log, TEXT("Traversal Failed Reason: %s"), *FailReason.ToString());
			}
#endif
			return Result;
		}
	}

	return false;
}

void UZodiacSkillAbility_Traversal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                         const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo())
	{
		if (UZodiacTraversalComponent* TraversalComponent = ZodiacCharacter->FindComponentByClass<UZodiacTraversalComponent>())
		{
			TraversalComponent->OnTraversalFinished.BindUObject(this, &ThisClass::OnTraversalFinished);
			if (HasAuthority(&CurrentActivationInfo))
			{
				TraversalComponent->PerformTraversalActionFromAbility();	
			}
			return;
		}
	}
	
	// Automatically ends ability after some time in case.
	UAbilityTask_WaitDelay* WaitDelay =  UAbilityTask_WaitDelay::WaitDelay(this, 5.0f);
	WaitDelay->OnFinish.AddDynamic(this, &ThisClass::OnTraversalFinished);
	WaitDelay->Activate();
}

void UZodiacSkillAbility_Traversal::OnTraversalFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

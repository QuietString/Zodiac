// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySystemComponent.h"

#include "AbilitySystem/Hero/Abilities/ZodiacHeroAbility.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "Character/Hero/ZodiacHeroActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilitySystemComponent)

void UZodiacHeroAbilitySystemComponent::SetHostAbilitySystemComponent(UZodiacHostAbilitySystemComponent* InASC)
{
	check(InASC);
	HostAbilitySystemComponent = InASC;
}

void UZodiacHeroAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UZodiacHeroAbility* HeroAbility = CastChecked<UZodiacHeroAbility>(Ability))
	{
		AddAbilityToActivationGroup(HeroAbility->GetActivationGroup(), HeroAbility);
	}
}

void UZodiacHeroAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UZodiacHeroAbility* HeroAbility = Cast<UZodiacHeroAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(HeroAbility->GetActivationGroup(), HeroAbility);	
	}
}

void UZodiacHeroAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	if (AZodiacHeroActor* HeroAvatar = Cast<AZodiacHeroActor>(GetAvatarActor()))
	{
		if (!HeroAvatar->IsHostLocallyControlled() && Ability->IsSupportedForNetworking())
		{
			ClientNotifyAbilityFailed(Ability, FailureReason);
			return;
		}
	}

	HandleAbilityFailed(Ability, FailureReason);
}

void UZodiacHeroAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
                                                                       const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
	
	if (HostAbilitySystemComponent)
	{
		HostAbilitySystemComponent->ApplyAbilityBlockAndCancelTags_FromHeroASC(AbilityTags, nullptr, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
	}
}

void UZodiacHeroAbilitySystemComponent::ApplyAbilityBlockAndCancelTags_FromHostASC(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags,
	const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, BlockTags, bExecuteCancelTags, CancelTags);
}

void UZodiacHeroAbilitySystemComponent::ClearCameraModeAfterDuration(const FGameplayAbilitySpecHandle& OwningSpecHandle, float Duration, FTimerHandle& RemoveHandle)
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetOwnerActor()))
	{
		GetWorld()->GetTimerManager().SetTimer(
    		RemoveHandle,
    		[HostCharacter = HostCharacter, OwningSpecHandle = OwningSpecHandle]()
    		{
    			HostCharacter->ClearAbilityCameraMode(OwningSpecHandle);
    		},
    		Duration,
    		false
    	);	
	}
}

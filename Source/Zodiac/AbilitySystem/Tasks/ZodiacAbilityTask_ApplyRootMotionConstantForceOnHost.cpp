// the.quiet.string@gmail.com


#include "ZodiacAbilityTask_ApplyRootMotionConstantForceOnHost.h"

#include "AbilitySystemGlobals.h"
#include "AbilitySystemLog.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacAbilityTask_ApplyRootMotionConstantForceOnHost)

UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost* UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::ApplyRootMotionConstantForceOnHost
(
	UGameplayAbility* OwningAbility,
	FName TaskInstanceName,
	FVector WorldDirection,
	float Strength,
	float Duration,
	bool bIsAdditive,
	UCurveFloat* StrengthOverTime,
	ERootMotionFinishVelocityMode VelocityOnFinishMode,
	FVector SetVelocityOnFinish,
	float ClampVelocityOnFinish,
	bool bEnableGravity
)
{
	UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Duration(Duration);

	UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost* MyTask = NewAbilityTask<UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost>(OwningAbility, TaskInstanceName);

	MyTask->ForceName = TaskInstanceName;
	MyTask->WorldDirection = WorldDirection.GetSafeNormal();
	MyTask->Strength = Strength;
	MyTask->Duration = Duration;
	MyTask->bIsAdditive = bIsAdditive;
	MyTask->StrengthOverTime = StrengthOverTime;
	MyTask->FinishVelocityMode = VelocityOnFinishMode;
	MyTask->FinishSetVelocity = SetVelocityOnFinish;
	MyTask->FinishClampVelocity = ClampVelocityOnFinish;
	MyTask->bEnableGravity = bEnableGravity;
	MyTask->SharedInitAndApply();

	return MyTask;
}

void UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::TickTask(float DeltaTime)
{
	if (bIsFinished)
	{
		return;
	}

	Super::TickTask(DeltaTime);

	AActor* MyActor = GetOwnerActor();
	if (MyActor)
	{
		const bool bTimedOut = HasTimedOut();
		const bool bIsInfiniteDuration = Duration < 0.f;

		if (!bIsInfiniteDuration && bTimedOut)
		{
			// Task has finished
			bIsFinished = true;
			if (!bIsSimulating)
			{
				MyActor->ForceNetUpdate();
				if (ShouldBroadcastAbilityTaskDelegates())
				{
					OnFinish.Broadcast();
				}
				EndTask();
			}
		}
	}
	else
	{
		bIsFinished = true;
		EndTask();
	}
}

void UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, WorldDirection);
	DOREPLIFETIME(ThisClass, Strength);
	DOREPLIFETIME(ThisClass, Duration);
	DOREPLIFETIME(ThisClass, bIsAdditive);
	DOREPLIFETIME(ThisClass, StrengthOverTime);
	DOREPLIFETIME(ThisClass, bEnableGravity);
}

void UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::PreDestroyFromReplication()
{
	Super::PreDestroyFromReplication();
}

void UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::OnDestroy(bool AbilityIsEnding)
{
	if (MovementComponent.Get())
	{
		MovementComponent->RemoveRootMotionSourceByID(RootMotionSourceID);
	}
	
	Super::OnDestroy(AbilityIsEnding);
}

void UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost::SharedInitAndApply()
{
	UZodiacHeroAbilitySystemComponent* HeroASC = Cast<UZodiacHeroAbilitySystemComponent>(AbilitySystemComponent.Get());
	UZodiacHostAbilitySystemComponent* HostASC = HeroASC->GetHostAbilitySystemComponent();
	
	if (HostASC && HostASC->AbilityActorInfo->MovementComponent.IsValid())
	{
		MovementComponent = Cast<UCharacterMovementComponent>(HostASC->AbilityActorInfo->MovementComponent.Get());
		StartTime = GetWorld()->GetTimeSeconds();
		EndTime = StartTime + Duration;

		if (MovementComponent.Get())
		{
			ForceName = ForceName.IsNone() ? FName("AbilityTaskApplyRootMotionConstantForce"): ForceName;
			TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
			ConstantForce->InstanceName = ForceName;
			ConstantForce->AccumulateMode = bIsAdditive ? ERootMotionAccumulateMode::Additive : ERootMotionAccumulateMode::Override;
			ConstantForce->Priority = 5;
			ConstantForce->Force = WorldDirection * Strength;
			ConstantForce->Duration = Duration;
			ConstantForce->StrengthOverTime = StrengthOverTime;
			ConstantForce->FinishVelocityParams.Mode = FinishVelocityMode;
			ConstantForce->FinishVelocityParams.SetVelocity = FinishSetVelocity;
			ConstantForce->FinishVelocityParams.ClampVelocity = FinishClampVelocity;
			if (bEnableGravity)
			{
				ConstantForce->Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
			}
			RootMotionSourceID = MovementComponent->ApplyRootMotionSource(ConstantForce);
		}
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("UZodiacAbilityTask_ApplyRootMotionConstantForceOnHost called in Ability %s with null MovementComponent; Task Instance Name %s."), 
			Ability ? *Ability->GetName() : TEXT("NULL"), 
			*InstanceName.ToString());
	}}

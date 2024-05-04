// the.quiet.string@gmail.com

#include "Weapons/ZodiacGameplayAbility_Ranged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "Character/ZodiacPlayerCharacter.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Physics/ZodiacCollisionChannels.h"
#include "Teams/ZodiacTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_Ranged)

namespace ZodiacConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuration(
		TEXT("zodiac.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	static float DrawBulletHitDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHits(
		TEXT("zodiac.Weapon.DrawBulletHitDuration"),
		DrawBulletHitDuration,
		TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
		ECVF_Default);
	
	static float DrawBulletHitRadius = 7.0f;
	static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
		TEXT("zodiac.Weapon.DrawBulletHitRadius"),
		DrawBulletHitRadius,
		TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
		ECVF_Default);
}

void UZodiacGameplayAbility_Ranged::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	PlayAbilityMontage();

	CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);

	AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);
	
	FVector StartPoint = PlayerCharacter->GetPawnViewLocation();
	FVector ForwardVector = PlayerCharacter->GetBaseAimRotation().Vector();
	FVector EndPoint = StartPoint + ForwardVector * 10000.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(PlayerCharacter);
	Params.bTraceComplex = true;

	const ECollisionChannel TraceChannel = ZODIAC_TRACE_CHANNEL_WEAPON;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartPoint, EndPoint, TraceChannel, Params);

	// Execute a gameplay cue
	GCNParameter = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(HitResult);
	K2_ExecuteGameplayCueWithParams(GameplayCueTag_Firing, GCNParameter);

#if ENABLE_DRAW_DEBUG

	const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
	const FVector EndLocation = bHit ? HitResult.Location : EndPoint;
	
	if (ZodiacConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		DrawDebugLine(GetWorld(), StartPoint, EndLocation, DebugColor, false, ZodiacConsoleVariables::DrawBulletTracesDuration, 0, 1.5f);	
	}
	
	if (ZodiacConsoleVariables::DrawBulletHitDuration > 0.0f)
	{
		DrawDebugPoint(GetWorld(), EndLocation, ZodiacConsoleVariables::DrawBulletHitRadius, DebugColor, false, ZodiacConsoleVariables::DrawBulletHitRadius);
	}
	
#endif
	
	if (bHit && HitResult.GetActor())
	{
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor());
		
		UZodiacTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UZodiacTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			if (TeamSubsystem->CanCauseDamage(PlayerCharacter, HitResult.GetActor()))
			{
				if (ASC)
				{
					FGameplayAbilityTargetData_SingleTargetHit* SingleTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
					SingleTargetData->HitResult = HitResult;

					FGameplayAbilityTargetDataHandle TargetData;
					TargetData.Add(SingleTargetData);
					
					ApplyGameplayEffectToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, TargetData, DamageEffect, 1);
				}
			}
		}
	}
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UZodiacGameplayAbility_Ranged::PlayAbilityMontage()
{
	UAbilityTask_PlayMontageAndWait* Task_PlayMontageAndWait = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, MontageToPlay, 1.0f);
	Task_PlayMontageAndWait->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnd);
	Task_PlayMontageAndWait->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnd);
	Task_PlayMontageAndWait->OnCancelled.AddDynamic(this, &ThisClass::OnMontageEnd);

	Task_PlayMontageAndWait->Activate();
}

void UZodiacGameplayAbility_Ranged::OnMontageEnd()
{
}

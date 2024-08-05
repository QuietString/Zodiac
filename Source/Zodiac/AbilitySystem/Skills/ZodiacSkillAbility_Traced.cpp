// the.quiet.string@gmail.com


#include "ZodiacSkillAbility_Traced.h"

#include "GameplayCueFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/ZodiacHero.h"
#include "Character/ZodiacHostCharacter.h"
#include "Physics/ZodiacCollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbility_Traced)

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

UZodiacSkillAbility_Traced::UZodiacSkillAbility_Traced(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RateOfFire(1)
	, FireInterval(1)
{
}

void UZodiacSkillAbility_Traced::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
	// Bind target data callback
	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	OnTargetDataReadyCallbackDelegateHandle = MyASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);
}

void UZodiacSkillAbility_Traced::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyASC);

		// When ability ends, consume target data and remove delegate
		MyASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UZodiacSkillAbility_Traced::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyASC);

	// @TODO: Prediction Key?
	//FScopedPredictionWindow ScopedPrediction(MyASC, CurrentActivationInfo.GetActivationPredictionKey());

	TArray<FHitResult> FoundHits;
	PerformLocalTargeting(OUT FoundHits);

	// Fill out the target data from the hit results
	FGameplayAbilityTargetDataHandle TargetData;

	if (FoundHits.Num() > 0)
	{
		for (const FHitResult& FoundHit : FoundHits)
		{
			FGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;

			TargetData.Add(NewTargetData);
		}
	}
	
	// Process the target data immediately
	OnTargetDataReadyCallback(TargetData, FGameplayTag());	
}

void UZodiacSkillAbility_Traced::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyASC);

	if (const FGameplayAbilitySpec* AbilitySpec = MyASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		//FScopedPredictionWindow	ScopedPrediction(MyASC);

		// Take ownership of the target data to make sure no callbacks into game code invalidate it out from under us
		FGameplayAbilityTargetDataHandle LocalTargetDataHandle(MoveTemp(const_cast<FGameplayAbilityTargetDataHandle&>(InData)));

		const bool bShouldNotifyServer = CurrentActorInfo->IsLocallyControlled() && !CurrentActorInfo->IsNetAuthority();
		if (bShouldNotifyServer)
		{
			// Send client target data to server
			MyASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), LocalTargetDataHandle, ApplicationTag, MyASC->ScopedPredictionKey);
		}
		
		bool bProjectileWeapon = false;

#if WITH_SERVER_CODE
		if (!bProjectileWeapon)
		{
			if (AController* Controller = GetControllerFromActorInfo())
			{
				if (Controller->GetLocalRole() == ROLE_Authority)
				{
					// Confirm hit markers
				}
			}
		}
#endif
		
		// See if we still have ammo
		if (CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo))
		{
			// Apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
	}
	
	// We've processed the data
	MyASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UZodiacSkillAbility_Traced::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	AZodiacHero* HeroActor = Cast<AZodiacHero>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	
	FRangedSkillTraceData TraceData;
	TraceData.bCanPlayBulletFX = (HeroActor->GetNetMode() != NM_DedicatedServer);

	const FTransform TargetTransform = GetTargetingTransform(HostCharacter, HeroActor, AimTraceRule);
	TraceData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
	TraceData.StartTrace = TargetTransform.GetTranslation();
	const FVector EndTrace = TraceData.StartTrace + TraceData.AimDir * 100000.f;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(HeroActor);
	Params.bTraceComplex = true;

	const ECollisionChannel TraceChannel = ZODIAC_TRACE_CHANNEL_WEAPON;

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceData.StartTrace, EndTrace, TraceChannel, Params);

#if ENABLE_DRAW_DEBUG

	const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
	const FVector EndLocation = bHit ? HitResult.Location : EndTrace;
	
	if (ZodiacConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		DrawDebugLine(GetWorld(), TraceData.StartTrace, EndLocation, DebugColor, false, ZodiacConsoleVariables::DrawBulletTracesDuration, 0, 1.5f);	
	}
	
	if (ZodiacConsoleVariables::DrawBulletHitDuration > 0.0f)
	{
		DrawDebugPoint(GetWorld(), EndLocation, ZodiacConsoleVariables::DrawBulletHitRadius, DebugColor, false, ZodiacConsoleVariables::DrawBulletHitDuration);
	}
	
#endif

	if (!HitResult.bBlockingHit)
	{
		HitResult.Location = EndTrace;
		HitResult.ImpactPoint = EndTrace;
	}

	OutHits.Add(HitResult);
}

FVector UZodiacSkillAbility_Traced::GetTargetingSourceLocation() const
{
	return GetFXSourceLocation();
}

FTransform UZodiacSkillAbility_Traced::GetTargetingTransform() const
{
	AZodiacHero* HeroActor = Cast<AZodiacHero>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	
	return GetTargetingTransform(HostCharacter, HeroActor, AimTraceRule);
}

FTransform UZodiacSkillAbility_Traced::GetFXTargetingTransform() const
{
	AZodiacHero* HeroActor = Cast<AZodiacHero>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	
	return GetTargetingTransform(HostCharacter, HeroActor, EZodiacAbilityAimTraceRule::WeaponTowardsFocus);
}

FTransform UZodiacSkillAbility_Traced::GetTargetingTransform(APawn* OwningPawn, AActor* SourceActor, EZodiacAbilityAimTraceRule Source) const
{
	check(OwningPawn);
	check(SourceActor);
	
	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EZodiacAbilityAimTraceRule::Custom);
	
	const FVector ActorLoc = SourceActor->GetActorLocation();
	FQuat AimQuat = SourceActor->GetActorQuat();
	AController* Controller = OwningPawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;
	
	if (Controller && ((Source == EZodiacAbilityAimTraceRule::CameraTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::PawnTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus)))
	{
		// Get camera position for later
		bFoundFocus = true;

		APlayerController* PC = Cast<APlayerController>(Controller);
		if (PC)
		{
			PC->GetPlayerViewPoint(OUT CamLoc, OUT CamRot);
		}
		else // for AI controlled Pawn
		{
			SourceLoc = GetTargetingSourceLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		// Determine initial focal point to 
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);

		// Move the start and focal point up in front of pawn
		if (PC)
		{
			const FVector WeaponLoc = GetTargetingSourceLocation();
			CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
			FocalLoc = CamLoc + (AimDir * FocalDistance);
		}
		
		//Move the start to be the HeadPosition of the AI
		else if (AAIController* AIController = Cast<AAIController>(Controller))
		{
			CamLoc = SourceActor->GetActorLocation() + FVector(0, 0, OwningPawn->BaseEyeHeight);
		}

		if (Source == EZodiacAbilityAimTraceRule::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EZodiacAbilityAimTraceRule::WeaponForward) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus))
	{
		SourceLoc = GetFXSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EZodiacAbilityAimTraceRule::PawnTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

void UZodiacSkillAbility_Traced::OnRangedWeaponTargetDataReady_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	for (auto& SingleTargetData : TargetData.Data)
	{
		const FHitResult* HitResult = SingleTargetData->GetHitResult();

		GCNParameters = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(*HitResult);
		if (UZodiacAbilitySystemComponent* ZodiacASC = GetHeroAbilitySystemComponentFromActorInfo())
		{
			ZodiacASC->GameplayCueReadyData.SetGameplayTagCue(GameplayCueTag_Firing);
			ZodiacASC->GameplayCueReadyData.SetGCNParameters(GCNParameters);
			ZodiacASC->CheckAndExecuteGameplayCue();
		}

		// if (ChargeUltimateEffect)
		// {
		// 	for (auto& Actor : SingleTargetData->GetActors())
		// 	{
		// 		if (AZodiacMonster* Monster = Cast<AZodiacMonster>(Actor))
		// 		{
		// 			if (!Monster->HasMatchingGameplayTag(ZodiacGameplayTags::Status_Death_Dying))
		// 			{
		// 				// charge ultimate when any enemies is hit
		// 				ChargeUltimate();
		// 				break;
		// 			}
		// 		}
		// 	}
		// }
	}
	
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, 1);
	EffectSpecHandle.Data->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_SkillMultiplier, GetAbilityLevel());
	
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetData);
}

#if WITH_EDITOR
void UZodiacSkillAbility_Traced::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacSkillAbility_Traced, RateOfFire))
	{
		InvertValue(FireInterval, RateOfFire);
	}

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacSkillAbility_Traced, FireInterval))
	{
		InvertValue(RateOfFire, FireInterval);
	}}

void UZodiacSkillAbility_Traced::InvertValue(float& A, float& B)
{
	if (B > 0)
	{
		A = 1.0f / B;
	}
	else
	{
		A = BIG_NUMBER;
	}
}
#endif
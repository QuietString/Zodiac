// the.quiet.string@gmail.com

#include "Weapons/ZodiacSkillAbility_Ranged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "GameplayCueFunctionLibrary.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "Character/ZodiacPlayerCharacter.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Monster/ZodiacMonster.h"
#include "Physics/ZodiacCollisionChannels.h"
#include "Teams/ZodiacTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacSkillAbility_Ranged)

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

UZodiacSkillAbility_Ranged::UZodiacSkillAbility_Ranged(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RateOfFire(1)
	, FireInterval(1)
{
}

bool UZodiacSkillAbility_Ranged::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                       const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                       const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags); 
}

void UZodiacSkillAbility_Ranged::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	// Bind target data callback
	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	OnTargetDataReadyCallbackDelegateHandle = MyASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);

	//UpdateCurrentAbilitySockets();
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UZodiacSkillAbility_Ranged::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		UAbilitySystemComponent* MyAbilityComponent = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyAbilityComponent);

		// When ability ends, consume target data and remove delegate
		MyAbilityComponent->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyAbilityComponent->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

void UZodiacSkillAbility_Ranged::StartRangedWeaponTargeting()
{
	check(CurrentActorInfo);

	AActor* AvatarActor = CurrentActorInfo->AvatarActor.Get();
	check(AvatarActor);

	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyASC);
	
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

void UZodiacSkillAbility_Ranged::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData,
	FGameplayTag ApplicationTag)
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

void UZodiacSkillAbility_Ranged::OnRangedWeaponTargetDataReady_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	for (auto& SingleTargetData : TargetData.Data)
	{
		const FHitResult* HitResult = SingleTargetData->GetHitResult();

		GCNParameters = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(*HitResult);
		if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponentFromActorInfo())
		{
			ZodiacASC->GameplayCueReadyData.SetGameplayTagCue(GameplayCueTag_Firing);
			ZodiacASC->GameplayCueReadyData.SetGCNParameters(GCNParameters);
			ZodiacASC->CheckAndExecuteGameplayCue();
		}

		if (ChargeUltimateEffect)
		{
			for (auto& Actor : SingleTargetData->GetActors())
			{
				if (AZodiacMonster* Monster = Cast<AZodiacMonster>(Actor))
				{
					if (!Monster->HasMatchingGameplayTag(ZodiacGameplayTags::Status_Death_Dying))
					{
						// charge ultimate when any enemies is hit
						ChargeUltimate();
						break;
					}
				}
			}
		}
	}
	
	FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, 1);
	EffectSpecHandle.Data->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_SkillMultiplier, SkillMultiplier.GetValueAtLevel(GetAbilityLevel()));
	
	ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetData);
	//ApplyGameplayEffectToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, TargetData, DamageEffect, 1);
}

void UZodiacSkillAbility_Ranged::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	AZodiacPlayerCharacter* AvatarActor = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);
	
	FRangedSkillTraceData TraceData;
	TraceData.bCanPlayBulletFX = (AvatarActor->GetNetMode() != NM_DedicatedServer);

	const FTransform TargetTransform = GetTargetingTransform(AvatarActor, TargetingSourceRule);
	TraceData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
	TraceData.StartTrace = TargetTransform.GetTranslation();
	const FVector EndTrace = TraceData.StartTrace + TraceData.AimDir * 100000.f;
	
	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(AvatarActor);
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

FVector UZodiacSkillAbility_Ranged::GetTargetingSourceLocation() const
{
	AZodiacPlayerCharacter* ZodiacCharacter = GetZodiacCharacterFromActorInfo();
	check(ZodiacCharacter);

	FName Socket = ComboSockets.IsValidIndex(ComboIndex) ? ComboSockets[ComboIndex] : FName();
	return ZodiacCharacter->GetMesh()->GetSocketLocation(Socket);
}

FTransform UZodiacSkillAbility_Ranged::GetTargetingTransform() const
{
	AZodiacPlayerCharacter* AvatarActor = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);

	return GetTargetingTransform(AvatarActor, TargetingSourceRule);
}

FTransform UZodiacSkillAbility_Ranged::GetFXTargetingTransform() const
{
	AZodiacPlayerCharacter* AvatarActor = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);

	return GetTargetingTransform(AvatarActor, EZodiacAbilityTargetingRule::WeaponTowardsFocus);
}

FTransform UZodiacSkillAbility_Ranged::GetTargetingTransform(APawn* SourcePawn,
                                                             EZodiacAbilityTargetingRule Source) const
{
	check(SourcePawn);
	AController* SourcePawnController = SourcePawn->GetController(); 
	
	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EZodiacAbilityTargetingRule::Custom);
	
	const FVector ActorLoc = SourcePawn->GetActorLocation();
	FQuat AimQuat = SourcePawn->GetActorQuat();
	AController* Controller = SourcePawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024.0f;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;
	
	if (Controller && ((Source == EZodiacAbilityTargetingRule::CameraTowardsFocus) || (Source == EZodiacAbilityTargetingRule::PawnTowardsFocus) || (Source == EZodiacAbilityTargetingRule::WeaponTowardsFocus)))
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
			CamLoc = SourcePawn->GetActorLocation() + FVector(0, 0, SourcePawn->BaseEyeHeight);
		}

		if (Source == EZodiacAbilityTargetingRule::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EZodiacAbilityTargetingRule::WeaponForward) || (Source == EZodiacAbilityTargetingRule::WeaponTowardsFocus))
	{
		SourceLoc = GetTargetingSourceLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EZodiacAbilityTargetingRule::PawnTowardsFocus) || (Source == EZodiacAbilityTargetingRule::WeaponTowardsFocus)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

#if WITH_EDITOR
void UZodiacSkillAbility_Ranged::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacSkillAbility_Ranged, RateOfFire))
	{
		InvertValue(FireInterval, RateOfFire);
	}

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacSkillAbility_Ranged, FireInterval))
	{
		InvertValue(RateOfFire, FireInterval);
	}
}

void UZodiacSkillAbility_Ranged::InvertValue(float& A, float& B)
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
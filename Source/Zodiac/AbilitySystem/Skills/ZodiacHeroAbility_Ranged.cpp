// the.quiet.string@gmail.com


#include "ZodiacHeroAbility_Ranged.h"

#include "GameplayCueFunctionLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/ZodiacGameplayAbilityTargetData_SingleTargetHit.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Hero/ZodiacHeroAbilitySlot_RangedWeapon.h"
#include "Physics/ZodiacCollisionChannels.h"
#include "Teams/ZodiacTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbility_Ranged)

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

	static bool bDrawTargetingTraceHit = false;
	static FAutoConsoleVariableRef CVarDrawTargetingTraceHit(
		TEXT("zodiac.Weapon.DrawTargetingTraceHit"),
		bDrawTargetingTraceHit,
		TEXT("Should we do debug drawing for weapon targeting trace hit"),
		ECVF_Default);
}

FVector VRandConeNormalDistribution(const FVector& Dir, const float ConeHalfAngleRad, const float Exponent)
{
	if (ConeHalfAngleRad > 0.f)
	{
		const float ConeHalfAngleDegrees = FMath::RadiansToDegrees(ConeHalfAngleRad);

		// consider the cone a concatenation of two rotations. one "away" from the center line, and another "around" the circle
		// apply the exponent to the away-from-center rotation. a larger exponent will cluster points more tightly around the center
		const float FromCenter = FMath::Pow(FMath::FRand(), Exponent);
		const float AngleFromCenter = FromCenter * ConeHalfAngleDegrees;
		const float AngleAround = FMath::FRand() * 360.0f;

		FRotator Rot = Dir.Rotation();
		FQuat DirQuat(Rot);
		FQuat FromCenterQuat(FRotator(0.0f, AngleFromCenter, 0.0f));
		FQuat AroundQuat(FRotator(0.0f, 0.0, AngleAround));
		FQuat FinalDirectionQuat = DirQuat * AroundQuat * FromCenterQuat;
		FinalDirectionQuat.Normalize();

		return FinalDirectionQuat.RotateVector(FVector::ForwardVector);
	}
	else
	{
		return Dir.GetSafeNormal();
	}
}

UZodiacHeroAbility_Ranged::UZodiacHeroAbility_Ranged(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, RateOfFire(1)
	, FireInterval(1)
{
}

void UZodiacHeroAbility_Ranged::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate,
	const FGameplayEventData* TriggerEventData)
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate, TriggerEventData);
}

void UZodiacHeroAbility_Ranged::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                                                 const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UZodiacHeroAbilitySlot_Weapon* WeaponSlot = GetAssociatedSlot<UZodiacHeroAbilitySlot_Weapon>();
	check(WeaponSlot);
	WeaponSlot->UpdateActivationTime();
	
	// Bind target data callback
	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	OnTargetDataReadyCallbackDelegateHandle = MyASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).AddUObject(this, &ThisClass::OnTargetDataReadyCallback);
}

void UZodiacHeroAbility_Ranged::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsEndAbilityValid(Handle, ActorInfo))
	{
		if (ScopeLockCount > 0)
		{
			WaitingToExecute.Add(FPostLockDelegate::CreateUObject(this, &ThisClass::EndAbility, Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled));
		}
		
		UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
		check(MyASC);

		// When ability ends, consume target data and remove delegate
		MyASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(OnTargetDataReadyCallbackDelegateHandle);
		MyASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());

		Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	}
}

int32 UZodiacHeroAbility_Ranged::FindFirstPawnHitResult(const TArray<FHitResult>& HitResults)
{
	for (int32 Idx = 0; Idx < HitResults.Num(); ++Idx)
	{
		const FHitResult& CurHitResult = HitResults[Idx];
		if (CurHitResult.HitObjectHandle.DoesRepresentClass(APawn::StaticClass()))
		{
			// If we hit a pawn, we're good
			return Idx;
		}
		else
		{
			AActor* HitActor = CurHitResult.HitObjectHandle.FetchActor();
			if ((HitActor != nullptr) && (HitActor->GetAttachParentActor() != nullptr) && (Cast<APawn>(HitActor->GetAttachParentActor()) != nullptr))
			{
				// If we hit something attached to a pawn, we're good
				return Idx;
			}
		}
	}

	return INDEX_NONE;
}

FHitResult UZodiacHeroAbility_Ranged::WeaponTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated,
	TArray<FHitResult>& OutHitResults) const
{
	TArray<FHitResult> HitResults;
	
	FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/ true, /*IgnoreActor=*/ GetAvatarActorFromActorInfo());
	TraceParams.bReturnPhysicalMaterial = true;
	AddAdditionalTraceIgnoreActors(TraceParams);
	//TraceParams.bDebugQuery = true;
	
	const ECollisionChannel TraceChannel = ZODIAC_TRACE_CHANNEL_WEAPON;
	
	if (SweepRadius > 0.0f)
	{
		GetWorld()->SweepMultiByChannel(HitResults, StartTrace, EndTrace, FQuat::Identity, TraceChannel, FCollisionShape::MakeSphere(SweepRadius), TraceParams);
	}
	else
	{
		GetWorld()->LineTraceMultiByChannel(HitResults, StartTrace, EndTrace, TraceChannel, TraceParams);
	}

	FHitResult Hit(ForceInit);
	if (HitResults.Num() > 0)
	{
		// Filter the output list to prevent multiple hits on the same actor;
		// this is to prevent a single bullet dealing damage multiple times to
		// a single actor if using an overlap trace
		for (FHitResult& CurHitResult : HitResults)
		{
			auto Pred = [&CurHitResult](const FHitResult& Other)
			{
				return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
			};

			if (!OutHitResults.ContainsByPredicate(Pred))
			{
				OutHitResults.Add(CurHitResult);
			}
		}

		Hit = OutHitResults.Last();
	}
	else
	{
		Hit.TraceStart = StartTrace;
		Hit.TraceEnd = EndTrace;
	}

	return Hit;
}

void UZodiacHeroAbility_Ranged::AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const
{
	if (AActor* Avatar = GetAvatarActorFromActorInfo())
	{
		// Ignore any actors attached to the avatar doing the shooting
		TArray<AActor*> AttachedActors;
		Avatar->GetAttachedActors(OUT AttachedActors);
		TraceParams.AddIgnoredActors(AttachedActors);
	}
}

void UZodiacHeroAbility_Ranged::StartRangedWeaponTargeting()
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
			FZodiacGameplayAbilityTargetData_SingleTargetHit* NewTargetData = new FZodiacGameplayAbilityTargetData_SingleTargetHit();
			NewTargetData->HitResult = FoundHit;

			TargetData.Add(NewTargetData);
		}
		
		// Process the target data immediately
		OnTargetDataReadyCallback(TargetData, FGameplayTag());	
	}
}

void UZodiacHeroAbility_Ranged::OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag)
{
	UAbilitySystemComponent* MyASC = CurrentActorInfo->AbilitySystemComponent.Get();
	check(MyASC);

	if (const FGameplayAbilitySpec* AbilitySpec = MyASC->FindAbilitySpecFromHandle(CurrentSpecHandle))
	{
		FScopedPredictionWindow	ScopedPrediction(MyASC);
		
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
			if (UZodiacHeroAbilitySlot_RangedWeapon* WeaponSlot = GetAssociatedSlot<UZodiacHeroAbilitySlot_RangedWeapon>())
			{
				WeaponSlot->AddSpread();
			}
			
			// Apply effects to the targets
			OnRangedWeaponTargetDataReady(LocalTargetDataHandle);
		}
	}
	
	// We've processed the data
	MyASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey());
}

void UZodiacHeroAbility_Ranged::PerformLocalTargeting(TArray<FHitResult>& OutHits)
{
	AZodiacHeroCharacter* HeroActor = Cast<AZodiacHeroCharacter>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	UZodiacHeroAbilitySlot_RangedWeapon* WeaponSlot = GetAssociatedSlot<UZodiacHeroAbilitySlot_RangedWeapon>();

	if (HeroActor && HostCharacter->IsLocallyControlled() && WeaponSlot)
	{
		FRangedAbilityTraceData TraceData;
		TraceData.WeaponData = WeaponSlot;
		TraceData.bCanPlayBulletFX = (HeroActor->GetNetMode() != NM_DedicatedServer);

		const FTransform TargetTransform = GetTargetingTransform(HostCharacter, HeroActor, AimTraceRule);
		TraceData.AimDir = TargetTransform.GetUnitAxis(EAxis::X);
		TraceData.StartTrace = TargetTransform.GetTranslation();
		const FVector EndTrace = TraceData.StartTrace + TraceData.AimDir * 100000.f;

#if ENABLE_DRAW_DEBUG
		if (ZodiacConsoleVariables::DrawBulletTracesDuration > 0.0f)
		{
			static float DebugThickness = 2.0f;
			DrawDebugLine(GetWorld(), TraceData.StartTrace, TraceData.StartTrace + (TraceData.AimDir * 100.0f), FColor::Yellow, false, ZodiacConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
		}
#endif

		TraceBulletsInCartridge(TraceData, OUT OutHits);
	}
}

void UZodiacHeroAbility_Ranged::TraceBulletsInCartridge(const FRangedAbilityTraceData& InputData, TArray<FHitResult>& OutHits)
{
	UZodiacHeroAbilitySlot_RangedWeapon* WeaponData = InputData.WeaponData;
	check(WeaponData);

	const int32 BulletsPerCartridge = WeaponData->GetBulletsPerCartridge();

	for (int32 BulletIndex = 0; BulletIndex < BulletsPerCartridge; ++BulletIndex)
	{
		const float BaseSpreadAngle = WeaponData->GetCalculatedSpreadAngle();
		const float SpreadAngleMultiplier = WeaponData->GetCalculatedSpreadAngleMultiplier();
		const float ActualSpreadAngle = BaseSpreadAngle * SpreadAngleMultiplier;

		const float HalfSpreadAngleInRadians = FMath::DegreesToRadians(ActualSpreadAngle * 0.5f);

		const FVector BulletDir = VRandConeNormalDistribution(InputData.AimDir, HalfSpreadAngleInRadians, WeaponData->GetSpreadExponent());

		const FVector EndTrace = InputData.StartTrace + (BulletDir * WeaponData->GetMaxDamageRange());
		FVector HitLocation = EndTrace;

		TArray<FHitResult> AllImpacts;

		FHitResult Impact = DoSingleBulletTrace(InputData.StartTrace, EndTrace, WeaponData->GetBulletTraceSweepRadius(), false, OUT AllImpacts);

		const AActor* HitActor = Impact.GetActor();

		if (HitActor)
		{
#if ENABLE_DRAW_DEBUG
			if (ZodiacConsoleVariables::DrawBulletHitDuration > 0.0f)
			{
				DrawDebugPoint(GetWorld(), Impact.ImpactPoint, ZodiacConsoleVariables::DrawBulletHitRadius, FColor::Red, false, ZodiacConsoleVariables::DrawBulletHitRadius);
			}
#endif

			if (AllImpacts.Num() > 0)
			{
				OutHits.Append(AllImpacts);
			}

			HitLocation = Impact.ImpactPoint;
		}

		// Make sure there's always an entry in OutHits so the direction can be used for tracers, etc...
		if (OutHits.Num() == 0)
		{
			if (!Impact.bBlockingHit)
			{
				// Locate the fake 'impact' at the end of the trace
				Impact.Location = EndTrace;
				Impact.ImpactPoint = EndTrace;
			}

			OutHits.Add(Impact);
		}
	}
}

FHitResult UZodiacHeroAbility_Ranged::DoSingleBulletTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated,
	TArray<FHitResult>& OutHits) const
{
#if ENABLE_DRAW_DEBUG
	if (ZodiacConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		static float DebugThickness = 1.0f;
		DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, ZodiacConsoleVariables::DrawBulletTracesDuration, 0, DebugThickness);
	}
#endif

	FHitResult Impact;

	// Trace and process instant hit if something was hit
	// First trace without using sweep radius
	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		Impact = WeaponTrace(StartTrace, EndTrace, /*SweepRadius=*/ 0.0f, bIsSimulated, /*out*/ OutHits);
	}

	if (FindFirstPawnHitResult(OutHits) == INDEX_NONE)
	{
		// If this weapon didn't hit anything with a line trace and supports a sweep radius, try that
		if (SweepRadius > 0.0f)
		{
			TArray<FHitResult> SweepHits;
			Impact = WeaponTrace(StartTrace, EndTrace, SweepRadius, bIsSimulated, /*out*/ SweepHits);

			// If the trace with sweep radius enabled hit a pawn, check if we should use its hit results
			const int32 FirstPawnIdx = FindFirstPawnHitResult(SweepHits);
			if (SweepHits.IsValidIndex(FirstPawnIdx))
			{
				// If we had a blocking hit in our line trace that occurs in SweepHits before our
				// hit pawn, we should just use our initial hit results since the Pawn hit should be blocked
				bool bUseSweepHits = true;
				for (int32 Idx = 0; Idx < FirstPawnIdx; ++Idx)
				{
					const FHitResult& CurHitResult = SweepHits[Idx];

					auto Pred = [&CurHitResult](const FHitResult& Other)
					{
						return Other.HitObjectHandle == CurHitResult.HitObjectHandle;
					};
					if (CurHitResult.bBlockingHit && OutHits.ContainsByPredicate(Pred))
					{
						bUseSweepHits = false;
						break;
					}
				}

				if (bUseSweepHits)
				{
					OutHits = SweepHits;
				}
			}
		}
	}

	return Impact;
}

FTransform UZodiacHeroAbility_Ranged::GetTargetingTransform(const EZodiacAbilityAimTraceRule TraceRule) const
{
	AZodiacHeroCharacter* HeroActor = Cast<AZodiacHeroCharacter>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	
	return GetTargetingTransform(HostCharacter, HeroActor, TraceRule);
}

FTransform UZodiacHeroAbility_Ranged::GetWeaponTargetingTransform() const
{
	AZodiacHeroCharacter* HeroActor = Cast<AZodiacHeroCharacter>(GetCurrentActorInfo()->AvatarActor);
	AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCurrentActorInfo()->OwnerActor);
	
	return GetTargetingTransform(HostCharacter, HeroActor, EZodiacAbilityAimTraceRule::WeaponTowardsFocus);
}

FTransform UZodiacHeroAbility_Ranged::GetTargetingTransform(APawn* OwningPawn, AActor* SourceActor, EZodiacAbilityAimTraceRule Source) const
{
	check(OwningPawn);
	check(SourceActor);
	
	// The caller should determine the transform without calling this if the mode is custom!
	check(Source != EZodiacAbilityAimTraceRule::Custom);
	
	const FVector ActorLoc = SourceActor->GetActorLocation();
	FQuat AimQuat = SourceActor->GetActorQuat();
	AController* Controller = OwningPawn->Controller;
	FVector SourceLoc;

	double FocalDistance = 1024;
	FVector FocalLoc;

	FVector CamLoc;
	FRotator CamRot;
	bool bFoundFocus = false;
	
	if (Controller && ((Source == EZodiacAbilityAimTraceRule::CameraTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::PawnTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus)) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocusHit))
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
			SourceLoc = GetWeaponLocation();
			CamLoc = SourceLoc;
			CamRot = Controller->GetControlRotation();
		}

		
		// Determine initial focal point to 
		FVector AimDir = CamRot.Vector().GetSafeNormal();
		FocalLoc = CamLoc + (AimDir * FocalDistance);
		
		// Move the start and focal point up in front of pawn
		if (PC)
		{
			if (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocusHit)
			{
				FocalDistance = BIG_NUMBER;
				CamLoc += AimDir * 100.0f;
			}
			else
			{
				const FVector WeaponLoc = GetWeaponLocation();
				CamLoc = FocalLoc + (((WeaponLoc - FocalLoc) | AimDir) * AimDir);
			}

			FocalLoc = CamLoc + (AimDir * FocalDistance);
		}
		
		//Move the start to be the HeadPosition of the AI
		else if (AAIController* AIController = Cast<AAIController>(Controller))
		{
			CamLoc = SourceActor->GetActorLocation() + FVector(0, 0, OwningPawn->BaseEyeHeight);
		}

		if (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocusHit)
		{
			FocalDistance = BIG_NUMBER;
			
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.bReturnPhysicalMaterial = true;
			Params.AddIgnoredActor(SourceActor);
			Params.AddIgnoredActor(OwningPawn);
			Params.bTraceComplex = true;

			const ECollisionChannel TraceChannel = ECC_Visibility;
			bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, CamLoc, FocalLoc, TraceChannel, Params);

			float FallbackFocalDistance = 2000.0f;
			FVector FallbackFocalLoc = CamLoc + (AimDir * FallbackFocalDistance);
			FocalLoc =  bHit ? HitResult.ImpactPoint : FallbackFocalLoc;

#if ENABLE_DRAW_DEBUG
			if (ZodiacConsoleVariables::bDrawTargetingTraceHit)
			{
				const FColor DebugColor = bHit ? FColor::Cyan : FColor::Silver;
				const FVector EndLocation = bHit ? HitResult.ImpactPoint : FocalLoc;
				DrawDebugLine(GetWorld(), CamLoc, EndLocation, DebugColor, false, 3, 0, 1.5f);
			}
#endif
		}
		
		if (Source == EZodiacAbilityAimTraceRule::CameraTowardsFocus)
		{
			// If we're camera -> focus then we're done
			return FTransform(CamRot, CamLoc);
		}
	}

	if ((Source == EZodiacAbilityAimTraceRule::WeaponForward) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocusHit))
	{
		SourceLoc = GetWeaponLocation();
	}
	else
	{
		// Either we want the pawn's location, or we failed to find a camera
		SourceLoc = ActorLoc;
	}

	if (bFoundFocus && ((Source == EZodiacAbilityAimTraceRule::PawnTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocus) || (Source == EZodiacAbilityAimTraceRule::WeaponTowardsFocusHit)))
	{
		// Return a rotator pointing at the focal point from the source
		return FTransform((FocalLoc - SourceLoc).Rotation(), SourceLoc);
	}

	// If we got here, either we don't have a camera or we don't want to use it, either way go forward
	return FTransform(AimQuat, SourceLoc);
}

void UZodiacHeroAbility_Ranged::OnRangedWeaponTargetDataReady_Implementation(const FGameplayAbilityTargetDataHandle& TargetData)
{
	if (TargetData.Data.IsEmpty())
	{
		return;
	}

	UZodiacHeroAbilitySlot* Slot = GetAssociatedSlot();
	
	if (UZodiacAbilitySystemComponent* HostASC = Cast<UZodiacAbilitySystemComponent>(GetHostAbilitySystemComponent()))
	{
		const FHitResult* FirstHitResult = TargetData.Get(0)->GetHitResult();
		GameplayCueParams_Firing = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(*FirstHitResult);
		GameplayCueParams_Firing.SourceObject = GetSocket();
		HostASC->ExecuteGameplayCue(GameplayCueTag_Firing, GameplayCueParams_Firing);
		AdvanceCombo();

		for (auto& SingleTargetData : TargetData.Data)
		{
			const FHitResult* HitResult = SingleTargetData->GetHitResult();
			if (HitResult->bBlockingHit)
			{
				GameplayCueParams_Impact = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(*HitResult);
				GameplayCueParams_Impact.SourceObject = Slot;
				HostASC->ExecuteGameplayCue(GameplayCueTag_Impact, GameplayCueParams_Impact);
			}
		
			if (ChargeUltimateEffectClass && HasAuthorityOrPredictionKey(CurrentActorInfo, &CurrentActivationInfo))
			{
				for (auto& Actor : SingleTargetData->GetActors())
				{
					UZodiacTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UZodiacTeamSubsystem>();
					if (TeamSubsystem->CanCauseDamage(GetZodiacHostCharacterFromActorInfo(), Actor.Get(), false))
					{
						if (IGameplayTagAssetInterface* TagAssetInterface = Cast<IGameplayTagAssetInterface>(Actor))
						{
							if (!TagAssetInterface->HasMatchingGameplayTag(ZodiacGameplayTags::Status_Death_Dying))
							{
								// charge ultimate when any enemies is hit
								ChargeUltimate();
								break;
							}
						}
					}
				}
			}
		}
	}
	
	if (DamageEffect)
	{
		FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffect, 1);
		EffectSpecHandle.Data.Get()->GetContext().AddSourceObject(Slot);
		EffectSpecHandle.Data->SetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_Damage, DamagePerBullet.GetValue());
		ApplyGameplayEffectSpecToTarget(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, EffectSpecHandle, TargetData);	
	}
}

#if WITH_EDITOR
void UZodiacHeroAbility_Ranged::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacHeroAbility_Ranged, RateOfFire))
	{
		InvertValue(FireInterval, RateOfFire);
	}

	if (PropertyChangedEvent.Property->GetName() == GET_MEMBER_NAME_CHECKED(UZodiacHeroAbility_Ranged, FireInterval))
	{
		InvertValue(RateOfFire, FireInterval);
	}
}

void UZodiacHeroAbility_Ranged::InvertValue(float& A, float& B)
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
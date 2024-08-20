// the.quiet.string@gmail.com


#include "ZodiacHeroAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHostAnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHero.h"
#include "Character/ZodiacHostCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAnimInstance)

void UZodiacHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UZodiacHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	AActor* OwningActor = GetOwningActor();
	ParentCharacter = GetParentCharacter();
	ParentAnimInstance = GetParentAnimInstance();
	
	if (!ParentCharacter)
	{
		return;
	}

	UpdateMovementData();
	//UpdateRotationData(DeltaSeconds, OwningActor);
	UpdateAimingData(DeltaSeconds);
	UpdateBlendData(DeltaSeconds);
}

AZodiacCharacter* UZodiacHeroAnimInstance::GetParentCharacter() const
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwningActor()))
	{
		return ZodiacCharacter;
	}
	
	if (AZodiacHero* Hero = Cast<AZodiacHero>(GetOwningActor()))
	{
		return Hero->GetHostCharacter();
	}

	return nullptr;
}

UZodiacHostAnimInstance* UZodiacHeroAnimInstance::GetParentAnimInstance() const
{
	if (ParentCharacter)
	{
		return  Cast<UZodiacHostAnimInstance>(ParentCharacter->GetMesh()->GetAnimInstance());
	}

	return nullptr;
}

void UZodiacHeroAnimInstance::OnAimingChanged(bool bHasActivated)
{
	PlayHideOrRevealGunsMontage(bHasActivated);
}

void UZodiacHeroAnimInstance::OnIsWeaponReadyChanged(bool InIsReady)
{
	bIsWeaponReady = InIsReady;
}

void UZodiacHeroAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bActive)
{
	if (Tag == ZodiacGameplayTags::Status_WeaponReady)
	{
		bIsWeaponReady = bActive;
	}
	else if (Tag == ZodiacGameplayTags::Status_Focus)
	{
		bIsFocus = bActive;
	}
}

void UZodiacHeroAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	//GameplayTagPropertyMap.Initialize(this, InASC);
}

void UZodiacHeroAnimInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ThisClass, bShouldReveal_LeftPistol);
	//DOREPLIFETIME(ThisClass, bShouldReveal_RightPistol);
}

void UZodiacHeroAnimInstance::UpdateMovementData()
{
	//bIsFocus = HostAnimInstance->bIsFocus;
	bIsADS = ParentAnimInstance->bIsADS;
	bIsTraversal = ParentAnimInstance->CustomMovement == MOVE_Traversal;
}

void UZodiacHeroAnimInstance::UpdateRotationData(float DeltaSeconds, AActor* OwningActor)
{
	
}

void UZodiacHeroAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	if (ParentAnimInstance)
	{
		FRotator AimRotation = ParentCharacter->GetBaseAimRotation();
		FRotator RootTransform = ParentAnimInstance->RootTransform.Rotator();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootTransform);

		bShouldRaise_RightArm = (!bIsTraversal) && bIsFocus;
		bShouldRaise_LeftArm = (!bIsTraversal) && bIsADS;
		
		bool bIsSlotActive = IsSlotActive(FName("Weapon_Additive"));
		bool bIsRightPistolSlotActive = IsSlotActive(FName("Weapon_RightPistolAdditive"));

		bShouldReveal_LeftPistol = bAnimNotify_RevealLeftPistol || (bIsADS && bIsWeaponReady) || bIsSlotActive;
		bShouldReveal_RightPistol = (bAnimNotify_RevealRightPistol && bIsFocus) || (bIsFocus && bIsWeaponReady) || bIsRightPistolSlotActive || bShouldReveal_LeftPistol;

		// if (bIsGunsHidden && bIsAiming)
		// {
		// 	// set true from an animation blueprint
		// 	bIsGunsHidden = false;
		// }

		bApplyAimOffSet = (bShouldRaise_LeftArm | bShouldRaise_RightArm | bIsADS);

		AimPitch = (bApplyAimOffSet) ? Delta.Pitch : FMath::Lerp(AimPitch, 0.0f, DeltaSeconds * 5.0f);
		AimYaw = FMath::Lerp(Delta.Yaw, AimYaw, DeltaSeconds * AimYawBlendSpeed);
		FMath::Clamp(AimYaw, AimYawClampRange.X, AimYawClampRange.Y);
	}
}

void UZodiacHeroAnimInstance::UpdateBlendData(float DeltaSeconds)
{
	// if (bIsWeaponReady)
	// {
	// 	PistolBlendAlpha = 0.0f;
	// 	PistolScale = 1.0f;
	// }
	// else if (bIsADS) // when starts or leaving aiming movement
	// {
	// 	float ScaleCurve = GetCurveValue(TEXT("PistolScaleCurve"));
	// 	PistolBlendAlpha = GetCurveValue(TEXT("PistolBlendAlpha"));
	//
	// 	PistolScale = ScaleCurve;
	// }
	// else
	// {
	// 	PistolBlendAlpha = 1.0f;
	// 	PistolScale = 0.0f;
	// }
	//UE_LOG(LogTemp, Warning, TEXT("Alpha: %.1f, Scale: %.1f"), PistolBlendAlpha, PistolScale);

	float RightPistolTargetAlpha = (bShouldReveal_RightPistol) ? 0 : 1;
	RightPistolScaleAlpha = FMath::Lerp(RightPistolScaleAlpha, RightPistolTargetAlpha, DeltaSeconds * RightPistolAlphaSpeedMultiplier);
	
	float LeftPistolTargetAlpha = (bShouldReveal_LeftPistol) ? 0 : 1;
	LeftPistolScaleAlpha = FMath::Lerp(LeftPistolScaleAlpha, LeftPistolTargetAlpha, DeltaSeconds * LeftPistolAlphaSpeedMultiplier);
}
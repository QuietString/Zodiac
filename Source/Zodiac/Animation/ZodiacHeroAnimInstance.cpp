// the.quiet.string@gmail.com


#include "ZodiacHeroAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHostAnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Kismet/KismetMathLibrary.h"

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
	
	if (AZodiacHeroCharacter* Hero = Cast<AZodiacHeroCharacter>(GetOwningActor()))
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

void UZodiacHeroAnimInstance::UpdateMovementData()
{
	bIsFocus = ParentAnimInstance->bIsFocus;
	bIsADS = ParentAnimInstance->bIsADS;
	bIsMoving = ParentAnimInstance->bIsMoving; 
	bIsTraversal = ParentAnimInstance->CustomMovement == Move_Custom_Traversal;
	bIsWeaponReady = ParentAnimInstance->bIsWeaponReady;
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
		
		bool bIsSlotActive = IsSlotActive(FName("Weapon_Additive"));
		bool bIsRightPistolSlotActive = IsSlotActive(FName("Weapon_RightPistolAdditive"));

		bShouldReveal_LeftPistol = bAnimNotify_RevealLeftPistol || (bIsADS && bIsWeaponReady);
		bShouldReveal_RightPistol = (bAnimNotify_RevealRightPistol && bIsFocus) || (bIsFocus && bIsWeaponReady) || bIsRightPistolSlotActive || bShouldReveal_LeftPistol;
		
		AimPitch = Delta.Pitch;
		AimYaw = Delta.Yaw;
		FMath::Clamp(AimYaw, AimYawClampRange.X, AimYawClampRange.Y);
	}
}

void UZodiacHeroAnimInstance::UpdateBlendData(float DeltaSeconds)
{
	float RightPistolTargetAlpha = (bShouldReveal_RightPistol) ? 0 : 1;
	RightPistolScaleAlpha = FMath::Lerp(RightPistolScaleAlpha, RightPistolTargetAlpha, DeltaSeconds * RightPistolAlphaSpeedMultiplier);
	
	float LeftPistolTargetAlpha = (bShouldReveal_LeftPistol) ? 0 : 1;
	LeftPistolScaleAlpha = FMath::Lerp(LeftPistolScaleAlpha, LeftPistolTargetAlpha, DeltaSeconds * LeftPistolAlphaSpeedMultiplier);
}
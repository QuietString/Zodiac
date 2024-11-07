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
	UpdateAimingData(DeltaSeconds);
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

void UZodiacHeroAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bHasTag)
{
	if (Tag == ZodiacGameplayTags::Status_Focus)
	{
		bIsFocus = bHasTag;
	}
	else if (Tag == ZodiacGameplayTags::Status_Death)
	{
		bIsDead = bHasTag;
	}

	else if (Tag == ZodiacGameplayTags::Status_ADS)
	{
		bIsADS = bHasTag;
	}
}

void UZodiacHeroAnimInstance::UpdateMovementData()
{
	bIsMoving = ParentAnimInstance->bIsMoving; 
	bIsTraversal = ParentAnimInstance->CustomMovement == Move_Custom_Traversal;
}

void UZodiacHeroAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	if (ParentAnimInstance)
	{
		FRotator AimRotation = ParentCharacter->GetBaseAimRotation();
		FRotator RootTransform = ParentAnimInstance->RootTransform.Rotator();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootTransform);

		RootYawOffset = - Delta.Yaw;
		AimYaw = Delta.Yaw;
		AimPitch = Delta.Pitch;
		FMath::Clamp(AimYaw, AimYawClampRange.X, AimYawClampRange.Y);
	}
}
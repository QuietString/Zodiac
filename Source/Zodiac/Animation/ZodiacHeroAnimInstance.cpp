// the.quiet.string@gmail.com


#include "ZodiacHeroAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHostAnimInstance.h"
#include "Character/ZodiacHero.h"
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
	HostCharacter = GetHostCharacter();
	HostAnimInstance = GetHostAnimInstance();
	
	if (!HostCharacter)
	{
		return;
	}

	//UpdateHostData(HostCharacter);
	//UpdateRotationData(DeltaSeconds, OwningActor);
	UpdateAimingData(DeltaSeconds);
}

AZodiacHostCharacter* UZodiacHeroAnimInstance::GetHostCharacter() const
{
	if (AZodiacHero* Hero = Cast<AZodiacHero>(GetOwningActor()))
	{
		return Hero->GetHostCharacter();
	}

	return nullptr;
}

UZodiacHostAnimInstance* UZodiacHeroAnimInstance::GetHostAnimInstance() const
{
	if (HostCharacter)
	{
		return  Cast<UZodiacHostAnimInstance>(HostCharacter->GetMesh()->GetAnimInstance());
	}

	return nullptr;
}

void UZodiacHeroAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	GameplayTagPropertyMap.Initialize(this, InASC);
}

void UZodiacHeroAnimInstance::UpdateRotationData(float DeltaSeconds, AActor* OwningActor)
{
	
}

void UZodiacHeroAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	if (HostAnimInstance)
	{
		FRotator AimRotation = HostCharacter->GetBaseAimRotation();
		FRotator RootTransform = HostAnimInstance->RootTransform.Rotator();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootTransform);

		bIsAiming = HostAnimInstance->GetIsAiming();
		if (bIsGunsHidden && bIsAiming)
		{
			// set true from an animation blueprint
			bIsGunsHidden = false;
		}

		AimPitch = (bIsGunsHidden && !bIsAiming) ? FMath::Lerp(AimPitch, 0.0f, DeltaSeconds * 5.0f) : Delta.Pitch;
		AimYaw = Delta.Yaw;
	}
}
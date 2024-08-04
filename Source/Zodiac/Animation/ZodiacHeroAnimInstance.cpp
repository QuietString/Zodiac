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
	AZodiacHostCharacter* HostCharacter = GetHostCharacter();
	
	if (!HostCharacter)
	{
		return;
	}
	
	UpdateRotationData(DeltaSeconds, OwningActor);
	UpdateAimingData(HostCharacter);
}

void UZodiacHeroAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	GameplayTagPropertyMap.Initialize(this, InASC);
}

void UZodiacHeroAnimInstance::UpdateRotationData(float DeltaSeconds, AActor* OwningActor)
{
}

void UZodiacHeroAnimInstance::UpdateAimingData(AZodiacHostCharacter* HostCharacter)
{
	if (UZodiacHostAnimInstance* HostAnimInstance = Cast<UZodiacHostAnimInstance>(HostCharacter->GetMesh()->GetAnimInstance()))
	{
		FRotator AimRotation = HostCharacter->GetBaseAimRotation();
		FRotator RootTransform = HostAnimInstance->RootTransform.Rotator();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootTransform);

		bIsAiming = HostAnimInstance->GetIsAiming();

		AimPitch = bIsAiming ? Delta.Pitch : 0.0f;
		AimYaw = Delta.Yaw;
	}
}

AZodiacHostCharacter* UZodiacHeroAnimInstance::GetHostCharacter() const
{
	if (AZodiacHero* Hero = Cast<AZodiacHero>(GetOwningActor()))
	{
		return Hero->GetHostCharacter();
	}

	return nullptr;
}
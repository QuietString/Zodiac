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
	
	if (!ParentCharacter || !ParentAnimInstance)
	{
		return;
	}

	UpdateMovementData();
	UpdateRotationData();
	UpdateAimingData(DeltaSeconds);
}

void UZodiacHeroAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	GameplayTagPropertyMap.Initialize(this, InASC);
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
	bIsTraversal = ParentAnimInstance->bIsTraversal;

	ExtendedMovementMode = ParentAnimInstance->ExtendedMovementMode;
	
	bIsStrafing = (ExtendedMovementMode != EZodiacExtendedMovementMode::Sprinting);
	MovementAngle = ParentAnimInstance->MovementAngle;
}

void UZodiacHeroAnimInstance::UpdateRotationData()
{
	RootYaw_WorldSpace = ParentAnimInstance->RootTransform.Rotator().Yaw - 90.f;
}

void UZodiacHeroAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	FRotator ActorRotation = ParentCharacter->GetActorRotation();
	FRotator AimRotation = ParentCharacter->GetBaseAimRotation();
	FRotator ControlRotation = ParentCharacter->GetControlRotation();
	//FRotator TargetRotation = ParentCharacter->IsLocallyControlled() ? ControlRotation : AimRotation; 
	
	FRotator RootTransform = ParentAnimInstance->RootTransform.Rotator();
	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, RootTransform);
	
	RootYawOffset = - Delta.Yaw;
	if (!bIsStrafing)
	{
		FRotator DeltaRotator = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, ActorRotation);
		RootYawOffset += DeltaRotator.Yaw;
	}
	
	AimYaw = Delta.Yaw;
	AimPitch = Delta.Pitch;
}

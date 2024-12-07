// the.quiet.string@gmail.com


#include "ZodiacHostAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "Character/ZodiacCharacter.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAnimInstance)

void UZodiacHostAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);

	GameplayTagPropertyMap.Initialize(this, InASC);
}

void UZodiacHostAnimInstance::NativeInitializeAnimation()
{
	if (AZodiacCharacter* PawnOwner = Cast<AZodiacCharacter>(TryGetPawnOwner()))
	{
		OwningCharacter = PawnOwner;
		OwningCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate::CreateUObject(this, &ThisClass::InitializeWithAbilitySystem));
	}
}

void UZodiacHostAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (AZodiacCharacter* PawnOwner = Cast<AZodiacCharacter>(TryGetPawnOwner()))
	{
		OwningCharacter = PawnOwner;
		
		if (UZodiacCharacterMovementComponent* CharacterMovement = Cast<UZodiacCharacterMovementComponent>(OwningCharacter->GetCharacterMovement()))
		{
			ZodiacCharMovComp = CharacterMovement;	
		}
	}
}

void UZodiacHostAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (OwningCharacter && ZodiacCharMovComp)
	{
		UpdateLocationData(DeltaSeconds);
		
		UpdateVelocityData();
		UpdateMovementData();
		UpdateGait();
		UpdateAccelerationData(DeltaSeconds);
		UpdateAimingData();
	}
}

void UZodiacHostAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	const FVector PositionDiff = OwningCharacter->GetActorLocation() - WorldLocation;
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(PositionDiff);

	WorldLocation = OwningCharacter->GetActorLocation();
	
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
}

void UZodiacHostAnimInstance::UpdateMovementData()
{
	ExtendedMovementMode = ZodiacCharMovComp->GetExtendedMovementMode();

	bIsTraversal = OwningCharacter->bIsTraversal;
	bIsMoving = !Velocity.Equals(FVector(0, 0, 0), 0.1) && !FutureVelocity.Equals(FVector(0, 0, 0), 0.1);
}

void UZodiacHostAnimInstance::UpdateVelocityData()
{
	Velocity_Last = Velocity;
	Velocity = ZodiacCharMovComp->Velocity;
	Speed2D = UKismetMathLibrary::VSizeXY(Velocity);
	bHasVelocity = Speed2D > 5.0f;
	if (bHasVelocity) Velocity_LastNonZero = Velocity;
}

void UZodiacHostAnimInstance::UpdateAccelerationData(float DeltaSeconds)
{
	AccelerationFromVelocityDiff = (Velocity - Velocity_Last) / (FMath::Max(DeltaSeconds, 0.001f));
	
	Acceleration = ZodiacCharMovComp->GetCurrentAcceleration();
	AccelerationAmount = Acceleration.Length() / ZodiacCharMovComp->MaxAcceleration;
	bHasAcceleration = AccelerationAmount > 0;
}

void UZodiacHostAnimInstance::UpdateAimingData()
{
	AimPitch = UKismetMathLibrary::NormalizeAxis(TryGetPawnOwner()->GetBaseAimRotation().Pitch);
}

void UZodiacHostAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bHasTag)
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
	else if (Tag == ZodiacGameplayTags::Status_Stun)
	{
		bIsStun = bHasTag;
	}
}

void UZodiacHostAnimInstance::UpdateGait()
{
	if (ZodiacCharMovComp)
	{
		Gait_LastFrame = Gait;
		
		EMovementMode MovementMode = ZodiacCharMovComp->MovementMode;

		if (MovementMode == MOVE_Walking)
		{
			switch (ZodiacCharMovComp->GetExtendedMovementMode())
			{
			case EZodiacExtendedMovementMode::Running:
				Gait = Gait_Run;
				return;
				
			case EZodiacExtendedMovementMode::Walking:
			default:
				Gait = Gait_Walk;
				return;
			}
		}
	}
}
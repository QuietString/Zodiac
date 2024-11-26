// the.quiet.string@gmail.com


#include "ZodiacHostAnimInstance.h"

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
		UpdateAccelerationData(DeltaSeconds);

		UpdateAimingData();
	}
}

void UZodiacHostAnimInstance::UpdateMovementData()
{
	CustomMovement_Last = CustomMovement;
	CustomMovement = EZodiacCustomMovementMode(ZodiacCharMovComp->CustomMovementMode);
	
	ExtendedMovementMode_Last = ExtendedMovementMode;
	
	switch (ZodiacCharMovComp->MovementMode)
	{
	case MOVE_Walking:
	case MOVE_NavWalking:
		switch (CustomMovement)
		{
		case Move_Custom_Walking:
			ExtendedMovementMode = EZodiacExtendedMovementMode::Walking;
				break;
		case Move_Custom_Running:
			ExtendedMovementMode = EZodiacExtendedMovementMode::Running;
				break;
		case Move_Custom_Traversal:
			ExtendedMovementMode = EZodiacExtendedMovementMode::Traversal;
				break;
		case MOVE_None:
		default:
			ExtendedMovementMode = EZodiacExtendedMovementMode::Walking;
		}
		
	case MOVE_Falling:
		ExtendedMovementMode = EZodiacExtendedMovementMode::Falling;
		break;
	case MOVE_Flying:
		ExtendedMovementMode = EZodiacExtendedMovementMode::BindInAir;
		break;
		
	case MOVE_Custom:
	case MOVE_MAX:
	case MOVE_Swimming:
	case MOVE_None:
	default:
		break;
	}
	
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
	else if (Tag == ZodiacGameplayTags::Status_WeaponReady)
	{
		bIsWeaponReady = bHasTag;
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
		uint8 CustomMovementMode = ZodiacCharMovComp->CustomMovementMode;

		if (MovementMode == MOVE_Walking)
		{
			switch (CustomMovementMode)
			{
			case Move_Custom_Running:
				Gait = Gait_Run;
				return;
			default:
				Gait = Gait_Walk;
				return;
			}
		}
	}
}

void UZodiacHostAnimInstance::UpdateLocationData(float DeltaSeconds)
{
	const FVector PositionDiff = OwningCharacter->GetActorLocation() - WorldLocation;
	DisplacementSinceLastUpdate = UKismetMathLibrary::VSizeXY(PositionDiff);

	WorldLocation = OwningCharacter->GetActorLocation();
	
	DisplacementSpeed = UKismetMathLibrary::SafeDivide(DisplacementSinceLastUpdate, DeltaSeconds);
}
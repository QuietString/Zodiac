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
	if (ZodiacCharMovComp)
	{
		UpdateMovementData();
		UpdateVelocityData();
		UpdateAccelerationData(DeltaSeconds);
	}
}

void UZodiacHostAnimInstance::UpdateMovementData()
{
	CustomMovement_Last = CustomMovement;
	CustomMovement = EZodiacCustomMovementMode(ZodiacCharMovComp->CustomMovementMode);
	bIsADS = (CustomMovement == MOVE_ADS);
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

void UZodiacHostAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bHasTag)
{
	if (Tag == ZodiacGameplayTags::Status_Focus)
	{
		bIsFocus = bHasTag;
	}
}

void UZodiacHostAnimInstance::UpdateGait()
{
	if (ZodiacCharMovComp)
	{
		Gait_LastFrame = Gait;
		
		EMovementMode MovementMode = ZodiacCharMovComp->MovementMode;
		uint8 CustomMovementMode = ZodiacCharMovComp->CustomMovementMode;

		switch (MovementMode)
		{
		case MOVE_Walking:
			switch (CustomMovementMode)
			{
			case MOVE_ADS:
				Gait = Gait_Walk;
				return;
			
			case MOVE_Running:
				Gait = Gait_Run;
				return;
				
			default:
				break;
			}
		default:
			Gait = Gait_Run;
		}
	}
}

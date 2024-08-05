// the.quiet.string@gmail.com


#include "ZodiacHostAnimInstance.h"

#include "Character/ZodiacCharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostAnimInstance)

void UZodiacHostAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
}

void UZodiacHostAnimInstance::NativeInitializeAnimation()
{
	if (AZodiacHostCharacter* PawnOwner = Cast<AZodiacHostCharacter>(TryGetPawnOwner()))
	{
		HostCharacter = PawnOwner;
		HostCharacter->CallOrRegister_OnAbilitySystemInitialized(FOnHostAbilitySystemComponentLoaded::FDelegate::CreateUObject(this, &ThisClass::InitializeWithAbilitySystem));
	}
}

void UZodiacHostAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if (AZodiacHostCharacter* PawnOwner = Cast<AZodiacHostCharacter>(TryGetPawnOwner()))
	{
		HostCharacter = PawnOwner;
		
		if (UZodiacCharacterMovementComponent* CharacterMovement = Cast<UZodiacCharacterMovementComponent>(HostCharacter->GetCharacterMovement()))
		{
			ZodiacCharMovComp = CharacterMovement;	
		}
	}
}

void UZodiacHostAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (ZodiacCharMovComp)
	{
		TEnumAsByte CustomMovementMode = EZodiacCustomMovementMode(ZodiacCharMovComp->CustomMovementMode);
		bIsAiming = (CustomMovementMode == MOVE_Aiming);
	}
}

void UZodiacHostAnimInstance::UpdateVelocityData()
{
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
			case MOVE_Aiming:
				Gait = Gait_Walk;
				return;
			
			case MOVE_Sprinting:
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

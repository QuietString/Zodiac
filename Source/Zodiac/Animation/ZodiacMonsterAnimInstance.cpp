// the.quiet.string@gmail.com


#include "ZodiacMonsterAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UZodiacMonsterAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	
}

UCharacterMovementComponent* UZodiacMonsterAnimInstance::GetMovementComponent()
{
	if (APawn* Pawn = TryGetPawnOwner())
	{
		if (UCharacterMovementComponent* CharacterMovementComponent = Cast<UCharacterMovementComponent>(Pawn->GetMovementComponent()))
		{
			return CharacterMovementComponent;
		}
	}

	return nullptr;
}

void UZodiacMonsterAnimInstance::UpdateLocationData(float DeltaSeconds)
{

}

void UZodiacMonsterAnimInstance::UpdateRotationData(float DeltaSeconds)
{
}

void UZodiacMonsterAnimInstance::UpdateVelocityData()
{
}

void UZodiacMonsterAnimInstance::UpdateAccelerationData()
{
	FVector WorldAcceleration2D = GetMovementComponent()->GetCurrentAcceleration();
	WorldAcceleration2D.Z = 0;

	LocalAcceleration2D = WorldRotation.UnrotateVector(WorldAcceleration2D);

	const float AccelerationSquared = UKismetMathLibrary::VSizeXYSquared(LocalAcceleration2D);
	HasAcceleration = !(UKismetMathLibrary::NearlyEqual_FloatFloat(AccelerationSquared, 0.0, 0.000001));

}

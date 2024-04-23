// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ZodiacMonsterAnimInstance.generated.h"

class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	virtual void NativeThreadSafeUpdateAnimation(float DeltaSeconds) override;

	UCharacterMovementComponent* GetMovementComponent();

protected:

	void UpdateLocationData(float DeltaSeconds);
	void UpdateRotationData(float DeltaSeconds);
	void UpdateVelocityData();
	void UpdateAccelerationData();
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Rotation_Data)
	FRotator WorldRotation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Acceleration_Data)
	FVector LocalAcceleration2D;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool HasAcceleration;
};

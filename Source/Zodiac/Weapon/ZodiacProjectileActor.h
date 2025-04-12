// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacProjectileActor.generated.h"

class UProjectileMovementComponent;

UCLASS(Blueprintable)
class ZODIAC_API AZodiacProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	AZodiacProjectileActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	virtual void Tick(float DeltaTime) override;
	void UpdateHomingState(float DeltaTime);
	bool CheckTargetLost(float DeltaTime);

public:
	UPROPERTY(Replicated, EditInstanceOnly, BlueprintReadWrite, Category = "Homing", meta = (ExposeOnSpawn))
	TObjectPtr<AActor> HomingTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	bool bDisableTickWhenNotHoming = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float MaxHomingAngle = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float AllowedOutOfAngleTime = 0.5f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Homing")
	float OutOfAngleDuration;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
};

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

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Explosion")
	TArray<FHitResult> GetExplosionHitResults(TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility);
	
	UFUNCTION()
	void OnHomingTargetDeathStarted(AActor* OwningActor);
	void StartHoming();
	void StopHoming(bool bUseGravity);
	
	void UpdateHomingState(float DeltaTime);
	bool CheckTargetLost(float DeltaTime);

public:
	/** Radius used when searching for pawns/actors at explosion time. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionRadius = 300.0f;

	// Radius used for actual hit location and normal for each target actor
	// It should be smaller than projectile collision radius/extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float SingleTargetTraceRadius = 20.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing", meta = (ExposeOnSpawn))
	bool bSpawnWithHomingEnabled = false;
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Homing", meta = (ExposeOnSpawn))
	TObjectPtr<AActor> HomingTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	bool bDisableTickWhenNotHoming = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float MaxHomingAngle = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Homing")
	float AllowedOutOfAngleTime = 0.f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Homing")
	float OutOfAngleDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bDisableTickWhenReachTarget = true;
	
	UPROPERTY(Transient, BlueprintReadWrite, Category = "Projectile")
	bool bHasReachedTarget = false;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	bool bIsHoming = false;

	// To ignore when projectile trace
	UPROPERTY(Transient)
	TArray<AActor*> HeroActors;
};

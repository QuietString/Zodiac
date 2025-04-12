// the.quiet.string@gmail.com


#include "ZodiacProjectileActor.h"

#include "Character/ZodiacHealthComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacProjectileActor)

AZodiacProjectileActor::AZodiacProjectileActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovement"));
}

void AZodiacProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HomingTarget);
}

void AZodiacProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	if (!HasAuthority())
	{
		return;
	}

	UpdateHomingState(DeltaTime);
}

void AZodiacProjectileActor::UpdateHomingState(float DeltaTime)
{
	bool bShouldStopHoming = false;
	
	if (!HomingTarget)
	{
		bShouldStopHoming = true;
	}

	if (HomingTarget)
	{
		if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(HomingTarget))
		{
			if (HealthComponent->IsDeadOrDying())
			{
				bShouldStopHoming = true;
			}
		}

		if (CheckTargetLost(DeltaTime))
		{
			bShouldStopHoming = true;
		}
	}

	if (bShouldStopHoming && ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
		ProjectileMovement->ProjectileGravityScale = 1.f;
		
		if (bDisableTickWhenNotHoming)
		{
			SetActorTickEnabled(false);	
		}
	}
}

bool AZodiacProjectileActor::CheckTargetLost(float DeltaTime)
{
	if (ProjectileMovement && ProjectileMovement->bIsHomingProjectile)
	{
		if (ProjectileMovement->HomingTargetComponent.Get())
		{
			FVector ToTarget = (ProjectileMovement->HomingTargetComponent->GetComponentLocation() - GetActorLocation()).GetSafeNormal();
			FVector Forward = GetActorForwardVector();

			float AngleDegrees = FMath::RadiansToDegrees(acosf(FVector::DotProduct(Forward, ToTarget)));

			if (AngleDegrees > MaxHomingAngle)
			{
				OutOfAngleDuration += DeltaTime;

				if (OutOfAngleDuration >= AllowedOutOfAngleTime)
				{
					// Cease homing
					return true;
				}
			}
			else
			{
				// Target is within acceptable angle — reset timer
				OutOfAngleDuration = 0.0f;
			}
		}
	}

	return false;
}
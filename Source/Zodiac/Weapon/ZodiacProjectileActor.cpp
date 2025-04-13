// the.quiet.string@gmail.com


#include "ZodiacProjectileActor.h"

#include "Character/ZodiacHealthComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacProjectileActor)

AZodiacProjectileActor::AZodiacProjectileActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovement"));
}

void AZodiacProjectileActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HomingTarget);
}

void AZodiacProjectileActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		// Simulate only in server.
		if (ProjectileMovement)
		{
			ProjectileMovement->bSimulationEnabled = false;
		}
		
		return;
	}
	
	if (HomingTarget)
	{
		StartHoming();
	}
	else
	{
		// Straight projectile without gravity when spawned with no homing target.
		StopHoming(false);
	}
}

void AZodiacProjectileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	if (bDisableTickWhenReachTarget && bHasReachedTarget)
	{
		StopHoming(false);
		SetActorTickEnabled(false);
		
		return;
	}
	
	UpdateHomingState(DeltaTime);
}

void AZodiacProjectileActor::OnHomingTargetDeathStarted(AActor* OwningActor)
{
	if (HomingTarget)
	{
		if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(HomingTarget))
		{
			HealthComponent->OnDeathStarted.RemoveDynamic(this, &ThisClass::OnHomingTargetDeathStarted);
		}
	}
	
	StopHoming(true);
}

void AZodiacProjectileActor::StartHoming()
{
	bIsHoming = true;

	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		ProjectileMovement->ProjectileGravityScale = 0.f;
	}
		
	if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(HomingTarget))
	{
		HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnHomingTargetDeathStarted);
	}

	SetActorTickEnabled(true);
}

void AZodiacProjectileActor::StopHoming(bool bUseGravity)
{
	if (!bIsHoming)
	{
		return;
	}
	
	HomingTarget = nullptr;
	
	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = false;
		ProjectileMovement->HomingTargetComponent = nullptr;
		ProjectileMovement->ProjectileGravityScale = bUseGravity ? 1.f : 0.f;
	}

	if (bDisableTickWhenNotHoming)
	{
		SetActorTickEnabled(false);	
	}

	bIsHoming = false;
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

	if (bShouldStopHoming)
	{
		StopHoming(true);
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

				if ( (AllowedOutOfAngleTime <= 0.f) || (OutOfAngleDuration >= AllowedOutOfAngleTime) )
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
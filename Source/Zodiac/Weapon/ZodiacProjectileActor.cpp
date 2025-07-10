// the.quiet.string@gmail.com


#include "ZodiacProjectileActor.h"

#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacHostCharacter.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Physics/ZodiacCollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacProjectileActor)

AZodiacProjectileActor::AZodiacProjectileActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	ProjectileMovement = ObjectInitializer.CreateDefaultSubobject<UProjectileMovementComponent>(this, TEXT("ProjectileMovement"));
}

void AZodiacProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	if (AZodiacHostCharacter* Host = Cast<AZodiacHostCharacter>(GetInstigator()))
	{
		for (AZodiacHeroCharacter* Hero : Host->GetHeroes())
		{
			HeroActors.Add(Hero);
		}
	}
	
	if (!HasAuthority())
	{
		// Simulate only in server.
		if (ProjectileMovement)
		{
			ProjectileMovement->bSimulationEnabled = false;
		}
		
		return;
	}
	
	if (bSpawnWithHomingEnabled && HomingTarget)
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

TArray<FHitResult> AZodiacProjectileActor::GetExplosionHitResults(const TEnumAsByte<ECollisionChannel> TraceChannel)
{
	TArray<FHitResult> HitResults;
	
    // Store all actors from the overlap for easy access.
    TArray<AActor*> AllOverlapActors;
    
    const FVector ExplosionCenter = GetActorLocation();

    // 1) Overlap to find actors in the explosion radius.
    TArray<FOverlapResult> OverlapResults;
    const FCollisionShape SphereShape = FCollisionShape::MakeSphere(ExplosionRadius);
    TEnumAsByte<ECollisionChannel> ExplosionChannel = ZODIAC_TRACE_CHANNEL_WEAPON_CAPSULE;
    bool bAnyOverlap = GetWorld()->OverlapMultiByChannel(OverlapResults, ExplosionCenter, FQuat::Identity, ExplosionChannel, SphereShape);
    
    if (bAnyOverlap)
    {
        // Collect all unique actors from the overlap results.
        for (const FOverlapResult& Ovr : OverlapResults)
        {
            AActor* OverlapActor = Ovr.GetActor();
            if (OverlapActor && OverlapActor != this && OverlapActor != GetInstigator())
            {
                AllOverlapActors.AddUnique(OverlapActor);
            }
        }
        
        // 2) Iterate over the overlap results (each candidate).
        for (const FOverlapResult& Ovr : OverlapResults)
        {
            AActor* CandidateActor = Ovr.GetActor();
            UPrimitiveComponent* PrimComp = Ovr.GetComponent();

            // Skip candidate if invalid, self, or already processed (you can use AllOverlapActors or a separate tracking list if needed).
            if (!CandidateActor || CandidateActor == this || !PrimComp)
            {
                continue;
            }
            
            // Get the nearest collision point on the candidate’s collision component.
            FVector ClosestPoint;
            if (PrimComp->GetClosestPointOnCollision(ExplosionCenter, ClosestPoint))
            {
                // 3) Set up the sweep trace to get an actual hit result from that candidate.
                FHitResult SingleHit;
                FCollisionShape SingleSphere = FCollisionShape::MakeSphere(SingleTargetTraceRadius);
                FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ExplosionTrace), false, this);
                TraceParams.bReturnPhysicalMaterial = true;
                
                // Ignore all overlapping actors except the candidate.
                TArray<AActor*> ActorsToIgnore;
                for (AActor* OverlapActor : AllOverlapActors)
                {
                    if (OverlapActor != CandidateActor)
                    {
                        ActorsToIgnore.Add(OverlapActor);
                    }
                }
                TraceParams.AddIgnoredActors(ActorsToIgnore);
            	TraceParams.AddIgnoredActors(HeroActors);

                FVector TraceStart = ExplosionCenter;
                FVector TraceDir = (ClosestPoint - TraceStart).GetSafeNormal();
                FVector TraceEnd = ClosestPoint + TraceDir * ExplosionRadius;
                
                bool bHit = GetWorld()->SweepSingleByChannel(SingleHit, TraceStart, TraceEnd, FQuat::Identity, TraceChannel, SingleSphere, TraceParams);
                
                // Check that the hit came from the candidate.
                if (bHit && SingleHit.bBlockingHit && SingleHit.GetActor() == CandidateActor)
                {
                    HitResults.Add(SingleHit);
                }
                
                //DrawDebugLine(GetWorld(), TraceStart, TraceEnd, bHit ? FColor::Green : FColor::Red, false, 5.0f);
            }
        }
    }
    
    return HitResults;
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
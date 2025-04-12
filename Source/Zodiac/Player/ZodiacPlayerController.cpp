// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "EngineUtils.h"
#include "ZodiacPlayerState.h"
#include "ZodiacCheatManager.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacPlayerCameraManager.h"
#include "Character/ZodiacHealthComponent.h"
#include "Character/ZodiacHeroCharacter2.h"
#include "Character/ZodiacHostCharacter.h"
#include "Development/ZodiacDeveloperSettings.h"
#include "Teams/ZodiacTeamSubsystem.h"
#include "Utility/ZodiacKismetSystemLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPlayerController)

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PlayerCameraManagerClass = AZodiacPlayerCameraManager::StaticClass();

#if USING_CHEAT_MANAGER
	CheatClass = UZodiacCheatManager::StaticClass();
#endif
}

AZodiacPlayerState* AZodiacPlayerController::GetZodiacPlayerState() const
{
	return Cast<AZodiacPlayerState>(PlayerState);
}

UZodiacAbilitySystemComponent* AZodiacPlayerController::GetHeroAbilitySystemComponent()
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetCharacter()))
	{
		return ZodiacCharacter->GetZodiacAbilitySystemComponent();
	}

	return nullptr;
}

AZodiacHostCharacter* AZodiacPlayerController::GetHostCharacter() const
{
	return GetPawn<AZodiacHostCharacter>();
}

FGenericTeamId AZodiacPlayerController::GetGenericTeamId() const
{
	if (AZodiacPlayerState* ZodiacPlayerState = GetZodiacPlayerState())
	{
		return ZodiacPlayerState->GetGenericTeamId();
	}
	
	return FGenericTeamId();
}

void AZodiacPlayerController::ServerCheat_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		ClientMessage(ConsoleCommand(Msg));
	}
#endif  
}

bool AZodiacPlayerController::ServerCheat_Validate(const FString& Msg)
{
	return true;
}

void AZodiacPlayerController::ServerCheatAll_Implementation(const FString& Msg)
{
#if USING_CHEAT_MANAGER
	if (CheatManager)
	{
		UE_LOG(LogZodiac, Warning, TEXT("ServerCheatAll: %s"), *Msg);
		
	}
#endif
}

bool AZodiacPlayerController::ServerCheatAll_Validate(const FString& Msg)
{
	return true;
}

void AZodiacPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

#if WITH_SERVER_CODE && WITH_EDITOR
	if (GIsEditor && (InPawn != nullptr) && (GetPawn() == InPawn))
	{
		for (const FZodiacCheatToRun& CheatRow : GetDefault<UZodiacDeveloperSettings>()->CheatsToRun)
		{
			if (CheatRow.Phase == ECheatExecutionTime::OnPlayerPawnPossession)
			{
				ConsoleCommand(CheatRow.Cheat, true);
			}
		}
	}
#endif
}

void AZodiacPlayerController::BeginPlay()
{
	Super::BeginPlay();

	EnableCheats();
}

void AZodiacPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocalController())
	{
		CheckCrosshairTarget();
	}
}

void AZodiacPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if (AZodiacHostCharacter* HostCharacter = Cast<AZodiacHostCharacter>(GetCharacter()))
	{
		if (IsLocalPlayerController())
		{
			if (UZodiacAbilitySystemComponent* HostASC = HostCharacter->GetHostAbilitySystemComponent())
			{
				HostASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
			
			if (UZodiacAbilitySystemComponent* HeroASC = HostCharacter->GetHeroAbilitySystemComponent())
			{
				HeroASC->ProcessAbilityInput(DeltaTime, bGamePaused);
			}
		}
	}

	// if (AZodiacHeroCharacter2* HeroCharacter2 = Cast<AZodiacHeroCharacter2>(GetCharacter()))
	// {
	// 	if (IsLocalPlayerController())
	// 	{
	// 		if (UZodiacAbilitySystemComponent* HeroASC = HeroCharacter2->GetZodiacAbilitySystemComponent())
	// 		{
	// 			HeroASC->ProcessAbilityInput(DeltaTime, bGamePaused);
	// 		}
	// 	}
	// }
	
	Super::PostProcessInput(DeltaTime, bGamePaused);
}

void AZodiacPlayerController::CheckCrosshairTarget()
{
    bAimingAtEnemy = false;
    CurrentAimTarget = nullptr;

    // 1) Get camera location & forward vector
    FVector CamLoc;
    FRotator CamRot;
    GetPlayerViewPoint(CamLoc, CamRot);

    // Basic trace line: camera forward
    const FVector TraceStart = CamLoc;
    const FVector TraceEnd   = CamLoc + (CamRot.Vector() * TraceDistance);

    // 2) Compute the projection param for your Pawn's location
    APawn* MyPawn = GetPawn();
    FVector OwnerLoc = MyPawn ? MyPawn->GetActorLocation() : TraceStart; // fallback

    FVector Dir = TraceEnd - TraceStart;
    float DotDirDir = Dir | Dir; // same as Dir.SizeSquared()

    float t = 0.f; // we’ll store the projection parameter
    if (DotDirDir > KINDA_SMALL_NUMBER)
    {
        // (OwnerLoc - TraceStart) dot Dir / (Dir dot Dir)
        t = ((OwnerLoc - TraceStart) | Dir) / DotDirDir;

        // Optionally clamp t so the start can’t go beyond the original line segment
        t = FMath::Clamp(t, 0.f, 1.f);
    }

    // 3) The new start is the projection from OwnerLoc onto that line
    FVector AdjustedStart = TraceStart + (t * Dir);

    // 4) Do the sphere sweep from AdjustedStart to TraceEnd
    FCollisionQueryParams Params(SCENE_QUERY_STAT(CrosshairTrace), false, MyPawn);

    FHitResult Hit;
    GetWorld()->SweepSingleByChannel(
        Hit,
        AdjustedStart,
        TraceEnd,
        FQuat::Identity,
        TraceChannel,
        FCollisionShape::MakeSphere(TraceSphereRadius),
        Params
    );

    if (Hit.bBlockingHit)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            CurrentAimTarget = HitActor;

            // Check if it's an enemy
            if (UZodiacTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UZodiacTeamSubsystem>())
            {
                EZodiacTeamComparison CompareResult = TeamSubsystem->CompareTeams(this, HitActor);
                if (CompareResult == EZodiacTeamComparison::DifferentTeams)
                {
                    bAimingAtEnemy = true;
                }
            }
        }
    }
}

void AZodiacPlayerController::GetNearActorsFromAimCenter(TSubclassOf<AActor> ActorClass, TArray<AActor*>& OutActors, float MaxAngle, float MaxRange)
{
	APawn* OwningPawn = GetPawn();
	if (!OwningPawn || !ActorClass)
	{
		return;
	}

	UWorld* World = GetWorld();
	check(World);

	OutActors.Reset();
	
    // Get camera location & forward
    FVector CameraLoc;
    FRotator CameraRot;
    GetPlayerViewPoint(CameraLoc, CameraRot);

    // Collect potential targets
    TArray<AActor*> PotentialTargets;
	
    // Find all Pawns in the world.
    for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
    {
        AActor* OtherActor = *It;
    	
        if (OtherActor == OwningPawn) continue; // Skip self
    	if (OtherActor->IsHidden()) continue; // Skip hidden
    	
        if (UZodiacHealthComponent* HealthComponent = UZodiacHealthComponent::FindHealthComponent(OtherActor))
        {
	        if (HealthComponent->IsDeadOrDying())
	        {
	        	// Skip dead actors
		        continue;
	        }
        }

    	UZodiacTeamSubsystem* TeamSubsystem = GetWorld()->GetSubsystem<UZodiacTeamSubsystem>();
    	check(TeamSubsystem);
    	
    	if (!TeamSubsystem->CanCauseDamage(OwningPawn, OtherActor, false))
    	{
    		// Skip friendly actors
    		continue;
    	}
    	
        FVector ToTarget = OtherActor->GetActorLocation() - CameraLoc;
        float DistSqr = ToTarget.SizeSquared();
        if (DistSqr > FMath::Square(MaxRange))
        {
            continue; // Too far
        }

        // Angle from camera forward (dot product approach)
        float Dot = FVector::DotProduct(CameraRot.Vector().GetSafeNormal(), ToTarget.GetSafeNormal());
        float AngleDeg = FMath::RadiansToDegrees(acosf(Dot));
        if (AngleDeg <= MaxAngle)
        {
            PotentialTargets.Add(OtherActor);
        }
    }

    // Sort potential targets by distance
    PotentialTargets.Sort([CameraLoc](AActor& A, AActor& B)
    {
        float DistA = (A.GetActorLocation() - CameraLoc).SizeSquared();
        float DistB = (B.GetActorLocation() - CameraLoc).SizeSquared();
        return DistA < DistB; // ascending
    });

    OutActors = PotentialTargets;
}

// the.quiet.string@gmail.com


#include "ZodiacMonster.h"

#include "BrainComponent.h"
#include "ZodiacAIController.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Animation/ZodiacZombieAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "System/ZodiacGameData.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"
#include "ZodiacHitReactSimulationComponent.h"
#include "ZodiacPreMovementComponentTickComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/DecalComponent.h"
#include "Utility/ZodiacUtilityLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacMonster)

AZodiacMonster::AZodiacMonster(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bStartWithTickEnabled = false;
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	GetMesh()->PrimaryComponentTick.bStartWithTickEnabled = false;
	GetMesh()->PrimaryComponentTick.bAllowTickOnDedicatedServer = true;
	
	RetargetedMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("RetargetedMesh"));
	RetargetedMeshComponent->SetupAttachment(GetMesh());
	RetargetedMeshComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	RetargetedMeshComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;

	PhysicalAnimationComponent = ObjectInitializer.CreateDefaultSubobject<UPhysicalAnimationComponent>(this, TEXT("PhysicalAnimationComponent"));
	PhysicalAnimationComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	PhysicalAnimationComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	
	HitReactSimulationComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHitReactSimulationComponent>(this, TEXT("HitReactSimulationComponent"));
	HitReactSimulationComponent->PrimaryComponentTick.bStartWithTickEnabled = false;
	HitReactSimulationComponent->PrimaryComponentTick.bAllowTickOnDedicatedServer = false;
	
	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));

	GetPreMovementComponentTick()->PrimaryComponentTick.bStartWithTickEnabled = false;
	
	GetCharacterMovement()->PrimaryComponentTick.bStartWithTickEnabled = false;
	GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;
}

UZodiacAbilitySystemComponent* AZodiacMonster::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AZodiacMonster::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UZodiacHealthComponent* AZodiacMonster::GetHealthComponent() const
{
	return HealthComponent;
}

FGenericTeamId AZodiacMonster::GetGenericTeamId() const
{
	if (AZodiacAIController* ZodiacAC = GetController<AZodiacAIController>())
	{
		return ZodiacAC->GetGenericTeamId();
	}
	
	return static_cast<uint8>(EZodiacTeam::NoTeam);
}

void AZodiacMonster::OnPhysicsTagChanged(FGameplayTag Tag, int Count)
{
	Multicast_OnPhysicsTagChanged(Tag, Count);
}

UAbilitySystemComponent* AZodiacMonster::GetTraversalAbilitySystemComponent() const
{
	return GetAbilitySystemComponent();
}

void AZodiacMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	Sleep();
}

void AZodiacMonster::BeginPlay()
{
	Super::BeginPlay();
	
	if (CharacterData && !bHasMovementInitialized)
	{
		if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
		{
			FZodiacExtendedMovementConfig MovementConfig = CharacterData->ExtendedMovementConfig;
			ZodiacCharMovComp->SetExtendedMovementConfig(MovementConfig);

			if (MovementConfig.DefaultExtendedMovement == EZodiacExtendedMovementMode::Walking)
			{
				if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
				{
					if (UZodiacZombieAnimInstance* ZombieAnimInstance = Cast<UZodiacZombieAnimInstance>(AnimInstance))
					{
						FVector WalkSpeedRange = *MovementConfig.MovementSpeedsMap.Find(EZodiacExtendedMovementMode::Walking);
						ZombieAnimInstance->WalkSpeed = WalkSpeedRange.X;
						ZombieAnimInstance->MovementSpeedMultiplier = 1.f;
						ZombieAnimInstance->SelectAnimsBySeed(0);
					}
				}
			}
			
			bHasMovementInitialized = true;
		}
	}
}

void AZodiacMonster::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	InitializeAbilitySystem(AbilitySystemComponent, this);
}

void AZodiacMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, SpawnSeed, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(ThisClass, SpawnConfig, COND_InitialOnly);
}

void AZodiacMonster::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner)
{
	Super::InitializeAbilitySystem(InASC, InOwner);
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacMonster::SetSpawnSeed(const int32 Seed)
{
	SpawnSeed = Seed;
	
	if (HasAuthority())
	{
		OnSpawnSeedSet_Internal();
	}
}


void AZodiacMonster::SetSpawnConfig(const FZodiacZombieSpawnConfig& InSpawnConfig)
{
	if (HasAuthority())
	{
		SpawnConfig = InSpawnConfig;
		OnRep_SpawnConfig();
	}
}

void AZodiacMonster::Sleep()
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorLocation(FVector::ZeroVector, false, nullptr, ETeleportType::ResetPhysics);

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName(TEXT("ZodiacPawnCapsule"));
	}

	if (HasAuthority())
	{
		if (GetPreMovementComponentTick())
		{
			GetPreMovementComponentTick()->SetComponentTickEnabled(false);
		}
	}
	
	if (UMovementComponent* MovementComponent = GetMovementComponent())
	{
		MovementComponent->SetComponentTickEnabled(false);
	}
	
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetComponentTickEnabled(false);
	}
		
	if (RetargetedMeshComponent)
	{
		RetargetedMeshComponent->SetComponentTickEnabled(false);
	}
	
	if (HealthComponent)
	{
		HealthComponent->ResetHealthAndDeathState();
	}

	TArray<UDecalComponent*> DecalComps;
	GetComponents<UDecalComponent>(DecalComps);

	// Remove all blood decals.
	for (UDecalComponent* Decal : DecalComps)
	{
		FName BloodDecalTag = UZodiacUtilityLibrary::GetBloodDecalTag();
		if (!Decal->ComponentHasTag(BloodDecalTag)) continue;
		Decal->DestroyComponent();
	}

	if (HasAuthority())
	{
		if (AAIController* AC = GetController<AAIController>())
		{
			AC->SetActorTickEnabled(false);
		
			if (UBrainComponent* BrainComponent = AC->GetBrainComponent())
			{
				FString StopReason = TEXT("Sleep By AZodiacMonster");
				BrainComponent->StopLogic(StopReason);
			}
		}
		
		SetNetDormancy(DORM_DormantAll);
	}
	
	OnSleep.Broadcast();
}

void AZodiacMonster::WakeUp(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	if (HasAuthority())
	{
		SetNetDormancy(DORM_Awake);
	}
	
	FVector ActorScale = GetActorScale();
	FTransform SpawnTransform = FTransform(SpawnRotation, SpawnLocation, ActorScale);
	SetActorTransform(SpawnTransform, false, nullptr, ETeleportType::ResetPhysics);
	
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (HasAuthority())
	{
		if (GetPreMovementComponentTick())
		{
			GetPreMovementComponentTick()->SetComponentTickEnabled(true);
		}
	}
	
	if (UMovementComponent* MovementComponent = GetMovementComponent())
	{
		MovementComponent->SetComponentTickEnabled(true);
	}
	
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->ResetAnimInstanceDynamics();
		MeshComponent->InitAnim(true);
		MeshComponent->SetComponentTickEnabled(true);
	}
	
	if (GetNetMode() != NM_DedicatedServer)
	{
		if (RetargetedMeshComponent)
		{
			RetargetedMeshComponent->ResetAnimInstanceDynamics();
			RetargetedMeshComponent->ResetAllBodiesSimulatePhysics();
			RetargetedMeshComponent->SetComponentTickEnabled(true);
		}	
	}
	
	if (HasAuthority())
	{
		if (AAIController* AC = GetController<AAIController>())
		{
			AC->SetActorTickEnabled(true);
			AC->RunBehaviorTree(BehaviorTree);
		
			if (UBrainComponent* BrainComponent = AC->GetBrainComponent())
			{
				BrainComponent->RestartLogic();
			}

			if (UBlackboardComponent* BlackBoard = AC->GetBlackboardComponent())
			{
				BlackBoard->SetValueAsBool(FName("UseTargetSearchRadius"), SpawnConfig.bUseTargetSearchRadius);
				BlackBoard->SetValueAsFloat(FName("SearchRadius"), SpawnConfig.TargetSearchRadius);
				BlackBoard->SetValueAsFloat(FName("WaitTime"), SpawnConfig.WaitTimeAfterSpawn);
				BlackBoard->SetValueAsFloat(FName("WaitTimeRandomDeviation"), SpawnConfig.WaitTimeRandomDeviation);
				BlackBoard->SetValueAsBool(FName("CanSwitchExtendedMovementMode"), SpawnConfig.bAllowSwitchingExtendedMovementMode);

				if (UZodiacCharacterMovementComponent* ZodiacCharacterMovementComponent = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
				{
					EZodiacExtendedMovementMode ExtendedMovementMode = ZodiacCharacterMovementComponent->GetExtendedMovementMode();
					BlackBoard->SetValueAsEnum(FName("ExtendedMovementMode"), static_cast<uint8>(ExtendedMovementMode));
				}
			}
		}
		
		ForceNetUpdate();
	}

	OnWakeUp.Broadcast();
}

void AZodiacMonster::OnSpawnSeedSet_Internal()
{
	check(CharacterData);
	
	float MovementSpeedMultiplier = FMath::GetMappedRangeValueClamped(FVector2d(0, 255), FVector2d(0.9f, 1.1f), SpawnSeed);

	const UZodiacGameData& GameData = UZodiacGameData::Get();
	
	TArray<FZodiacExtendedMovementConfig> MovementConfigs = GameData.MovementConfigTemplates;
	uint8 Num = MovementConfigs.Num();
	if (Num > 0)
	{
		FZodiacExtendedMovementConfig MovementConfig = MovementConfigs[SpawnSeed % Num];
	
		if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
		{
			for (auto& [K, V] : MovementConfig.MovementSpeedsMap)
			{
				V = V * MovementSpeedMultiplier;
			}
		
			// Change movement speed
			ZodiacCharMovComp->SetExtendedMovementConfig(MovementConfig);
			bHasMovementInitialized = true;

			if (FVector* WalkSpeeds = MovementConfig.MovementSpeedsMap.Find(EZodiacExtendedMovementMode::Walking))
			{
				if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
				{
					if (UZodiacZombieAnimInstance* ZombieAnimInstance = Cast<UZodiacZombieAnimInstance>(AnimInstance))
					{
						ZombieAnimInstance->WalkSpeed = WalkSpeeds->X;
						ZombieAnimInstance->MovementSpeedMultiplier = MovementSpeedMultiplier;
						ZombieAnimInstance->SelectAnimsBySeed(SpawnSeed);
					}
				}
			}
		}
	}
}

void AZodiacMonster::OnSpawnConfigSet()
{
	check(CharacterData);

	UWorld*	World = GetWorld();
	check(World);
	
	const UZodiacGameData& GameData = UZodiacGameData::Get();
	check (&GameData);

	if (HasAuthority() && SpawnConfig.BehaviorTree.Get())
	{
		BehaviorTree = SpawnConfig.BehaviorTree;
	}
	
	TArray<FZodiacExtendedMovementConfig> MovementConfigs = GameData.MovementConfigTemplates;
	int32 Index = SpawnConfig.MovementConfigTemplateIndex;
	check(MovementConfigs.IsValidIndex(Index));
	
	FZodiacExtendedMovementConfig MovementConfig = MovementConfigs[Index];
	MovementConfig.DefaultExtendedMovement = SpawnConfig.DefaultMovementMode;
	int16 Seed = SpawnConfig.Seed;

	float MovementSpeedMultiplier = FMath::GetMappedRangeValueClamped(FVector2d(0, 255), FVector2d(0.9f, 1.1f), Seed);

	if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		for (auto& [K, V] : MovementConfig.MovementSpeedsMap)
		{
			V *= MovementSpeedMultiplier;

			if (K == EZodiacExtendedMovementMode::Sprinting)
			{
				float ClampedSpeed = FMath::Clamp(V.X, 630.f, 680.f);
				V = FVector(ClampedSpeed, ClampedSpeed, ClampedSpeed);
			}
		}
		
		// Randomize movement speed
		ZodiacCharMovComp->SetExtendedMovementConfig(MovementConfig);
		bHasMovementInitialized = true;

		if (FVector* WalkSpeeds = MovementConfig.MovementSpeedsMap.Find(EZodiacExtendedMovementMode::Walking))
		{
			if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
			{
				if (UZodiacZombieAnimInstance* ZombieAnimInstance = Cast<UZodiacZombieAnimInstance>(AnimInstance))
				{
					ZombieAnimInstance->WalkSpeed = WalkSpeeds->X;
					ZombieAnimInstance->MovementSpeedMultiplier = MovementSpeedMultiplier;
					
					if (!UKismetSystemLibrary::IsDedicatedServer(World))
					{
						ZombieAnimInstance->SelectAnimsBySeed(Seed);	
					}
				}
			}
		}

		// Randomize jump velocity
		float JumpVelocityMultiplier = FMath::GetMappedRangeValueClamped(FVector2d(0, 150), FVector2d(0.75f, 1.f), Seed);
		ZodiacCharMovComp->JumpZVelocity *= JumpVelocityMultiplier;
	}
}

void AZodiacMonster::OnRep_SpawnSeed()
{
	OnSpawnSeedSet_Internal();
}

void AZodiacMonster::OnRep_SpawnConfig()
{
	 OnSpawnConfigSet();
}

void AZodiacMonster::Multicast_OnPhysicsTagChanged_Implementation(FGameplayTag Tag, int Count)
{
	AZodiacCharacter::OnPhysicsTagChanged(Tag, Count);
}

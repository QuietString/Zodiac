﻿// the.quiet.string@gmail.com


#include "ZodiacHitReactSimulationComponent.h"

#include "ZodiacCharacter.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHealthComponent.h"
#include "Components/CapsuleComponent.h"
#include "PhysicsEngine/PhysicalAnimationComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHitReactSimulationComponent)

namespace ZodiacCollisionProfileName
{
	FName HeroMesh = FName(TEXT("ZodiacHeroMesh"));
	FName Dying = FName(TEXT("ZodiacPawnCapsuleDying"));
	FName Ragdoll = FName(TEXT("Ragdoll"));
}

UZodiacHitReactSimulationComponent::UZodiacHitReactSimulationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
	bWantsInitializeComponent = true;
}

void UZodiacHitReactSimulationComponent::InitializeComponent()
{
	Super::InitializeComponent();

	AZodiacCharacter* OwnerCharacter = GetOwner<AZodiacCharacter>();
	check(OwnerCharacter);
	
	if (bEnableHitReact_UpperBody)
	{
		FZodiacPhysicalHitReactBody HitReactBody(EZodiacPhysicalHitReactBodyType::UpperBody, UpperBodyRoot);
		TargetBodies.Add(HitReactBody);
	}
	
	if (bEnableHitReact_LowerBody)
	{
		FZodiacPhysicalHitReactBody HitReactBody_LeftLeg(EZodiacPhysicalHitReactBodyType::LeftLeg, LeftLegRoot);
		TargetBodies.Add(HitReactBody_LeftLeg);
	
		FZodiacPhysicalHitReactBody HitReactBody_RightLeg(EZodiacPhysicalHitReactBodyType::RightLeg, RightLegRoot);
		TargetBodies.Add(HitReactBody_RightLeg);
	}
	
	if (UPhysicalAnimationComponent* PhysAnimComp = OwnerCharacter->FindComponentByClass<UPhysicalAnimationComponent>())
	{
		PhysicalAnimationComponent = PhysAnimComp;
		ensureMsgf(PhysicalAnimationComponent, TEXT("Hit React simulation component needs Physical Animation Component from owner actor"));
	}

	if (USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetRetargetedMesh())
	{
		TargetMeshComponent = SkeletalMeshComponent;
		OriginalRelativeScale = TargetMeshComponent->GetRelativeScale3D();
		ensureMsgf(TargetMeshComponent, TEXT("Hit React Simulation Component needs Skeletal Mesh Component from owner actor"));
	}

	if (UZodiacHealthComponent* HealthComponent = GetOwner()->FindComponentByClass<UZodiacHealthComponent>())
	{
		HealthComponent->OnDeathStarted.AddDynamic(this, &ThisClass::OnDeathStarted);
		ensureMsgf(HealthComponent, TEXT("Hit React Simulation Component needs Health Component from owner actor"));
	}
	
	OwnerCharacter->OnSimulateOrPlayHitReact.BindUObject(this, &ThisClass::OnPlayHitReact);
	OwnerCharacter->OnWakeUp.AddDynamic(this, &ThisClass::OnWakeUp);
}

void UZodiacHitReactSimulationComponent::ResetPhysicsSetup()
{
	if (AActor* Owner = GetOwner())
	{
		// Resume mesh animations.
		TArray<USkeletalMeshComponent*> Components;
		Owner->GetComponents(USkeletalMeshComponent::StaticClass(), Components);
		for (auto& Mesh : Components)
		{
			Mesh->bPauseAnims = false;
		}	
	}
		
	if (TargetMeshComponent)
	{
		TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
		TargetMeshComponent->SetCollisionProfileName(ZodiacCollisionProfileName::HeroMesh);
		TargetMeshComponent->SetEnableGravity(false);
		TargetMeshComponent->SetAllBodiesSimulatePhysics(false);
		TargetMeshComponent->SetAllBodiesPhysicsBlendWeight(0.f);
		TargetMeshComponent->ResetRelativeTransform();
		TargetMeshComponent->SetRelativeScale3D(OriginalRelativeScale);

		if (PhysicalAnimationComponent)
		{
			PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(Root, NAME_None, true, true);

			PhysicalAnimationComponent->SetSkeletalMeshComponent(TargetMeshComponent);
			PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(UpperBodyRoot, UpperBodyProfile);
			PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(LeftLegRoot, LowerBodyProfile);
			PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(RightLegRoot, LowerBodyProfile);
		}
	}

	bHasRagdollStarted = false;
}

void UZodiacHitReactSimulationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetNetMode() == NM_DedicatedServer)
	{
		SetComponentTickEnabled(false);
	}
	else
	{
		ResetPhysicsSetup();
	}
}

void UZodiacHitReactSimulationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (TargetMeshComponent)
	{
		bool bHasAnySimulatedBody = false;
		for (FZodiacPhysicalHitReactBody& TargetBody : TargetBodies)
		{
			if (TargetBody.BlendWeight > 0.f)
			{
				bHasAnySimulatedBody = true;

				float Duration = TargetBody.LastHit.bIsExplosive ? HitReactDuration_Explosive : HitReactDuration;
				float BlendWeightDiminishSpeed = 1 / Duration;
				float BlendWeightDelta = - (DeltaTime * BlendWeightDiminishSpeed);
				TargetMeshComponent->AccumulateAllBodiesBelowPhysicsBlendWeight(TargetBody.SimulationRootBone, BlendWeightDelta);
				
				// Record current blend weight
				TargetBody.BlendWeight = FMath::Min(TargetBody.BlendWeight + BlendWeightDelta, 1.f);
			}
			else if (TargetBody.bIsSimulated)
			{
				// Stop simulation when the blend weight is zero.
				TargetMeshComponent->SetAllBodiesBelowSimulatePhysics(TargetBody.SimulationRootBone, false);

				// Recover normal physical profile from explosive profile.
				if (TargetBody.LastHit.bIsExplosive)
				{
					PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(UpperBodyRoot, UpperBodyProfile, true, true);	
				}
				TargetBody.LastHit = FZodiacHitDamageData();
				TargetBody.bIsSimulated = false;
			}
		}

		// Stop ticking when no simulated bodies exist.
		if (!bHasAnySimulatedBody)
		{
			SetComponentTickEnabled(false);
			PhysicalAnimationComponent->SetComponentTickEnabled(false);
		}
	}
}

void UZodiacHitReactSimulationComponent::OnWakeUp()
{
	ResetPhysicsSetup();
}

EZodiacPhysicalHitReactBodyType UZodiacHitReactSimulationComponent::DetermineBodyType(FName HitBone) const
{
	if (!TargetMeshComponent || HitBone == NAME_None)
	{
		return EZodiacPhysicalHitReactBodyType::NoHitReactBody;
	}
	
	FName CurrentBone = HitBone;

	while (TargetMeshComponent->BoneIsChildOf(CurrentBone, Root))
	{
		if (CurrentBone == UpperBodyRoot)
		{
			return  EZodiacPhysicalHitReactBodyType::UpperBody;
		}
		if (CurrentBone == LeftLegRoot)
		{
			return EZodiacPhysicalHitReactBodyType::LeftLeg;
		}
		if (CurrentBone == RightLegRoot)
		{
			return EZodiacPhysicalHitReactBodyType::RightLeg;
		}
		if (CurrentBone == Root)
		{
			return EZodiacPhysicalHitReactBodyType::NoHitReactBody;
		}

		CurrentBone = TargetMeshComponent->GetParentBone(CurrentBone);
	}

	return EZodiacPhysicalHitReactBodyType::NoHitReactBody;
}

void UZodiacHitReactSimulationComponent::OnPlayHitReact(FVector HitDirection, FName HitBone, float Magnitude,
                                                         const FGameplayTagContainer& InstigatorTags)
{
	UWorld* World = GetWorld();
	check(World);

	// To update physics blend weights
	PhysicalAnimationComponent->SetComponentTickEnabled(true);
	SetComponentTickEnabled(true);
	
	FZodiacHitDamageData HitDamageData;
	
	EZodiacPhysicalHitReactBodyType HitBodyType = DetermineBodyType(HitBone);
	FZodiacPhysicalHitReactBody* TargetBody = TargetBodies.FindByKey(HitBodyType);

	bool bIsExplosive = InstigatorTags.HasTag(ZodiacGameplayTags::Effect_Type_Damage_Explosive);
	if (bIsExplosive)
	{
		TargetBody = TargetBodies.FindByKey(EZodiacPhysicalHitReactBodyType::UpperBody);
		HitBone = UpperBodyRoot;
		
		PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(UpperBodyRoot, NAME_None, true, true);
		TargetMeshComponent->SetEnableGravity(true);
	}

	if (!TargetBody)
	{
		return;
	}
	
	HitDamageData.bIsExplosive = bIsExplosive;
	HitDamageData.HitTime = World->GetTimeSeconds();
	HitDamageData.Impulse = HitDirection * Magnitude;
	HitDamageData.HitBone = HitBone;

	TargetBody->LastHit = HitDamageData;
	TargetBody->BlendWeight = (TargetBody->BodyType == EZodiacPhysicalHitReactBodyType::UpperBody) ? bIsExplosive ? 0.7f : Magnitude > 40.f ? 1.f : 0.6f : 0.3f;
	TargetBody->bIsSimulated = true;
	
	TargetMeshComponent->SetAllBodiesBelowSimulatePhysics(TargetBody->SimulationRootBone, true);
	TargetMeshComponent->SetAllBodiesBelowPhysicsBlendWeight(TargetBody->SimulationRootBone, TargetBody->BlendWeight);
	TargetMeshComponent->AddImpulse(HitDamageData.Impulse * HitReactStrength, HitBone, true);
}

void UZodiacHitReactSimulationComponent::OnDeathStarted(AActor* OwningActor)
{
	// Stop updating simulation blend weights from TickComponent()	
	SetComponentTickEnabled(false);

	if (UCapsuleComponent* OwnerCapsule = OwningActor->FindComponentByClass<UCapsuleComponent>())
	{
		OwnerCapsule->SetCollisionProfileName(ZodiacCollisionProfileName::Dying);
	}

	TargetMeshComponent->bPauseAnims = true;


	if (GetNetMode() == NM_DedicatedServer)
	{
		TargetMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		if (!bHasRagdollStarted)
		{
			StartRagdoll();
			bHasRagdollStarted = true;
		}
	}
}

void UZodiacHitReactSimulationComponent::StartRagdoll()
{
	if (AActor* Owner = GetOwner())
	{
		// Stop mesh animations.
		TArray<USkeletalMeshComponent*> Components;
		Owner->GetComponents(USkeletalMeshComponent::StaticClass(), Components);
		for (auto& Mesh : Components)
		{
			Mesh->bPauseAnims = true;
		}

		// Clear hit react profile.
		PhysicalAnimationComponent->ApplyPhysicalAnimationProfileBelow(Root, NAME_None, true, true);
		TargetMeshComponent->SetEnableGravity(true);
		TargetMeshComponent->SetCollisionProfileName(ZodiacCollisionProfileName::Ragdoll);
		TargetMeshComponent->SetAllBodiesSimulatePhysics(true);
		TargetMeshComponent->SetAllBodiesPhysicsBlendWeight(1.f);

		FZodiacHitDamageData LastHit;
		
		for (auto& Body : TargetBodies)
		{
			if (LastHit.HitTime < Body.LastHit.HitTime)
			{
				LastHit = Body.LastHit;
			}
		}
		
		float ImpulseMultiplier = LastHit.bIsExplosive ? RagdollStrength_Explosive : RagdollStrength;
		FVector Impulse = LastHit.Impulse * ImpulseMultiplier;
		
		// For non-explosive damage, impulse is applied twice for a same damage. First one from PlayHitReact(), second one from this,
		// which is okay since death hit react should be stronger.
		TargetMeshComponent->AddImpulse(Impulse, LastHit.HitBone, true);
	}
}
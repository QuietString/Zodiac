// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ZodiacHitReactSimulationComponent.generated.h"

class UZodiacHitReactSimulationComponent;

UENUM(BlueprintType)
enum class EZodiacPhysicalHitReactBodyType : uint8
{
	NoHitReactBody,
	UpperBody,
	LeftLeg,
	RightLeg
};

struct FZodiacHitDamageData
{
	FZodiacHitDamageData()
	{
		HitBone = NAME_None;
		Impulse = FVector();
		bIsExplosive = false;
	}
	
	FName HitBone;
	FVector Impulse;
	bool bIsExplosive;
	float HitTime;
};

struct FZodiacPhysicalHitReactBody
{
	FZodiacPhysicalHitReactBody(){}
	
	FZodiacPhysicalHitReactBody(EZodiacPhysicalHitReactBodyType InBodyType, FName InSimulationRootBone)
		: BodyType(InBodyType)
		, SimulationRootBone(InSimulationRootBone)
		{}
	
	EZodiacPhysicalHitReactBodyType BodyType = EZodiacPhysicalHitReactBodyType::NoHitReactBody;
	
	FName SimulationRootBone;

	float BlendWeight = 0.f;
	
	bool bIsSimulated = false;

	FZodiacHitDamageData LastHit;
	
	FORCEINLINE bool operator==(const FZodiacPhysicalHitReactBody& Other) const
	{
		return (this->BodyType == Other.BodyType);
	}
	FORCEINLINE bool operator!=(const FZodiacPhysicalHitReactBody& Other) const
	{
		return !(*this == Other);
	}

	friend UZodiacHitReactSimulationComponent;
};

FORCEINLINE bool operator==(const FZodiacPhysicalHitReactBody& LHS, EZodiacPhysicalHitReactBodyType RHS)
{
	return (LHS.BodyType == RHS);
}

class UPhysicalAnimationComponent;

UCLASS(meta=(BlueprintSpawnableComponent))
class ZODIAC_API UZodiacHitReactSimulationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZodiacHitReactSimulationComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void InitializeComponent() override;
	void ResetPhysicsSetup();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION()
	void OnWakeUp();
	
	EZodiacPhysicalHitReactBodyType DetermineBodyType(FName HitBone) const;
	
	void OnPlayHitReact(FVector HitDirection, FName HitBone, float Magnitude, const FGameplayTagContainer& InstigatorTags);

	UFUNCTION()
	void OnDeathStarted(AActor* OwningActor);
	
	void StartRagdoll();

protected:
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|Root")
	FName Pelvis = FName("Pelvis");
	
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|Root")
	FName Root = FName("root");

	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|UpperBody")
	bool bEnableHitReact_UpperBody = true;
	
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|UpperBody")
	FName UpperBodyRoot = FName("spine_01");

	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|UpperBody")
	FName UpperBodyProfile = FName("HitReact");

	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|LowerBody")
	bool bEnableHitReact_LowerBody = true;
	
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|LowerBody")
	FName LeftLegRoot = FName("thigh_l");
	
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|LowerBody")
	FName RightLegRoot = FName("thigh_r");
	
	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|LowerBody")
	FName LowerBodyProfile = FName("HitReact_LowerBody");

	UPROPERTY(EditAnywhere, Category = "HitReact")
	float HitReactDuration = 0.34f;
	
	UPROPERTY(EditAnywhere, Category = "HitReact")
	float HitReactDuration_Explosive = 2.f;
	
	UPROPERTY(EditAnywhere, Category = "HitReact")
	float HitReactStrength = 40.f;

	UPROPERTY(EditAnywhere, Category = "HitReact|Ragdoll")
	float RagdollStrength = 65.f;

	UPROPERTY(EditAnywhere, Category = "HitReact|Ragdoll")
	float RagdollStrength_Explosive = 150.f;
	
private:	
	TArray<FZodiacPhysicalHitReactBody> TargetBodies;
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> TargetMeshComponent;
	
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

	UPROPERTY(Transient)
	FVector OriginalRelativeScale;
};

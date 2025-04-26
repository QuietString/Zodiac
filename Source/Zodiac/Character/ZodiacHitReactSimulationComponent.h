// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ZodiacHitReactSimulationComponent.generated.h"

class UZodiacHitReactSimulationComponent;

USTRUCT(BlueprintType)
struct FZodiacHitReactDamageConfig
{
	GENERATED_BODY()

	FZodiacHitReactDamageConfig()
		: BlendDuration(0.34f)
		, BlendWeightRange(FVector2D(0.f, 1.f))
		, DamageRange(10.f, 50.f)
		, ImpulseStrength(40.f)
		, RagdollStrength(65.f)
	{}

	// Duration for physical hit react simulation blending. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BlendDuration;
	
	// Initial blend weight range for the simulation. 
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D BlendWeightRange;

	// Damage range to scale initial blend weight.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D DamageRange;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ImpulseStrength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RagdollStrength;

	float GetScaledBlendWeightByDamage(const float Damage) const
	{
		return FMath::GetMappedRangeValueClamped(DamageRange, BlendWeightRange, Damage);
	}
};

UENUM(BlueprintType, DisplayName = "EHitReactBlendMode")
enum class EZodiacHitReactBlendMode : uint8
{
	Linear      UMETA(DisplayName="Linear"),
	EaseIn      UMETA(DisplayName="Ease‑In"),
	EaseOut     UMETA(DisplayName="Ease‑Out"),
	EaseInOut   UMETA(DisplayName="Ease‑In‑Out")
};

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
	{}

	FGameplayTag DamageType;
	float HitTime;
	FName HitBone;
	FVector BaseImpulse;
	float BlendDuration;

	float HitReactImpulseMultiplier;
	float RagdollImpulseMultiplier;
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
	float InitialBlendWeight = 1.f;
	
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

	FGameplayTag GetDamageType(const FGameplayTagContainer& Tags) const;
	const FZodiacHitReactDamageConfig& GetDamageHitReactConfig(const FGameplayTag DamageType) const;
	
	// Return the first “branch root” (clavicle_l, clavicle_r, spine_01, thigh_l …) that
	// is an ancestor of HitBone.  Used to restrict physics to the correct limb only.
	// Currently not used.  
	FName FindSimulationRootForHit(FName HitBone) const;
	
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
	FName LeftClavicle = FName("clavicle_l");

	UPROPERTY(EditAnywhere, Category = "HitReact|MeshConfig|UpperBody")
	FName RightClavicle = FName("clavicle_r");
	
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

	// Blend mode for diminishing physics weight for restoring original body pose after hit.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReact|Blend")
	EZodiacHitReactBlendMode BlendMode = EZodiacHitReactBlendMode::Linear;

	// Exponent for the Ease curves
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HitReact|Blend", meta=(ClampMin="1.0"))
	float BlendEaseExponent = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HitReact", meta=(Categories="Effect.Type.Damage"))
	TMap<FGameplayTag, FZodiacHitReactDamageConfig> BlendConfigs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HitReact")
	FZodiacHitReactDamageConfig DefaultBlendConfig = FZodiacHitReactDamageConfig();

private:	
	TArray<FZodiacPhysicalHitReactBody> TargetBodies;
	
	UPROPERTY()
	TObjectPtr<USkeletalMeshComponent> TargetMeshComponent;
	
	UPROPERTY()
	TObjectPtr<UPhysicalAnimationComponent> PhysicalAnimationComponent;

	UPROPERTY(Transient)
	FVector OriginalRelativeScale;

	bool bHasRagdollStarted = false;
};

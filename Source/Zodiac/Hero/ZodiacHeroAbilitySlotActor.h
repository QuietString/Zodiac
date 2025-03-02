// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacHeroAbilitySlotActor.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FZodiacImpactParticles
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Default;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Concrete;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UParticleSystem> Glass;
};

UENUM(BlueprintType)
enum class EZodiacHeroAbilitySlotSocketSourceType : uint8
{
	Self,
	AttachedParent,
	RemoteActor
};

UCLASS(BlueprintType, Abstract)
class ZODIAC_API AZodiacHeroAbilitySlotActor : public AActor
{
	GENERATED_BODY()

public:
	AZodiacHeroAbilitySlotActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Fire(const TArray<FVector>& ImpactPositions, const TArray<FVector>& ImpactNormals, const TArray<TEnumAsByte<EPhysicalSurface>>& SurfaceTypes, const AActor* RemoteActor);

	FVector GetSocketLocation(FName InSocketName) const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FVector GetSocketLocation() const;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void K2_OnRep_Owner();
	
private:
	virtual void OnRep_Owner() override;
	
public:
	UPROPERTY(EditAnywhere)
	USkeletalMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	EZodiacHeroAbilitySlotSocketSourceType SocketSource;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX" )
	FName SocketName;

	// Offsets are used when multiple bullets are fired from different sockets from a single fire.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TArray<FVector> Offsets;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> Trace_Niagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FLinearColor TracerColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	FZodiacImpactParticles Impacts;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX")
	TObjectPtr<UAnimMontage> MontageToPlay;
};
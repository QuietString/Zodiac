// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacNiagaraEffectTypes.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType, DisplayName = "Weapon Tracer Config")
struct FZodiacWeaponTracerConfig
{
	GENERATED_BODY()

public:
	FZodiacWeaponTracerConfig()
		: TracerColor(FLinearColor::White)
		, TracerLength(1.f)
		, TracerSpeed(40000)
		, TracerWidth(3.f)
		, bUseSmoke(true)
	{
		SmokeColors.Add(FLinearColor::White);
		SmokeColors.Add(FLinearColor::White);
		SmokeColors.Add(FLinearColor::White);
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	TObjectPtr<UNiagaraSystem> TracerSystem;
	
	// Offsets are used when multiple bullets are fired from different sockets from a single fire.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	TArray<FVector> Offsets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	FLinearColor TracerColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	float TracerLength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	float TracerSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	float TracerWidth;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer")
	bool bUseSmoke;
	
	// Smoke color array. Color elements will be interpolated by NormalizedAge.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FX|Tracer", meta = (EditCondition = "bUseSmoke"))
	TArray<FLinearColor> SmokeColors;
};

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
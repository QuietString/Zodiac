// the.quiet.string@gmail.com

#include "ZodiacDamageExecution.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySourceInterface.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "AbilitySystem/Attributes/ZodiacCombatSet.h"
#include "AbilitySystem/ZodiacGameplayEffectContext.h"
#include "Engine/World.h"
#include "Teams/ZodiacTeamSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacDamageExecution)

namespace ZodiacConsoleVariables
{
	static bool EnableLogDamageExecution = false;
	static FAutoConsoleVariableRef CVarEnableLogDamageExecution(
		TEXT("zodiac.LogChannel.DamageExecution"),
		EnableLogDamageExecution,
		TEXT("Should we log debug information of DamageExecution"),
		ECVF_Default);
}

struct FDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseDamageDef;

	FDamageStatics()
	{
		BaseDamageDef = FGameplayEffectAttributeCaptureDefinition(UZodiacCombatSet::GetBaseDamageAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


UZodiacDamageExecution::UZodiacDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().BaseDamageDef);
}

void UZodiacDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	FGameplayEffectContext* TypedContext = Spec.GetContext().Get();
	check(TypedContext);
	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;
	
	float BaseDamage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BaseDamageDef, EvaluateParameters, BaseDamage);

	float DamageMultiplier = Spec.GetSetByCallerMagnitude(ZodiacGameplayTags::SetByCaller_DamageMultiplier, false, 1.0f);

	const AActor* EffectCauser = TypedContext->GetEffectCauser();
	const FHitResult* HitActorResult = TypedContext->GetHitResult();

	AActor* HitActor = nullptr;
	FVector ImpactLocation = FVector::ZeroVector;
	FVector ImpactNormal = FVector::ZeroVector;
	FVector StartTrace = FVector::ZeroVector;
	FVector EndTrace = FVector::ZeroVector;

	float PhysicalMaterialAttenuation = 1.0f;
	
	// Calculation of hit actor, surface, zone, and distance all rely on whether the calculation has a hit result or not.
	// Effects just being added directly w/o having been targeted will always come in without a hit result, which must default
	// to some fallback information.
	if (HitActorResult)
	{
		const FHitResult& CurHitResult = *HitActorResult;
		HitActor = CurHitResult.HitObjectHandle.FetchActor();
		if (HitActor)
		{
			ImpactLocation = CurHitResult.ImpactPoint;
			ImpactNormal = CurHitResult.ImpactNormal;
			StartTrace = CurHitResult.TraceStart;
			EndTrace = CurHitResult.TraceEnd;
		}

		if (UObject* Source = TypedContext->GetSourceObject())
		{
			if (IZodiacAbilitySourceInterface* SourceInterface = Cast<IZodiacAbilitySourceInterface>(Source))
			{
				if (const UPhysicalMaterial* PhysMat = HitActorResult->PhysMaterial.Get())
				{
					PhysicalMaterialAttenuation = SourceInterface->GetPhysicalMaterialAttenuation(PhysMat);
				}
			}
		}
	}

	// Handle case of no hit result or hit result not actually returning an actor
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!HitActor)
	{
		HitActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor_Direct() : nullptr;
		if (HitActor)
		{
			ImpactLocation = HitActor->GetActorLocation();
		}
	}
	
	// Apply rules for team damage/self damage/etc...
	float DamageInteractionAllowedMultiplier = 0.0f;
	if (HitActor)
	{
		UZodiacTeamSubsystem* TeamSubsystem = HitActor->GetWorld()->GetSubsystem<UZodiacTeamSubsystem>();
		if (ensure(TeamSubsystem))
		{
			DamageInteractionAllowedMultiplier = TeamSubsystem->CanCauseDamage(EffectCauser, HitActor) ? 1.0 : 0.0;
		}
	}

	// Clamping is done when damage is converted to -health
	const float DamageDone = FMath::Max(BaseDamage * PhysicalMaterialAttenuation * DamageInteractionAllowedMultiplier, 0.0f);
	if (DamageDone > 0.0f)
	{
		// Apply a damage modifier, this gets turned into - health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UZodiacHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, DamageDone));
#if WITH_EDITOR
		if (ZodiacConsoleVariables::EnableLogDamageExecution)
		{
			UE_LOG(LogZodiacAbilitySystem, Display, TEXT("Damage execution: Base Damage: %1.f, Final Damage: %.1f"), BaseDamage, DamageDone);
		}
#endif
	}
#endif
}
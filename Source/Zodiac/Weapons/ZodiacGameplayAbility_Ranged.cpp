// the.quiet.string@gmail.com

#include "Weapons/ZodiacGameplayAbility_Ranged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "Character/ZodiacPlayerCharacter.h"
#include "Engine/StaticMeshActor.h"
#include "Physics/ZodiacCollisionChannels.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacGameplayAbility_Ranged)

namespace ZodiacConsoleVariables
{
	static float DrawBulletTracesDuration = 0.0f;
	static FAutoConsoleVariableRef CVarDrawBulletTraceDuration(
		TEXT("zodiac.Weapon.DrawBulletTraceDuration"),
		DrawBulletTracesDuration,
		TEXT("Should we do debug drawing for bullet traces (if above zero, sets how long (in seconds))"),
		ECVF_Default);

	// static float DrawBulletHitDuration = 0.0f;
	// static FAutoConsoleVariableRef CVarDrawBulletHits(
	// 	TEXT("zodiac.Weapon.DrawBulletHitDuration"),
	// 	DrawBulletHitDuration,
	// 	TEXT("Should we do debug drawing for bullet impacts (if above zero, sets how long (in seconds))"),
	// 	ECVF_Default);
	//
	// static float DrawBulletHitRadius = 3.0f;
	// static FAutoConsoleVariableRef CVarDrawBulletHitRadius(
	// 	TEXT("zodiac.Weapon.DrawBulletHitRadius"),
	// 	DrawBulletHitRadius,
	// 	TEXT("When bullet hit debug drawing is enabled (see DrawBulletHitDuration), how big should the hit radius be? (in uu)"),
	// 	ECVF_Default);
}

void UZodiacGameplayAbility_Ranged::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);
	
	FVector StartPoint = PlayerCharacter->GetPawnViewLocation();
	FVector ForwardVector = PlayerCharacter->GetBaseAimRotation().Vector();
	FVector EndPoint = StartPoint + ForwardVector * 10000.0f;

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActor(PlayerCharacter);
	Params.bTraceComplex = true;

	const ECollisionChannel TraceChannel = ZODIAC_TRACE_CHANNEL_WEAPON;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPoint,
		EndPoint,
		TraceChannel,
		Params
	);
	
	GCNParameter = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(HitResult);
	K2_ExecuteGameplayCueWithParams(GameplayCueTag_Firing, GCNParameter);

#if ENABLE_DRAW_DEBUG
	if (ZodiacConsoleVariables::DrawBulletTracesDuration > 0.0f)
	{
		const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
		const FVector EndLocation = bHit ? HitResult.Location : EndPoint;
		DrawDebugLine(GetWorld(), StartPoint, EndLocation, DebugColor, false, ZodiacConsoleVariables::DrawBulletTracesDuration, 0, 1.5f);	
	}
#endif
	
	if (bHit && HitResult.GetActor())
	{
		// Apply damage or a gameplay effect to the hit actor
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor());
		if (ASC)
		{
			ASC->ApplyGameplayEffectToSelf(DamageEffect.GetDefaultObject(), 1, ASC->MakeEffectContext());
		}
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

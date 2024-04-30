// the.quiet.string@gmail.com

#include "Weapons/ZodiacGameplayAbility_Ranged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "GameplayCueFunctionLibrary.h"
#include "Character/ZodiacPlayerCharacter.h"

void UZodiacGameplayAbility_Ranged::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                                    const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	AZodiacPlayerCharacter* PlayerCharacter = Cast<AZodiacPlayerCharacter>(GetCurrentActorInfo()->AvatarActor);
	
	FVector StartPoint = PlayerCharacter->GetPawnViewLocation();
	FVector ForwardVector = PlayerCharacter->GetBaseAimRotation().Vector();
	FVector EndPoint = StartPoint + ForwardVector * 10000.0f; // Set your desired range

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerCharacter);
	Params.bTraceComplex = true;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		StartPoint,
		EndPoint,
		ECC_Pawn,
		Params
	);

	GCNParameter = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult(HitResult);
	K2_ExecuteGameplayCueWithParams(GameplayCueTag_Firing, GCNParameter);
	
	if (bHit)
	{
		// Draw a red line if there is a hit
		DrawDebugLine(GetWorld(), StartPoint, HitResult.Location, FColor::Red, false, 2.5f, 0, 2.5f);
	}
	else
	{
		// Draw a green line if there is no hit
		DrawDebugLine(GetWorld(), StartPoint, HitResult.Location, FColor::Green, false, 2.5f, 0, 2.5f);

	}
	
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

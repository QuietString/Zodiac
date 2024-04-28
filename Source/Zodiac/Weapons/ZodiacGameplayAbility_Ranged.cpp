// the.quiet.string@gmail.com

#include "Weapons/ZodiacGameplayAbility_Ranged.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
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
		ECC_Pawn, // Or your custom collision channel
		Params
	);

	if (bHit)
	{
		// Draw a red line if there is a hit
		DrawDebugLine(
			GetWorld(),
			StartPoint,
			HitResult.Location,
			FColor::Red,
			false, // Persistent lines
			5.0f, // Duration the line should last
			0,    // Depth priority
			5.0f  // Thickness of the line
		);
	}
	else
	{
		// Draw a green line if there is no hit
		DrawDebugLine(
			GetWorld(),
			StartPoint,
			EndPoint,
			FColor::Green,
			false, // Persistent lines
			5.0f, // Duration the line should last
			0,    // Depth priority
			5.0f  // Thickness of the line
		);
	}
	
	if (bHit && HitResult.GetActor())
	{
		UE_LOG(LogTemp, Warning, TEXT("actor name: %s"), *HitResult.GetActor()->GetName());
		// Apply damage or a gameplay effect to the hit actor
		UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitResult.GetActor());
		if (ASC)
		{
			UE_LOG(LogTemp, Warning, TEXT(" have asc"));
			ASC->ApplyGameplayEffectToSelf(DamageEffect.GetDefaultObject(), 1, ASC->MakeEffectContext());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT(" no asc"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT(" hit but no actor"));
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

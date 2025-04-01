// the.quiet.string@gmail.com


#include "ZodiacHeroAnimInstance.h"

#include "AbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHostAnimInstance.h"
#include "Character/ZodiacCharacterMovementComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "Kismet/KismetMathLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAnimInstance)

void UZodiacHeroAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	UE_LOG(LogTemp, Warning, TEXT("%s: %s Hero Anim Instance Initialized"), GetOwningActor()->HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwningActor()->GetName());
}

void UZodiacHeroAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);

	AActor* OwningActor = GetOwningActor();
	ParentCharacter = GetParentCharacter();
	ParentAnimInstance = GetParentAnimInstance();
	
	if (!ParentCharacter || !ParentAnimInstance)
	{
		return;
	}

	UpdateMovementData();
	UpdateRotationData();
	UpdateAimingData(DeltaSeconds);

	GameplayTagPropertyReverseMap.TickUpdateProperties();
}

void UZodiacHeroAnimInstance::InitializeWithAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	GameplayTagPropertyMap.Initialize(this, InASC);
	GameplayTagPropertyReverseMap.Initialize(this, InASC);

	UE_LOG(LogTemp, Warning, TEXT("%s: %s Hero Anim Instance ASC Initialized"), GetOwningActor()->HasAuthority() ? TEXT("Server") : TEXT("Client"), *GetOwningActor()->GetName());
}

AZodiacCharacter* UZodiacHeroAnimInstance::GetParentCharacter() const
{
	if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(GetOwningActor()))
	{
		return ZodiacCharacter;
	}
	
	if (AZodiacHeroCharacter* Hero = Cast<AZodiacHeroCharacter>(GetOwningActor()))
	{
		return Hero->GetHostCharacter();
	}

	return nullptr;
}

UZodiacHostAnimInstance* UZodiacHeroAnimInstance::GetParentAnimInstance() const
{
	if (ParentCharacter)
	{
		return  Cast<UZodiacHostAnimInstance>(ParentCharacter->GetMesh()->GetAnimInstance());
	}

	return nullptr;
}

void UZodiacHeroAnimInstance::OnStatusChanged(FGameplayTag Tag, bool bHasTag)
{
	if (Tag == ZodiacGameplayTags::Status_Focus)
	{
		bIsFocus = bHasTag;
	}
	else if (Tag == ZodiacGameplayTags::Status_Death)
	{
		bIsDead = bHasTag;
	}
	
	else if (Tag == ZodiacGameplayTags::Status_ADS)
	{
		bIsADS = bHasTag;
	}
}

void UZodiacHeroAnimInstance::UpdateMovementData()
{
	bIsMoving = ParentAnimInstance->bIsMoving;
	bIsTraversal = ParentAnimInstance->bIsTraversal;

	ExtendedMovementMode = ParentAnimInstance->ExtendedMovementMode;
	
	bIsStrafing = (ExtendedMovementMode != EZodiacExtendedMovementMode::Sprinting);
	MovementAngle = ParentAnimInstance->MovementAngle;
}

void UZodiacHeroAnimInstance::UpdateRotationData()
{
	RootRotationOffset = ParentAnimInstance->RootTransform.Rotator();
	RootRotationOffset.Yaw -= 90.f;
}

void UZodiacHeroAnimInstance::UpdateAimingData(float DeltaSeconds)
{
	AimYawLast = AimYaw;
	
	FRotator AimRotation = ParentCharacter->GetBaseAimRotation();
	FRotator ControlRotation = ParentCharacter->GetControlRotation();
	FRotator TargetRotation = ParentCharacter->IsLocallyControlled() ? ControlRotation : AimRotation; 
	FRotator RootTransform = ParentAnimInstance->RootTransform.Rotator();

	FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(TargetRotation, RootTransform);
	AimPitch = Delta.Pitch;
	
	if (ParentCharacter->GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Ignore large error caused by actor rotation de-sync when not strafing.
		// When UCharacterMovementComponent->bOrientRotationToMovement become true, actor rotate towards input direction and AimYaw become large.
		// In that case, we will use ReplicatedIndependentYaw, but it replicates slowly.
		float YawDiff = FRotator::NormalizeAxis(Delta.Yaw - AimYawLast);
		
		if (FMath::Abs(YawDiff) < 70.f)
		{
			AimYaw = Delta.Yaw;
		}
		
		FZodiacReplicatedIndependentYaw IndependentYaw = ParentCharacter->GetReplicatedIndependentYaw();
		if (IndependentYaw.bIsAllowed)
		{
			FRotator IndependentAimRotation(0.f, IndependentYaw.GetUnpackedYaw(), 0.f);
			AimYaw = FRotator::NormalizeAxis(IndependentYaw.GetUnpackedYaw() - RootTransform.Yaw);
		}
	}
	else
	{
		AimYaw = Delta.Yaw;
	}
}

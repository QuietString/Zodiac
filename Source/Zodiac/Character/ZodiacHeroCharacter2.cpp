// the.quiet.string@gmail.com


#include "ZodiacHeroCharacter2.h"

#include "AbilitySystemComponent.h"
#include "ZodiacCharacterMovementComponent.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Camera/ZodiacCameraMode.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroCharacter2)

AZodiacHeroCharacter2::AZodiacHeroCharacter2(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	RetargetSourceMesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("RetargetSourceMesh"));
	RetargetSourceMesh->SetupAttachment(GetCapsuleComponent());
	
	CameraComponent = CreateDefaultSubobject<UZodiacCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	HealthComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacHealthComponent>(this, TEXT("HealthComponent"));
}

UZodiacAbilitySystemComponent* AZodiacHeroCharacter2::GetZodiacAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAbilitySystemComponent* AZodiacHeroCharacter2::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacHeroCharacter2::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner)
{
	Super::InitializeAbilitySystem(InASC, InOwner);

	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
}

void AZodiacHeroCharacter2::BeginPlay()
{
	Super::BeginPlay();

	if (!HeroData.IsEmpty())
	{
		if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
		{
			FZodiacExtendedMovementConfig MovementConfig = HeroData.Last()->ExtendedMovementConfig;
			ZodiacCharMovComp->SetExtendedMovementConfig(MovementConfig);
		}
	}
}

void AZodiacHeroCharacter2::PossessedBy(class AController* NewController)
{
	Super::PossessedBy(NewController);

	APlayerState* PS = GetPlayerStateChecked<APlayerState>();
	InitializeAbilitySystem(AbilitySystemComponent, PS);
}

void AZodiacHeroCharacter2::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveHeroIndex);
}

void AZodiacHeroCharacter2::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
	CameraComponent->OnCloseContactStarted.BindUObject(this, &ThisClass::OnCloseContactStarted);
	CameraComponent->OnCloseContactFinished.BindUObject(this, &ThisClass::OnCloseContactFinished);
	
	if (CameraComponent->bApplyTranslationOffset)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			
		}
	}

	if (RetargetSourceMesh)
	{
		RetargetSourceMesh->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	}

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AZodiacHeroCharacter2::ToggleSprint(bool bShouldSprint)
{
	
}

TSubclassOf<UZodiacCameraMode> AZodiacHeroCharacter2::DetermineCameraMode()
{
	if (ActiveAbilityCameraMode)
	{
		return ActiveAbilityCameraMode;
	}

	return DefaultAbilityCameraMode;
}

void AZodiacHeroCharacter2::UpdateHeroEyeLocationOffset()
{
}

void AZodiacHeroCharacter2::OnRep_ActiveHeroIndex(int32 OldIndex)
{
	
}

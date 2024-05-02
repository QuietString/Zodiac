// the.quiet.string@gmail.com


#include "ZodiacPlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroComponent.h"
#include "Input/ZodiacInputData.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Input/ZodiacInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerState.h"

AZodiacPlayerCharacter::AZodiacPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	ActiveHeroIndex = INDEX_NONE;

	HeroComponent1 = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroComponent>(this, TEXT("HeroComponent1"));
	HeroComponent2 = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroComponent>(this, TEXT("HeroComponent2"));

	HeroMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("HeroMeshComponent"));
	HeroMeshComponent->SetupAttachment(GetMesh(), NAME_None);

	CameraComponent = CreateDefaultSubobject<UZodiacCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));

	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());
	ZodiacMoveComp->GravityScale = 1.0f;
	ZodiacMoveComp->MaxAcceleration = 2400.0f;
	ZodiacMoveComp->BrakingFrictionFactor = 1.0f;
	ZodiacMoveComp->BrakingFriction = 6.0f;
	ZodiacMoveComp->GroundFriction = 8.0f;
	ZodiacMoveComp->BrakingDecelerationWalking = 1400.0f;
	ZodiacMoveComp->bUseControllerDesiredRotation = false;
	ZodiacMoveComp->bOrientRotationToMovement = false;
	ZodiacMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	ZodiacMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	ZodiacMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	ZodiacMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	ZodiacMoveComp->SetCrouchedHalfHeight(65.0f);
}

void AZodiacPlayerCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveHeroIndex);
}

void AZodiacPlayerCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (GetZodiacAbilitySystemComponent())
	{
		GetZodiacAbilitySystemComponent()->GetOwnedGameplayTags(TagContainer);
	}
}

UZodiacAbilitySystemComponent* AZodiacPlayerCharacter::GetZodiacAbilitySystemComponent() const
{
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex) && bHeroesInitialized)
	{
		return AbilitySystemComponents[ActiveHeroIndex];
	}

	return nullptr;
}

UAbilitySystemComponent* AZodiacPlayerCharacter::GetAbilitySystemComponent() const
{
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex) && bHeroesInitialized)
	{
		return AbilitySystemComponents[ActiveHeroIndex];
	}
	
	return nullptr;
}

void AZodiacPlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>( GetPlayerState()))
	{
		MyTeam = GenericTeamToZodiacTeam(ZodiacPS->GetGenericTeamId());
	}
	else
	{
		MyTeam = EZodiacTeam::Neutral;
	}
}

void AZodiacPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	SelectFirstHero();

	// TInlineComponentArray<UZodiacHealthComponent*> HealthComponents;
	// GetComponents<UZodiacHealthComponent>(HealthComponents);
}

void AZodiacPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
	InitializeHeroComponents();

	UE_LOG(LogTemp, Warning,TEXT("PostInitializeComponents"));
}

void AZodiacPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (IsLocallyControlled())
	{
		InitializePlayerInput();
	}
}

void AZodiacPlayerCharacter::SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode,
	const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		ActiveAbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void AZodiacPlayerCharacter::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		ActiveAbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

void AZodiacPlayerCharacter::ChangeHero(int32 NewIndex)
{
	if (HeroComponents.IsValidIndex(NewIndex))
	{
		if (HasAuthority())
		{
			int32 OldIndex = ActiveHeroIndex;
			ActiveHeroIndex = NewIndex;
			OnRep_ActiveHeroIndex(OldIndex);
		}
	}
}

void AZodiacPlayerCharacter::ChangeCharacterMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimInstance)
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetSkeletalMeshAsset(NewMesh);
		MeshComponent->SetAnimInstanceClass(NewAnimInstance);
	}
}

void AZodiacPlayerCharacter::ChangeHeroMesh(USkeletalMesh* NewMesh)
{
	HeroMeshComponent->SetSkeletalMeshAsset(NewMesh);
}

void AZodiacPlayerCharacter::CheckReady()
{
	// check player connected
	bool bServerConnected = false;
	if (HasAuthority())
	{
		bServerConnected = true;
	}
	else
	{
		if (APlayerState* PS = GetPlayerState())
		{
			bServerConnected = true;
		}
	}

	if (bServerConnected && bHeroesInitialized)
	{
		SelectFirstHero();
		bReady = true;
	}
}

void AZodiacPlayerCharacter::InitializeHeroComponents()
{
	HeroComponents.Reset(2);
	AbilitySystemComponents.Reset(2);

	bool bComponent1Ready = false;
	bool bComponent2Ready = false;
	
	if (HeroComponent1)
	{
		HeroComponents.Add(HeroComponent1);
		UZodiacAbilitySystemComponent* HeroASC1 = HeroComponent1->InitializeAbilitySystem();
		check(HeroASC1);
		AbilitySystemComponents.Add(HeroASC1);
		bComponent1Ready = true;	
	}

	if (HeroComponent2)
	{
		HeroComponents.Add(HeroComponent2);
		UZodiacAbilitySystemComponent* HeroASC2 = HeroComponent2->InitializeAbilitySystem();
		check(HeroASC2);
		AbilitySystemComponents.Add(HeroASC2);
		bComponent2Ready = true;
	}

	if (bComponent1Ready & bComponent2Ready)
	{
		bHeroesInitialized = true;
	}

	CheckReady();
}

void AZodiacPlayerCharacter::SelectFirstHero()
{
	HeroComponent1->DeactivateHero();
	HeroComponent2->DeactivateHero();
	
	ActiveHeroIndex = 0;
	OnRep_ActiveHeroIndex(INDEX_NONE);
}

TSubclassOf<UZodiacCameraMode> AZodiacPlayerCharacter::DetermineCameraMode()
{
	if (ActiveAbilityCameraMode)
	{
		return ActiveAbilityCameraMode;
	}

	return DefaultAbilityCameraMode;
}

void AZodiacPlayerCharacter::InitializePlayerInput()
{
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	if (InputData)
	{
		// Add input mapping context (player input to input action)
		if (InputData->InputMappingContexts.Num() > 0)
		{
			for (UInputMappingContext* IMC : InputData->InputMappingContexts)
			{
				FModifyContextOptions Options = {};
				Options.bIgnoreAllPressedKeysUntilRelease = true;
				Subsystem->AddMappingContext(IMC, 0, Options);
			}
		}
		
		if (const UZodiacInputTagMapping* InputConfig = InputData->InputConfig)
		{
			UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(InputComponent);
			if (ensureMsgf(ZodiacIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UZodiacInputComponent or a subclass of it.")))
			{
				// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
				// be triggered directly by these input actions Triggered events. 
				TArray<uint32> BindHandles;
				ZodiacIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, OUT BindHandles);

				ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
				ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, false);
				//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
				ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, false);
				//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
			}
		}
	}
}

void AZodiacPlayerCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex))
	{
		AbilitySystemComponents[ActiveHeroIndex]->AbilityInputTagPressed(InputTag);	
	}
}

void AZodiacPlayerCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex))
	{
		AbilitySystemComponents[ActiveHeroIndex]->AbilityInputTagReleased(InputTag);	
	}
}

void AZodiacPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void AZodiacPlayerCharacter::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		AddControllerPitchInput(-Value.Y);
	}
}

void AZodiacPlayerCharacter::Input_Crouch(const FInputActionValue& InputActionValue)
{
	const UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());

	if (bIsCrouched || ZodiacMoveComp->bWantsToCrouch)
	{
		UnCrouch();
	}
	else if (ZodiacMoveComp->IsMovingOnGround())
	{
		Crouch();
	}
}

void AZodiacPlayerCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Crouching, 1);
	}
	
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);}

void AZodiacPlayerCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Crouching, 0);
	}
	
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);}

bool AZodiacPlayerCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void AZodiacPlayerCharacter::OnRep_ActiveHeroIndex(int32 OldIndex)
{
	if (HeroComponents.IsValidIndex(OldIndex))
	{
		HeroComponents[OldIndex]->DeactivateHero();
	}

	if (HeroComponents.IsValidIndex(ActiveHeroIndex))
	{
		HeroComponents[ActiveHeroIndex]->ActivateHero();
		
	}
}
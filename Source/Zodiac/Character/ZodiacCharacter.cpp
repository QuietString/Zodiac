﻿// Copyright Epic Games, Inc. All Rights Reserved.
// the.quiet.string@gmail.com


#include "ZodiacCharacter.h"
#include "ZodiacPawnExtensionComponent.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHealthComponent.h"
#include "Hero/ZodiacHeroData.h"
#include "ZodiacPreMovementComponentTickComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Camera/ZodiacCameraMode.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCharacter)

/////////////////////////////////////////////////////////////////
///

FSharedRepMovement::FSharedRepMovement()
{
	RepMovement.LocationQuantizationLevel = EVectorQuantization::RoundTwoDecimals;
}

bool FSharedRepMovement::FillForCharacter(ACharacter* Character)
{
	if (USceneComponent* PawnRootComponent = Character->GetRootComponent())
	{
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();

		RepMovement.Location = FRepMovement::RebaseOntoZeroOrigin(PawnRootComponent->GetComponentLocation(), Character);
		RepMovement.Rotation = PawnRootComponent->GetComponentRotation();
		RepMovement.LinearVelocity = CharacterMovement->Velocity;
		RepMovementMode = CharacterMovement->PackNetworkMovementMode();
		bProxyIsJumpForceApplied = Character->bProxyIsJumpForceApplied || (Character->JumpForceTimeRemaining > 0.0f);

		// Timestamp is sent as zero if unused
		if ((CharacterMovement->NetworkSmoothingMode == ENetworkSmoothingMode::Linear) || CharacterMovement->bNetworkAlwaysReplicateTransformUpdateTimestamp)
		{
			RepTimeStamp = CharacterMovement->GetServerLastTransformUpdateTimeStamp();
		}
		else
		{
			RepTimeStamp = 0.f;
		}

		return true;
	}
	return false;
}

bool FSharedRepMovement::Equals(const FSharedRepMovement& Other, ACharacter* Character) const
{
	if (RepMovement.Location != Other.RepMovement.Location)
	{
		return false;
	}

	if (RepMovement.Rotation != Other.RepMovement.Rotation)
	{
		return false;
	}

	if (RepMovement.LinearVelocity != Other.RepMovement.LinearVelocity)
	{
		return false;
	}

	if (RepMovementMode != Other.RepMovementMode)
	{
		return false;
	}

	if (bProxyIsJumpForceApplied != Other.bProxyIsJumpForceApplied)
	{
		return false;
	}
	
	return true;
}

bool FSharedRepMovement::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	bOutSuccess = true;
	RepMovement.NetSerialize(Ar, Map, bOutSuccess);
	Ar << RepMovementMode;
	Ar << bProxyIsJumpForceApplied;

	// Timestamp, if non-zero.
	uint8 bHasTimeStamp = (RepTimeStamp != 0.f);
	Ar.SerializeBits(&bHasTimeStamp, 1);
	if (bHasTimeStamp)
	{
		Ar << RepTimeStamp;
	}
	else
	{
		RepTimeStamp = 0.f;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////
///

const FName AZodiacCharacter::ExtensionComponentName("PawnExtensionComponent");

AZodiacCharacter::AZodiacCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(CharacterMovementComponentName))
{
	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());
	ZodiacMoveComp->GravityScale = 1.0f;
	ZodiacMoveComp->MaxAcceleration = 800.0f;
	ZodiacMoveComp->BrakingDecelerationWalking = 1500.f;
	ZodiacMoveComp->BrakingFrictionFactor = 1.0f;
	ZodiacMoveComp->BrakingFriction = 6.0f;
	ZodiacMoveComp->GroundFriction = 5.0f;
	ZodiacMoveComp->JumpZVelocity = 500.f;
	ZodiacMoveComp->RotationRate = FRotator(0, 0, -1.f);
	ZodiacMoveComp->bUseControllerDesiredRotation = true;
	ZodiacMoveComp->bOrientRotationToMovement = false;
	ZodiacMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	ZodiacMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	ZodiacMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	ZodiacMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	ZodiacMoveComp->SetCrouchedHalfHeight(65.0f);
	
	PawnExtComponent = CreateDefaultSubobject<UZodiacPawnExtensionComponent>(ExtensionComponentName);
	PawnExtComponent->RegisterAndCall_OnAbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));
	PawnExtComponent->Register_OnAbilitySystemUninitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemUninitialized));
	
	CameraComponent = CreateDefaultSubobject<UZodiacCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
	
	PreMovementComponentTick = ObjectInitializer.CreateDefaultSubobject<UZodiacPreMovementComponentTickComponent>(this, TEXT("PreMovementComponentTick"));
}

void AZodiacCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedIndependentYaw, COND_SimulatedOnly);
}

void AZodiacCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	if (UCharacterMovementComponent* MovementComponent = GetCharacterMovement())
	{
		// Compress Acceleration: XY components as direction + magnitude, Z component as direct value
		const double MaxAccel = MovementComponent->MaxAcceleration;
		const FVector CurrentAccel = MovementComponent->GetCurrentAcceleration();
		double AccelXYRadians, AccelXYMagnitude;
		FMath::CartesianToPolar(CurrentAccel.X, CurrentAccel.Y, AccelXYMagnitude, AccelXYRadians);

		ReplicatedAcceleration.AccelXYRadians   = FMath::FloorToInt((AccelXYRadians / TWO_PI) * 255.0);     // [0, 2PI] -> [0, 255]
		ReplicatedAcceleration.AccelXYMagnitude = FMath::FloorToInt((AccelXYMagnitude / MaxAccel) * 255.0);	// [0, MaxAccel] -> [0, 255]
		ReplicatedAcceleration.AccelZ           = FMath::FloorToInt((CurrentAccel.Z / MaxAccel) * 127.0);   // [-MaxAccel, MaxAccel] -> [-127, 127]
	}

	if (Controller && ReplicatedIndependentYaw.bIsAllowed)
	{
		const FRotator ControlRotation = Controller->GetControlRotation();
		ReplicatedIndependentYaw.Yaw = FMath::FloorToInt(ControlRotation.Yaw * 255.0 / 360.0);     // [0, 360] -> [0, 255]
	}
}

void AZodiacCharacter::RegisterGameplayTagEvents(UAbilitySystemComponent* InASC)
{
	if (!InASC)
	{
		return;
	}
	
	InASC->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Death, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AZodiacCharacter::OnStatusTagChanged);
	InASC->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Movement_Disabled, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AZodiacCharacter::OnStatusTagChanged);
	InASC->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AZodiacCharacter::OnStatusTagChanged);
	InASC->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Physics_Collision_Disabled, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AZodiacCharacter::OnPhysicsTagChanged);
}

void AZodiacCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (IsLocallyControlled())
	{
		SetupCameraComponent();
	}
	
	PawnExtComponent->HandleControllerChanged();
}

void AZodiacCharacter::UnPossessed()
{
	Super::UnPossessed();

	PawnExtComponent->HandleControllerChanged();
}

void AZodiacCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	UE_LOG(LogTemp, Warning, TEXT("%s Onrep Controller: %s"), HasAuthority() ? TEXT("server") : TEXT("Client"), *GetNameSafe(Controller));
	
	if (IsLocallyControlled())
	{
		SetupCameraComponent();	
	}
	
	PawnExtComponent->HandleControllerChanged();
}

void AZodiacCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->HandlePlayerStateReplicated();
}

void AZodiacCharacter::BeginPlay()
{
	Super::BeginPlay();

	GetCharacterMovement()->AddTickPrerequisiteComponent(PreMovementComponentTick);
}

UAbilitySystemComponent* AZodiacCharacter::GetAbilitySystemComponent() const
{
	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacCharacter::GetZodiacAbilitySystemComponent() const
{
	return nullptr;
}

UZodiacHealthComponent* AZodiacCharacter::GetHealthComponent() const
{
	return nullptr;
}

FGenericTeamId AZodiacCharacter::GetGenericTeamId() const
{
	return IZodiacTeamAgentInterface::GetGenericTeamId();
}

void AZodiacCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->GetOwnedGameplayTags(TagContainer);
	}
}

bool AZodiacCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasMatchingGameplayTag(TagToCheck);
	}

	return false;
}

bool AZodiacCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasAllMatchingGameplayTags(TagContainer);
	}

	return false;
}

bool AZodiacCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		return ZodiacASC->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}

void AZodiacCharacter::OnCharacterAttached(AActor* AttachedActor)
{
	if (UZodiacHostAnimInstance* HostAnimInstance = CastChecked<UZodiacHostAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		HostAnimInstance->ActorsToIgnoreTrajectory.AddUnique(AttachedActor);
	}
}

void AZodiacCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	check(Canvas);

	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	if (IsLocallyControlled())
	{
		FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

		DisplayDebugManager.SetFont(GEngine->GetSmallFont());
		DisplayDebugManager.SetDrawColor(FColor::Yellow);
		DisplayDebugManager.DrawString(FString::Printf(TEXT("ZodiacCharacter: %s"), *GetNameSafe(this)));
		DisplayDebugManager.DrawString(FString::Printf(TEXT("MovementMode: %d, CustomMovementMode: %d"), GetCharacterMovement()->MovementMode.GetValue(), GetCharacterMovement()->CustomMovementMode));
	}
}

void AZodiacCharacter::SimulateOrPlayHitReact_Implementation(FVector HitDirection, FName HitBone, float Magnitude,
	FGameplayTagContainer InstigatorTags)
{
	OnSimulateOrPlayHitReact.ExecuteIfBound(HitDirection, HitBone, Magnitude, InstigatorTags);
}

void AZodiacCharacter::SetExtendedMovementMode(const EZodiacExtendedMovementMode& InMode)
{
	if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		ZodiacCharMovComp->SetExtendedMovementMode(InMode);
	}
}

void AZodiacCharacter::SetExtendedMovementConfig(const FZodiacExtendedMovementConfig& InConfig)
{
	if (UZodiacCharacterMovementComponent* ZodiacCharMovComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		ZodiacCharMovComp->SetExtendedMovementConfig(InConfig);
	}
}

TSubclassOf<UZodiacCameraMode> AZodiacCharacter::DetermineCameraMode()
{
	if (ActiveAbilityCameraMode)
	{
		return ActiveAbilityCameraMode;
	}

	return DefaultAbilityCameraMode;
}

void AZodiacCharacter::SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		ActiveAbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void AZodiacCharacter::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		ActiveAbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}

void AZodiacCharacter::OnAbilitySystemInitialized()
{
	UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent();
	check(ZodiacASC);

	if (GetHealthComponent())
	{
		GetHealthComponent()->InitializeWithAbilitySystem(ZodiacASC);
	}

	InitializeGameplayTags();
}

void AZodiacCharacter::OnAbilitySystemUninitialized()
{
	if (GetHealthComponent())
	{
		GetHealthComponent()->UninitializeFromAbilitySystem();	
	}
}

void AZodiacCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UAbilitySystemComponent* ZodiacASC = GetAbilitySystemComponent())
	{
		for (const TPair<uint8, FGameplayTag>& TagMapping : ZodiacGameplayTags::MovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				ZodiacASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		for (const TPair<uint8, FGameplayTag>& TagMapping : ZodiacGameplayTags::CustomMovementModeTagMap)
		{
			if (TagMapping.Value.IsValid())
			{
				ZodiacASC->SetLooseGameplayTagCount(TagMapping.Value, 0);
			}
		}

		UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());
		SetMovementModeTag(ZodiacMoveComp->MovementMode, ZodiacMoveComp->CustomMovementMode, true);
	}
}

void AZodiacCharacter::SetupCameraComponent()
{
	if (!CameraComponent->DetermineCameraModeDelegate.IsBound())
	{
		CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);	
	}

	if (!CameraComponent->OnCloseContactFinished.IsBound())
	{
		CameraComponent->OnCloseContactStarted.BindUObject(this, &ThisClass::OnCloseContactStarted);	
	}

	if (!CameraComponent->OnCloseContactFinished.IsBound())
	{
		CameraComponent->OnCloseContactFinished.BindUObject(this, &ThisClass::OnCloseContactFinished);	
	}
	
	if (CameraComponent->bApplyTranslationOffset)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (UZodiacHostAnimInstance* HostAnimInstance = Cast<UZodiacHostAnimInstance>(AnimInstance))
			{
				if (!CameraComponent->UpdateCameraTranslationOffsetDelegate.IsBound())
				{
					CameraComponent->UpdateCameraTranslationOffsetDelegate.BindUObject(HostAnimInstance, &UZodiacHostAnimInstance::GetTranslationOffset);
					CameraComponent->AimYawPtr = &HostAnimInstance->AimYaw;	
				}
			}
		}	
	}
}

void AZodiacCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (IsLocallyControlled())
	{ 
		InitializePlayerInput();
	}
}

void AZodiacCharacter::InitializePlayerInput()
{
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	if (InputConfig.IsValid())
	{
		// Add input mapping context (player input to input action)
		for (UInputMappingContext* IMC : InputConfig.MappingContexts)
		{
			FModifyContextOptions Options = {};
			Options.bIgnoreAllPressedKeysUntilRelease = true;
			Subsystem->AddMappingContext(IMC, 0, Options);
		}
		
		UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(InputComponent);
		if (ensureMsgf(ZodiacIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UZodiacInputComponent or a subclass of it.")))
		{
			// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
			// be triggered directly by these input actions Triggered events. 
			TArray<uint32> BindHandles;
			ZodiacIC->BindAbilityActions(InputConfig.TagMapping, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, OUT BindHandles);

			ZodiacIC->BindNativeAction(InputConfig.TagMapping, ZodiacGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
			ZodiacIC->BindNativeAction(InputConfig.TagMapping, ZodiacGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, false);
			ZodiacIC->BindNativeAction(InputConfig.TagMapping, ZodiacGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, false);
			ZodiacIC->BindNativeAction(InputConfig.TagMapping, ZodiacGameplayTags::InputTag_Move_Fly, ETriggerEvent::Triggered, this, &ThisClass::Input_Fly, false);
		}
	}
}

void AZodiacCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->AbilityInputTagPressed(InputTag);	
	}
}

void AZodiacCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->AbilityInputTagReleased(InputTag);	
	}
}

void AZodiacCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
	if (Controller)
	{
		if (HasMatchingGameplayTag(ZodiacGameplayTags::Status_Stun) || HasMatchingGameplayTag(ZodiacGameplayTags::Status_Movement_Disabled))
		{
			return;
		}
		
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

void AZodiacCharacter::Input_Fly(const FInputActionValue& InputActionValue)
{
	if (Controller)
	{
		if (HasMatchingGameplayTag(ZodiacGameplayTags::Status_Stun) || HasMatchingGameplayTag(ZodiacGameplayTags::Status_Movement_Disabled))
		{
			return;
		}
		
		const float Value = InputActionValue.Get<float>();
		if (Value != 0.0f)
		{
			AddMovementInput(FVector::UpVector, Value);
		}
	}
}

void AZodiacCharacter::Input_LookMouse(const FInputActionValue& InputActionValue)
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

void AZodiacCharacter::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	bool bHasTag = Count > 0;

	UZodiacHostAnimInstance* HostAnimInstance = CastChecked<UZodiacHostAnimInstance>(GetMesh()->GetAnimInstance());

	HostAnimInstance->OnStatusChanged(Tag, bHasTag);

	if (Tag == ZodiacGameplayTags::Status_Movement_Disabled || Tag == ZodiacGameplayTags::Status_Stun)
	{
		bMovementDisabled = bHasTag;
	}
}

void AZodiacCharacter::OnPhysicsTagChanged(FGameplayTag Tag, int Count)
{
	bool bHasTag = Count > 0;

	if (Tag == ZodiacGameplayTags::Status_Physics_Collision_Disabled)
	{
		if (bHasTag)
		{
			GetCapsuleComponent()->SetCollisionProfileName("NoCollision");
		}
		else
		{
			GetCapsuleComponent()->SetCollisionProfileName("ZodiacPawnCapsule");
		}
	}
}

void AZodiacCharacter::OnJustLanded()
{
	if (const APlayerController* PC = GetController<APlayerController>())
	{
		if (const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer()))
		{
			UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			check(Subsystem);

			if (InputConfig.IsValid() && InputConfig.InAirContext)
			{
				Subsystem->RemoveMappingContext(InputConfig.InAirContext);
			}
		}
	}
}

void AZodiacCharacter::OnJustLifted()
{
	if (const APlayerController* PC = GetController<APlayerController>())
	{
		if (const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer()))
		{
			UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			check(Subsystem);

			if (InputConfig.IsValid() && InputConfig.InAirContext)
			{
				FModifyContextOptions Options = {};
				Options.bIgnoreAllPressedKeysUntilRelease = true;
				Subsystem->AddMappingContext(InputConfig.InAirContext, 6, Options);
			}
		}
	}
}

void AZodiacCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);

	EMovementMode MovementMode = ZodiacMoveComp->MovementMode;
	uint8 CustomMovementMode = ZodiacMoveComp->CustomMovementMode;
	
	SetMovementModeTag(MovementMode, CustomMovementMode, true);

	if (ZodiacMoveComp->IsMovingOnGround())
	{
		if (PrevMovementMode == MOVE_Falling || PrevMovementMode == MOVE_Flying || PreviousCustomMode == Move_Custom_Traversal)
		{
			if (MovementMode == MOVE_Walking || MOVE_NavWalking)
			{
				OnJustLanded();	
			}
		}
	}
	else
	{
		if (PrevMovementMode == MOVE_Walking || PrevMovementMode == MOVE_NavWalking)
		{
			if (MovementMode == MOVE_Flying || MOVE_Falling)
			{
				OnJustLifted();	
			}
		}
	}
}

void AZodiacCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag;

		if (CustomMovementMode == MOVE_None)
		{
			MovementModeTag = ZodiacGameplayTags::MovementModeTagMap.Find(MovementMode);
			if (MovementModeTag && MovementModeTag->IsValid())
			{
				ASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
			}
		}
		else if (CustomMovementMode == Move_Custom_Traversal)
		{
			MovementModeTag = ZodiacGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
			if (MovementModeTag && MovementModeTag->IsValid())
			{
				ASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
			}
		}
	}
}

void AZodiacCharacter::OnExtendedMovementModeChanged(EZodiacExtendedMovementMode PreviousMode)
{
	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());

	SetExtendedMovementModeTag(PreviousMode, false);
	SetExtendedMovementModeTag(ZodiacMoveComp->GetExtendedMovementMode(), true);
	BP_OnExtendedMovementModeChanged.Broadcast(ZodiacMoveComp->GetExtendedMovementMode());
}

void AZodiacCharacter::SetExtendedMovementModeTag(EZodiacExtendedMovementMode ExtendedMovementMode, bool bTagEnabled)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = ZodiacGameplayTags::ExtendedMovementModeTagMap.Find(ExtendedMovementMode);
		
		if (MovementModeTag && MovementModeTag->IsValid())
		{
			ZodiacASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

void AZodiacCharacter::ToggleSprint(bool bShouldSprint)
{
	if (UZodiacCharacterMovementComponent* ZodiacCharMoveComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		ReplicatedIndependentYaw.bIsAllowed = bShouldSprint;
		ZodiacCharMoveComp->ToggleSprint(bShouldSprint);

		// Don't strafe when sprinting
		ZodiacCharMoveComp->ToggleStrafe(!bShouldSprint);
	}
}

bool AZodiacCharacter::GetIsStrafing() const
{
	if (UZodiacCharacterMovementComponent* ZodiacCharacterMovementComponent = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		return ZodiacCharacterMovementComponent->GetIsStrafing();
	}

	return false;
}

void AZodiacCharacter::WakeUp(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
}

void AZodiacCharacter::Multicast_WakeUp_Implementation(const FVector& SpawnLocation, const FRotator& SpawnRotation)
{
	WakeUp(SpawnLocation, SpawnRotation);
}

void AZodiacCharacter::Sleep()
{
}

void AZodiacCharacter::Multicast_Sleep_Implementation()
{
	Sleep();
}

bool AZodiacCharacter::UpdateSharedReplication()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FSharedRepMovement SharedMovement;
		if (SharedMovement.FillForCharacter(this))
		{
			// Only call FastSharedReplication if data has changed since the last frame.
			// Skipping this call will cause replication to reuse the same bunch that we previously
			// produced, but not send it to clients that already received. (But a new client who has not received
			// it, will get it this frame)
			if (!SharedMovement.Equals(LastSharedReplication, this))
			{
				LastSharedReplication = SharedMovement;
				ReplicatedMovementMode = SharedMovement.RepMovementMode;

				FastSharedReplication(SharedMovement);
			}
			return true;
		}
	}
	
	// We cannot fast rep right now. Don't send anything.
	return false;
}

void AZodiacCharacter::OnRep_ReplicatedAcceleration()
{
	if (UZodiacCharacterMovementComponent* ZodiacMovementComponent = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		// Decompress Acceleration
		const double MaxAccel         = ZodiacMovementComponent->MaxAcceleration;
		const double AccelXYMagnitude = double(ReplicatedAcceleration.AccelXYMagnitude) * MaxAccel / 255.0; // [0, 255] -> [0, MaxAccel]
		const double AccelXYRadians   = double(ReplicatedAcceleration.AccelXYRadians) * TWO_PI / 255.0;     // [0, 255] -> [0, 2PI]

		FVector UnpackedAcceleration(FVector::ZeroVector);
		FMath::PolarToCartesian(AccelXYMagnitude, AccelXYRadians, UnpackedAcceleration.X, UnpackedAcceleration.Y);
		UnpackedAcceleration.Z = double(ReplicatedAcceleration.AccelZ) * MaxAccel / 127.0; // [-127, 127] -> [-MaxAccel, MaxAccel]

		ZodiacMovementComponent->SetReplicatedAcceleration(UnpackedAcceleration);
	}
}

void AZodiacCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
{
	if (GetWorld()->IsPlayingReplay())
	{
		return;
	}

	// Timestamp is checked to reject old moves.
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		// Timestamp
		ReplicatedServerLastTransformUpdateTimeStamp = SharedRepMovement.RepTimeStamp;

		// Movement mode
		if (ReplicatedMovementMode != SharedRepMovement.RepMovementMode)
		{
			ReplicatedMovementMode = SharedRepMovement.RepMovementMode;
			GetCharacterMovement()->bNetworkMovementModeChanged = true;
			GetCharacterMovement()->bNetworkUpdateReceived = true;
		}

		// Location, Rotation, Velocity, etc.
		FRepMovement& MutableRepMovement = GetReplicatedMovement_Mutable();
		MutableRepMovement = SharedRepMovement.RepMovement;

		// This also sets LastRepMovement
		OnRep_ReplicatedMovement();

		// Update Jump force
		bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;
	}
}

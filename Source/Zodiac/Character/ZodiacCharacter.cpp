// the.quiet.string@gmail.com


#include "ZodiacCharacter.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroData.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

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

AZodiacCharacter::AZodiacCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(CharacterMovementComponentName))
{
	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());
	ZodiacMoveComp->GravityScale = 1.0f;
	ZodiacMoveComp->MaxAcceleration = 800.0f;
	ZodiacMoveComp->BrakingFrictionFactor = 1.0f;
	ZodiacMoveComp->BrakingFriction = 6.0f;
	ZodiacMoveComp->GroundFriction = 5.0f;
	ZodiacMoveComp->BrakingDecelerationWalking = 750.0f;
	ZodiacMoveComp->JumpZVelocity = 500.f;
	ZodiacMoveComp->RotationRate = FRotator(0, 0, -1.f);
	ZodiacMoveComp->bUseControllerDesiredRotation = false;
	ZodiacMoveComp->bOrientRotationToMovement = false;
	ZodiacMoveComp->RotationRate = FRotator(0.0f, 720.0f, 0.0f);
	ZodiacMoveComp->bAllowPhysicsRotationDuringAnimRootMotion = false;
	ZodiacMoveComp->GetNavAgentPropertiesRef().bCanCrouch = true;
	ZodiacMoveComp->bCanWalkOffLedgesWhenCrouching = true;
	ZodiacMoveComp->SetCrouchedHalfHeight(65.0f);
}

void AZodiacCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);
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

void AZodiacCharacter::CallOrRegister_OnAbilitySystemInitialized(FOnAbilitySystemComponentInitialized::FDelegate&& Delegate)
{
	if (AbilitySystemComponent)
	{
		Delegate.Execute(AbilitySystemComponent);
	}
	else
	{
		OnAbilitySystemComponentInitialized.Add(MoveTemp(Delegate));
	}
}

void AZodiacCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
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

void AZodiacCharacter::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwner)
{
	check(InASC);

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwner, this);

	// Focus and ADS is not applied to host character.
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Death, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Movement_Disabled, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Physics_Collision_Disabled, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnPhysicsTagChanged);
	
	OnAbilitySystemComponentInitialized.Broadcast(AbilitySystemComponent);
	OnAbilitySystemComponentInitialized.Clear();

	if (CharacterData && HasAuthority())
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : CharacterData->AbilitySets)
		{
			if (AbilitySet)
			{
				AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);	
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

void AZodiacCharacter::OnMovementTagChanged(FGameplayTag Tag, int Count)
{
	if (UZodiacCharacterMovementComponent* ZodiacMoveComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		if (const uint8* CustomMovementPtr = ZodiacGameplayTags::TagCustomMovementModeMap.Find(Tag))
		{
			uint8 CustomMovement = *CustomMovementPtr;
			bool bHasTag = Count > 0;
			uint8 CustomMode = bHasTag ? CustomMovement : Move_Custom_None;
			ZodiacMoveComp->SetMovementMode(MOVE_Walking, CustomMode);
		}
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

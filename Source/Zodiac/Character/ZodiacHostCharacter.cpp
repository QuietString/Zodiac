// the.quiet.string@gmail.com

#include "ZodiacHostCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHero.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Player/ZodiacPlayerState.h"
#include "Input/ZodiacInputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostCharacter)

namespace ZodiacConsoleVariables
{
	static TAutoConsoleVariable<bool> CVarHostMovementModeLoggingEnable(
		TEXT("zodiac.HostMovement.EnableLogging"),
		false,
		TEXT("Enables log of host character movement mode"));
	
	bool LogEnabled()
	{
		return CVarHostMovementModeLoggingEnable.GetValueOnAnyThread();
	}
}

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

/////////////////////////////////////////////////////////////////
///
AZodiacHostCharacter::AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)),
	HeroList(this)
{
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

bool AZodiacHostCharacter::UpdateSharedReplication()
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

	// We cannot fastrep right now. Don't send anything.
	return false;
}

void AZodiacHostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, ReplicatedAcceleration, COND_SimulatedOnly);

	DOREPLIFETIME(ThisClass, HeroList);
	DOREPLIFETIME(ThisClass, ActiveHeroIndex);
}

void AZodiacHostCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>(GetPlayerState()))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(ZodiacPS->GetAbilitySystemComponent()))
		{
			InitializeHostAbilitySystem(ZodiacASC);
		}
	}
}

void AZodiacHostCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>(GetPlayerState()))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(ZodiacPS->GetAbilitySystemComponent()))
		{
			InitializeHostAbilitySystem(ZodiacASC);
		}
	}
}

void AZodiacHostCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
	
	if (HasAuthority())
	{
		InitializeHeroes();
	}
}

void AZodiacHostCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	ChangeHero(0);
}

void AZodiacHostCharacter::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
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

void AZodiacHostCharacter::FastSharedReplication_Implementation(const FSharedRepMovement& SharedRepMovement)
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

		// Jump force
		bProxyIsJumpForceApplied = SharedRepMovement.bProxyIsJumpForceApplied;
	}
}

void AZodiacHostCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (IsLocallyControlled())
	{ 
		InitializePlayerInput();
	}
}

void AZodiacHostCharacter::InitializePlayerInput()
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
			//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
			//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
		}
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
		{
			ZodiacASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetHeroAbilitySystemComponent())
		{
			ZodiacASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void AZodiacHostCharacter::Input_Move(const FInputActionValue& InputActionValue)
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

void AZodiacHostCharacter::Input_LookMouse(const FInputActionValue& InputActionValue)
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

void AZodiacHostCharacter::OnStatusTagChanged(FGameplayTag Tag, int Count)
{
	bool bHasTag = Count > 0;
	int32 NewCount = bHasTag ? 1 : 0;

	UZodiacHostAnimInstance* HostAnimInstance = CastChecked<UZodiacHostAnimInstance>(GetMesh()->GetAnimInstance());
	HostAnimInstance->OnStatusChanged(Tag, bHasTag);
}

void AZodiacHostCharacter::OnMovementTagChanged(FGameplayTag Tag, int Count)
{
	if (UZodiacCharacterMovementComponent* ZodiacMoveComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		uint8 CustomMode_Candidate;
		if (Tag == ZodiacGameplayTags::Movement_Mode_ADS)
		{
			CustomMode_Candidate = MOVE_ADS;
		}
		else if (Tag == ZodiacGameplayTags::Movement_Mode_Focus)
		{
			CustomMode_Candidate = MOVE_Focus;
		}
		else
		{
			CustomMode_Candidate = MOVE_None;
		}

		bool bHasTag = Count > 0;
		uint8 CustomMode = bHasTag ? CustomMode_Candidate : MOVE_None;
		ZodiacMoveComp->SetMovementMode(MOVE_Walking, CustomMode);
	}
}

void AZodiacHostCharacter::OnJustLanded()
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
	
	UAbilitySystemComponent* HeroASC = GetHeroAbilitySystemComponent();
	FGameplayEffectContextHandle ContextHandle = HeroASC->MakeEffectContext();
	FGameplayEventData Payload;
	Payload.EventTag = ZodiacGameplayTags::Event_JustLanded;
	Payload.Target = HeroASC->GetAvatarActor();
	Payload.ContextHandle = ContextHandle;
	Payload.EventMagnitude = 1;
	
	FScopedPredictionWindow NewScopedWindow(HeroASC, true);
	HeroASC->HandleGameplayEvent(Payload.EventTag, &Payload);
}

void AZodiacHostCharacter::OnJustLifted()
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

void AZodiacHostCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);
	
	UZodiacCharacterMovementComponent* ZodiacMoveComp = CastChecked<UZodiacCharacterMovementComponent>(GetCharacterMovement());

	SetMovementModeTag(PrevMovementMode, PreviousCustomMode, false);

	EMovementMode MovementMode = ZodiacMoveComp->MovementMode;
	uint8 CustomMovementMode = ZodiacMoveComp->CustomMovementMode;
	
	SetMovementModeTag(MovementMode, CustomMovementMode, true);

	if (ZodiacMoveComp->IsMovingOnGround())
	{
		if (PrevMovementMode == MOVE_Falling || PrevMovementMode == MOVE_Flying)
		{
			OnJustLanded();
		}
	}
	else
	{
		if (PrevMovementMode == MOVE_Walking || PrevMovementMode == MOVE_NavWalking)
		{
			OnJustLifted();
		}
	}

#if WITH_EDITOR
	if (ZodiacConsoleVariables::LogEnabled())
	{
		UE_LOG(LogZodiacMovement, Warning, TEXT("MovementMode: %d, Custom: %d on %s"), MovementMode, CustomMovementMode,  HasAuthority() ? TEXT("server") : TEXT("Client"));	
	}
#endif
}

void AZodiacHostCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	// MOVE_Walking and MOVE_Traversal tags applied from CharMoveComp to ASC. 
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = nullptr;

		if (CustomMovementMode == MOVE_None)
		{
			MovementModeTag = ZodiacGameplayTags::MovementModeTagMap.Find(MovementMode);
			if (MovementModeTag && MovementModeTag->IsValid())
			{
				ZodiacASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
			}
		}
		else if (CustomMovementMode == MOVE_Traversal)
		{
			MovementModeTag = ZodiacGameplayTags::CustomMovementModeTagMap.Find(CustomMovementMode);
			if (MovementModeTag && MovementModeTag->IsValid())
			{
				ZodiacASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
			}
		}
	}
}

void AZodiacHostCharacter::InitializeHostAbilitySystem(UZodiacAbilitySystemComponent* InASC)
{
	check(InASC);
	
	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Movement_Mode_ADS, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnMovementTagChanged);	
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Status_Focus, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnStatusTagChanged);	

	OnHostAbilitySystemComponentLoaded.Broadcast(AbilitySystemComponent);
	OnHostAbilitySystemComponentLoaded.Clear();
}

void AZodiacHostCharacter::CallOrRegister_OnAbilitySystemInitialized(FOnHostAbilitySystemComponentLoaded::FDelegate&& Delegate)
{
	if (AbilitySystemComponent)
	{
		Delegate.Execute(AbilitySystemComponent);
	}
	else
	{
		OnHostAbilitySystemComponentLoaded.Add(MoveTemp(Delegate));
	}
}

UAbilitySystemComponent* AZodiacHostCharacter::GetAbilitySystemComponent() const
{
	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>(GetPlayerState()))
	{
		return ZodiacPS->GetAbilitySystemComponent();
	}

	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetZodiacAbilitySystemComponent()
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		return ZodiacASC;
	}

	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetHeroAbilitySystemComponent()
{
	if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHeroAbilitySystemComponent();
	}
	
	return nullptr;
}

UZodiacHealthComponent* AZodiacHostCharacter::GetCurrentHeroHealthComponent()
{
	if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHealthComponent();
	}

	return nullptr;
}

void AZodiacHostCharacter::InitializeHeroes()
{
	for (TSubclassOf<AZodiacHero> HeroClass : HeroClasses)
	{
		if (HeroClass)
		{
			AZodiacHero* Hero = HeroList.AddEntry(HeroClass, GetWorld());
		}
	}
}

void AZodiacHostCharacter::ChangeHero(const int32 Index)
{
	if (AZodiacHero* Hero = HeroList.GetHero(Index))
	{
		int32 OldIndex = ActiveHeroIndex;
		ActiveHeroIndex = Index;
		if (OldIndex != Index)
		{
			// Locally predicted on clients.
			OnRep_ActiveHeroIndex(OldIndex);
		}
	}
}

void AZodiacHostCharacter::SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		ActiveAbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void AZodiacHostCharacter::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		ActiveAbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}



TSubclassOf<UZodiacCameraMode> AZodiacHostCharacter::DetermineCameraMode()
{
	if (ActiveAbilityCameraMode)
	{
		return ActiveAbilityCameraMode;
	}

	return DefaultAbilityCameraMode;
}

void AZodiacHostCharacter::OnRep_ReplicatedAcceleration()
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

void AZodiacHostCharacter::OnRep_ActiveHeroIndex(int32 OldIndex)
{
	if (ActiveHeroIndex != OldIndex)
	{
		if (AZodiacHero* Hero = HeroList.GetHero(OldIndex))
		{
			Hero->Deactivate();
		}

		if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
		{
			Hero->Activate();
		}	
	}
}

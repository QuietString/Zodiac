// the.quiet.string@gmail.com

#include "ZodiacHostCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHero.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Animation/ZodiacHeroAnimInstance.h"
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

AZodiacHostCharacter::AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName)),
	HeroList(this)
{
	CameraComponent = CreateDefaultSubobject<UZodiacCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
}

void AZodiacHostCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HeroList);
	DOREPLIFETIME(ThisClass, ActiveHeroIndex);
}

void AZodiacHostCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>(GetPlayerState()))
	{
		if (UAbilitySystemComponent* ASC = ZodiacPS->GetAbilitySystemComponent())
		{
			InitializeHostAbilitySystem(ASC);
		}
	}
}

void AZodiacHostCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AZodiacPlayerState* ZodiacPS = Cast<AZodiacPlayerState>(GetPlayerState()))
	{
		if (UAbilitySystemComponent* ASC = ZodiacPS->GetAbilitySystemComponent())
		{
			InitializeHostAbilitySystem(ASC);
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
		if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetZodiacAbilitySystemComponent())
		{
			ZodiacASC->AbilityInputTagPressed(InputTag);
		}
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (AZodiacHero* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = Hero->GetZodiacAbilitySystemComponent())
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

void AZodiacHostCharacter::OnAimingTagChanged(FGameplayTag Tag, int Count)
{
	if (UZodiacCharacterMovementComponent* ZodiacMoveComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		if (Count > 0)
		{
			ZodiacMoveComp->SetMovementMode(MOVE_Walking, MOVE_Aiming);
		}
		else
		{
			ZodiacMoveComp->SetMovementMode(MOVE_Walking, MOVE_None);
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

#if WITH_EDITOR
	if (ZodiacConsoleVariables::LogEnabled())
	{
		UE_LOG(LogTemp, Warning, TEXT("new movement mode: %d, %d on %s"), MovementMode, CustomMovementMode,  HasAuthority() ? TEXT("server") : TEXT("Client"));	
	}
#endif
	
	SetMovementModeTag(MovementMode, CustomMovementMode, true);
}

void AZodiacHostCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
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
	}
}

void AZodiacHostCharacter::InitializeHostAbilitySystem(UAbilitySystemComponent* InASC)
{
	check(InASC);
	
	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
	AbilitySystemComponent->RegisterGameplayTagEvent(ZodiacGameplayTags::Movement_Mode_Custom_Aiming, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ThisClass::OnAimingTagChanged);	
	
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
		return Hero->GetZodiacAbilitySystemComponent();
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
// the.quiet.string@gmail.com

#include "ZodiacHeroComponent.h"
#include "ZodiacLogChannels.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"
#include "Character/ZodiacPawnExtensionComponent.h"
#include "Character/ZodiacCharacter.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "ZodiacGameplayTags.h"
#include "Engine/LocalPlayer.h"
#include "Components/GameFrameworkComponentManager.h"

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace ZodiacHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UZodiacHeroComponent::NAME_BindInputsNow("BindInputsNow");

UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;
	
	//AbilityCameraMode = nullptr;
	bPawnHasInitialized = false;
	bReadyToBindInputs = false;
}

void UZodiacHeroComponent::OnRegister()
{
	Super::OnRegister();
	
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			PawnExtComp->RegisterAndCall_OnPawnReadyToInitialize(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnPawnReadyToInitialize));
		}
	}
	else
	{
		UE_LOG(LogZodiac, Error, TEXT("[UZodiacHeroComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("ZodiacHeroComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("ZodiacHeroComponent");
			
			FMessageLog(HeroMessageLogName).Error()
				->AddToken(FUObjectToken::Create(this, FText::FromString(GetNameSafe(this))))
				->AddToken(FTextToken::Create(Message));
				
			FMessageLog(HeroMessageLogName).Open();
		}
#endif
	}
}

bool UZodiacHeroComponent::IsPawnComponentReadyToInitialize() const
{
	// The player state is required.
	if (!GetPlayerState<AZodiacPlayerState>())
	{
		return false;
	}

	const APawn* Pawn = GetPawn<APawn>();

	// A pawn is required.
	if (!Pawn)
	{
		return false;
	}

	// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
	if (Pawn->GetLocalRole() != ROLE_SimulatedProxy)
	{
		AController* Controller = GetController<AController>();

		const bool bHasControllerPairedWithPS = (Controller != nullptr) && \
												(Controller->PlayerState != nullptr) && \
												(Controller->PlayerState->GetOwner() == Controller);

		if (!bHasControllerPairedWithPS)
		{
			return false;
		}
	}
	
	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
	const bool bIsBot = Pawn->IsBotControlled();

	if (bIsLocallyControlled && !bIsBot)
	{
		// The input component is required when locally controlled.
		if (!Pawn->InputComponent)
		{
			return false;
		}
	}

	return true;
}

void UZodiacHeroComponent::OnPawnReadyToInitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("on pawn ready to initialized"));

	if (!ensure(!bPawnHasInitialized))
	{
		// Don't initialize twice
		return;
	}

	APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}
	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

	AZodiacPlayerState* ZodiacPS = GetPlayerState<AZodiacPlayerState>();
	check(ZodiacPS);
	
	UE_LOG(LogTemp, Warning, TEXT("on pawn ready to initialized 2"));
	
	if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
		// The ability system component and attribute sets live on the player state.
		PawnExtComp->InitializeAbilitySystem(ZodiacPS->GetZodiacAbilitySystemComponent(), ZodiacPS);
	}

	// if (AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>())
	// {
	// 	if (Pawn->InputComponent != nullptr)
	// 	{
	// 		InitializePlayerInput(Pawn->InputComponent);
	// 	}
	// }

	if (bIsLocallyControlled)
	{
		// if (UZodiacCameraComponent* CameraComponent = UZodiacCameraComponent::FindCameraComponent(Pawn))
		// {
		// 	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
		// }
	}

	bPawnHasInitialized = true;
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZodiacHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// if (const APawn* Pawn = GetPawn<APawn>())
	// {
	// 	if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	// 	{
	// 		PawnExtComp->UninitializeAbilitySystem();
	// 	}	
	// }

	Super::EndPlay(EndPlayReason);
}

#if 0
void UZodiacHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();

	if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UZodiacPawnData* PawnData = PawnExtComp->GetPawnData<UZodiacPawnData>())
		{
			if (const UZodiacInputConfig* InputConfig = PawnData->InputConfig)
			{
				const FZodiacGameplayTags& GameplayTags = FZodiacGameplayTags::Get();
	
				// Register any default input configs with the settings so that they will be applied to the player during AddInputMappings
				for (const FMappableConfigPair& Pair : DefaultInputConfigs)
				{
					FMappableConfigPair::ActivatePair(Pair);
				}
				
				UZodiacInputComponent* ZodiacIC = CastChecked<UZodiacInputComponent>(PlayerInputComponent);
				ZodiacIC->AddInputMappings(InputConfig, Subsystem);
				if (UZodiacSettingsLocal* LocalSettings = UZodiacSettingsLocal::Get())
				{
					LocalSettings->OnInputConfigActivated.AddUObject(this, &UZodiacHeroComponent::OnInputConfigActivated);
					LocalSettings->OnInputConfigDeactivated.AddUObject(this, &UZodiacHeroComponent::OnInputConfigDeactivated);
				}

				TArray<uint32> BindHandles;
				ZodiacIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

				ZodiacIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
				ZodiacIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
				ZodiacIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
				ZodiacIC->BindNativeAction(InputConfig, GameplayTags.InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
				ZodiacIC->BindNativeAction(InputConfig, GameplayTags.InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
			}
		}
	}

	if (ensure(!bReadyToBindInputs))
	{
		bReadyToBindInputs = true;
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UZodiacHeroComponent::OnInputConfigActivated(const FLoadedMappableConfigPair& ConfigPair)
{
	if (AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>())
	{
		if (APawn* Pawn = GetPawn<APawn>())
		{
			if (UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(Pawn->InputComponent))
			{
				if (const ULocalPlayer* LP = ZodiacPC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						ZodiacIC->AddInputConfig(ConfigPair, Subsystem);	
					}	
				}
			}
		}
	}
}

void UZodiacHeroComponent::OnInputConfigDeactivated(const FLoadedMappableConfigPair& ConfigPair)
{
	if (AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>())
	{
		if (APawn* Pawn = GetPawn<APawn>())
		{
			if (UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(Pawn->InputComponent))
			{
				if (const ULocalPlayer* LP = ZodiacPC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						ZodiacIC->RemoveInputConfig(ConfigPair, Subsystem);
					}
				}
			}
		}
	}
}

void UZodiacHeroComponent::AddAdditionalInputConfig(const UZodiacInputConfig* InputConfig)
{
	TArray<uint32> BindHandles;

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	UZodiacInputComponent* ZodiacIC = Pawn->FindComponentByClass<UZodiacInputComponent>();
	check(ZodiacIC);

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = PC->GetLocalPlayer();
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		ZodiacIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);
	}
}

void UZodiacHeroComponent::RemoveAdditionalInputConfig(const UZodiacInputConfig* InputConfig)
{
	//@TODO: Implement me!
}
#endif

bool UZodiacHeroComponent::HasPawnInitialized() const
{
	return bPawnHasInitialized;
}

bool UZodiacHeroComponent::IsReadyToBindInputs() const
{
	return bReadyToBindInputs;
}

#if 0
void UZodiacHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = PawnExtComp->GetZodiacAbilitySystemComponent())
			{
				ZodiacASC->AbilityInputTagPressed(InputTag);
			}
		}	
	}
}

void UZodiacHeroComponent::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (UZodiacAbilitySystemComponent* ZodiacASC = PawnExtComp->GetZodiacAbilitySystemComponent())
		{
			ZodiacASC->AbilityInputTagReleased(InputTag);
		}
	}
}

void UZodiacHeroComponent::Input_Move(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();
	AController* Controller = Pawn ? Pawn->GetController() : nullptr;

	// If the player has attempted to move again then cancel auto running
	if (AZodiacPlayerController* ZodiacController = Cast<AZodiacPlayerController>(Controller))
	{
		ZodiacController->SetIsAutoRunning(false);
	}
	
	if (Controller)
	{
		const FVector2D Value = InputActionValue.Get<FVector2D>();
		const FRotator MovementRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);

		if (Value.X != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::RightVector);
			Pawn->AddMovementInput(MovementDirection, Value.X);
		}

		if (Value.Y != 0.0f)
		{
			const FVector MovementDirection = MovementRotation.RotateVector(FVector::ForwardVector);
			Pawn->AddMovementInput(MovementDirection, Value.Y);
		}
	}
}

void UZodiacHeroComponent::Input_LookMouse(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X);
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y);
	}
}

void UZodiacHeroComponent::Input_LookStick(const FInputActionValue& InputActionValue)
{
	APawn* Pawn = GetPawn<APawn>();

	if (!Pawn)
	{
		return;
	}
	
	const FVector2D Value = InputActionValue.Get<FVector2D>();

	const UWorld* World = GetWorld();
	check(World);

	if (Value.X != 0.0f)
	{
		Pawn->AddControllerYawInput(Value.X * ZodiacHero::LookYawRate * World->GetDeltaSeconds());
	}

	if (Value.Y != 0.0f)
	{
		Pawn->AddControllerPitchInput(Value.Y * ZodiacHero::LookPitchRate * World->GetDeltaSeconds());
	}
}

void UZodiacHeroComponent::Input_Crouch(const FInputActionValue& InputActionValue)
{
	if (AZodiacCharacter* Character = GetPawn<AZodiacCharacter>())
	{
		Character->ToggleCrouch();
	}
}

void UZodiacHeroComponent::Input_AutoRun(const FInputActionValue& InputActionValue)
{
	if (APawn* Pawn = GetPawn<APawn>())
	{
		if (AZodiacPlayerController* Controller = Cast<AZodiacPlayerController>(Pawn->GetController()))
		{
			// Toggle auto running
			Controller->SetIsAutoRunning(!Controller->GetIsAutoRunning());
		}	
	}
}

TSubclassOf<UZodiacCameraMode> UZodiacHeroComponent::DetermineCameraMode() const
{
	if (AbilityCameraMode)
	{
		return AbilityCameraMode;
	}

	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return nullptr;
	}

	if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UZodiacPawnData* PawnData = PawnExtComp->GetPawnData<UZodiacPawnData>())
		{
			return PawnData->DefaultCameraMode;
		}
	}

	return nullptr;
}

void UZodiacHeroComponent::SetAbilityCameraMode(TSubclassOf<UZodiacCameraMode> CameraMode, const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (CameraMode)
	{
		AbilityCameraMode = CameraMode;
		AbilityCameraModeOwningSpecHandle = OwningSpecHandle;
	}
}

void UZodiacHeroComponent::ClearAbilityCameraMode(const FGameplayAbilitySpecHandle& OwningSpecHandle)
{
	if (AbilityCameraModeOwningSpecHandle == OwningSpecHandle)
	{
		AbilityCameraMode = nullptr;
		AbilityCameraModeOwningSpecHandle = FGameplayAbilitySpecHandle();
	}
}
#endif

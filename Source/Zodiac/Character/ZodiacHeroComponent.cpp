// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacHeroComponent.h"
#include "Components/GameFrameworkComponentDelegates.h"
#include "Logging/MessageLog.h"
#include "ZodiacLogChannels.h"
#include "EnhancedInputSubsystems.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"
#include "Player/ZodiacLocalPlayer.h"
#include "Character/ZodiacPawnExtensionComponent.h"
#include "Character/ZodiacPawnData.h"
#include "Character/ZodiacCharacter.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Input/ZodiacInputConfig.h"
#include "Input/ZodiacInputComponent.h"
#include "ZodiacGameplayTags.h"
#include "Components/GameFrameworkComponentManager.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "InputMappingContext.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroComponent)

#if WITH_EDITOR
#include "Misc/UObjectToken.h"
#endif	// WITH_EDITOR

namespace ZodiacHero
{
	static const float LookYawRate = 300.0f;
	static const float LookPitchRate = 165.0f;
};

const FName UZodiacHeroComponent::NAME_BindInputsNow("BindInputsNow");
const FName UZodiacHeroComponent::NAME_ActorFeatureName("Hero");

UZodiacHeroComponent::UZodiacHeroComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	//AbilityCameraMode = nullptr;
}

void UZodiacHeroComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
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
	else
	{
		// Register with the init state system early, this will only work if this is a game world
		RegisterInitStateFeature();
	}
}

bool UZodiacHeroComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	APawn* Pawn = GetPawn<APawn>();

	if (!CurrentState.IsValid() && DesiredState == ZodiacGameplayTags::InitState_Spawned)
	{
		// As long as we have a real pawn, let us transition
		if (Pawn)
		{
			UE_LOG(LogTemp, Warning, TEXT("have pawn"));
			return true;
		}
	}
	else if (CurrentState == ZodiacGameplayTags::InitState_Spawned && DesiredState == ZodiacGameplayTags::InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<AZodiacPlayerState>())
		{
			UE_LOG(LogTemp, Warning, TEXT("no ps"));
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
				UE_LOG(LogTemp, Warning, TEXT("simul"));
				return false;
			}
		}

		const bool bIsLocallyControlled = Pawn->IsLocallyControlled();
		const bool bIsBot = Pawn->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>();

			// The input component and local player is required when locally controlled.
			if (!Pawn->InputComponent || !ZodiacPC || !ZodiacPC->GetLocalPlayer())
			{
				UE_LOG(LogTemp, Warning, TEXT("no ic or pc or lp"));
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == ZodiacGameplayTags::InitState_DataAvailable && DesiredState == ZodiacGameplayTags::InitState_DataInitialized)
	{
		// Wait for player state and extension component
		AZodiacPlayerState* ZodiacPS = GetPlayerState<AZodiacPlayerState>();
		UE_LOG(LogTemp, Warning, TEXT("featur reached"));
		return ZodiacPS && Manager->HasFeatureReachedInitState(Pawn, UZodiacPawnExtensionComponent::NAME_ActorFeatureName, ZodiacGameplayTags::InitState_DataInitialized);
	}
	else if (CurrentState == ZodiacGameplayTags::InitState_DataInitialized && DesiredState == ZodiacGameplayTags::InitState_GameplayReady)
	{
		// TODO add ability initialization checks?
		return true;
	}

	return false;
}

void UZodiacHeroComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	if (CurrentState == ZodiacGameplayTags::InitState_DataAvailable && DesiredState == ZodiacGameplayTags::InitState_DataInitialized)
	{
		UE_LOG(LogTemp, Warning, TEXT("handle change init state"));

		APawn* Pawn = GetPawn<APawn>();
		AZodiacPlayerState* ZodiacPS = GetPlayerState<AZodiacPlayerState>();
		if (!ensure(Pawn && ZodiacPS))
		{
			UE_LOG(LogTemp, Warning, TEXT("handle change init state: no ps"));

			return;
		}

		const UZodiacPawnData* PawnData = nullptr;

		if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("handle change init state: have pawnext"));

			PawnData = PawnExtComp->GetPawnData<UZodiacPawnData>();

			// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
			// The ability system component and attribute sets live on the player state.
			PawnExtComp->InitializeAbilitySystem(ZodiacPS->GetZodiacAbilitySystemComponent(), ZodiacPS);
		}
		if (AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>())
		{
			UE_LOG(LogTemp, Warning, TEXT("handle change init state: have pc"));

			if (Pawn->InputComponent != nullptr)
			{
				InitializePlayerInput(Pawn->InputComponent);
			}
			UE_LOG(LogTemp, Warning, TEXT("handle change init state: no input comp"));

		}

		// @TODO: Hook up the delegate for all pawns, in case we spectate later
		// if (PawnData)
		// {
		// 	if (UZodiacCameraComponent* CameraComponent = UZodiacCameraComponent::FindCameraComponent(Pawn))
		// 	{
		// 		CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);
		// 	}
		// }
	}
}

void UZodiacHeroComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UZodiacPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == ZodiacGameplayTags::InitState_DataInitialized)
		{
			// If the extension component says all all other components are initialized, try to progress to next state
			CheckDefaultInitialization();
		}
	}
}

void UZodiacHeroComponent::CheckDefaultInitialization()
{
	static const TArray<FGameplayTag> StateChain = { ZodiacGameplayTags::InitState_Spawned, ZodiacGameplayTags::InitState_DataAvailable, ZodiacGameplayTags::InitState_DataInitialized, ZodiacGameplayTags::InitState_GameplayReady };
	UE_LOG(LogTemp, Warning, TEXT("check default init"));
	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UZodiacHeroComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UZodiacPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(ZodiacGameplayTags::InitState_Spawned));
	CheckDefaultInitialization();
}

void UZodiacHeroComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UZodiacHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	UE_LOG(LogTemp, Warning, TEXT("init player input"));
	const APawn* Pawn = GetPawn<APawn>();
	if (!Pawn)
	{
		return;
	}

	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const UZodiacLocalPlayer* LP = Cast<UZodiacLocalPlayer>(PC->GetLocalPlayer());
	check(LP);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);

	Subsystem->ClearAllMappings();
	UE_LOG(LogTemp, Warning, TEXT("init player input2"));
	if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		if (const UZodiacPawnData* PawnData = PawnExtComp->GetPawnData<UZodiacPawnData>())
		{
			UE_LOG(LogTemp, Warning, TEXT("init player input3"));
			if (const UZodiacInputConfig* InputConfig = PawnData->InputConfig)
			{
				for (const FInputMappingContextAndPriority& Mapping : DefaultInputMappings)
				{
					if (UInputMappingContext* IMC = Mapping.InputMapping)
					{
						if (Mapping.bRegisterWithSettings)
						{
							if (UEnhancedInputUserSettings* Settings = Subsystem->GetUserSettings())
							{
								UE_LOG(LogTemp, Warning, TEXT("player input registered"));
								Settings->RegisterInputMappingContext(IMC);
							}
							
							FModifyContextOptions Options = {};
							Options.bIgnoreAllPressedKeysUntilRelease = false;
							// Actually add the config to the local player							
							Subsystem->AddMappingContext(IMC, Mapping.Priority, Options);
						}
					}
				}

				// The Zodiac Input Component has some additional functions to map Gameplay Tags to an Input Action.
				// If you want this functionality but still want to change your input component class, make it a subclass
				// of the UZodiacInputComponent or modify this component accordingly.
				UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(PlayerInputComponent);
				if (ensureMsgf(ZodiacIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UZodiacInputComponent or a subclass of it.")))
				{
					// Add the key mappings that may have been set by the player
					ZodiacIC->AddInputMappings(InputConfig, Subsystem);

					// This is where we actually bind and input action to a gameplay tag, which means that Gameplay Ability Blueprints will
					// be triggered directly by these input actions Triggered events. 
					TArray<uint32> BindHandles;
					ZodiacIC->BindAbilityActions(InputConfig, this, &ThisClass::Input_AbilityInputTagPressed, &ThisClass::Input_AbilityInputTagReleased, /*out*/ BindHandles);

					ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Move, ETriggerEvent::Triggered, this, &ThisClass::Input_Move, /*bLogIfNotFound=*/ false);
					ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Look_Mouse, ETriggerEvent::Triggered, this, &ThisClass::Input_LookMouse, /*bLogIfNotFound=*/ false);
					ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Look_Stick, ETriggerEvent::Triggered, this, &ThisClass::Input_LookStick, /*bLogIfNotFound=*/ false);
					ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
					ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);
				}
			}
		}
	}
 
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APlayerController*>(PC), NAME_BindInputsNow);
	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(const_cast<APawn*>(Pawn), NAME_BindInputsNow);
}

void UZodiacHeroComponent::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (const APawn* Pawn = GetPawn<APawn>())
	{
		if (const UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
		{
			if (UZodiacAbilitySystemComponent* ZodiacASC = PawnExtComp->GetZodiacAbilitySystemComponent())
			{
				// @TODO: ZodiacASC->AbilityInputTagPressed(InputTag);
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
			// @TODO: ZodiacASC->AbilityInputTagReleased(InputTag);
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

#if 0
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
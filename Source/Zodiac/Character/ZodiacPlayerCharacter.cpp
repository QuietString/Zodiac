// the.quiet.string@gmail.com


#include "ZodiacPlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroComponent.h"
#include "ZodiacInputData.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Input/ZodiacInputComponent.h"
#include "Net/UnrealNetwork.h"

AZodiacPlayerCharacter::AZodiacPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bStartWithTickEnabled = true;
	ActiveHeroIndex = INDEX_NONE;

	HeroComponent1 = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroComponent>(this, TEXT("HeroComponent1"));
	HeroComponent2 = ObjectInitializer.CreateDefaultSubobject<UZodiacHeroComponent>(this, TEXT("HeroComponent2"));
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
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex) && bInitialized)
	{
		return AbilitySystemComponents[ActiveHeroIndex];
	}

	return nullptr;
}

UAbilitySystemComponent* AZodiacPlayerCharacter::GetAbilitySystemComponent() const
{
	if (AbilitySystemComponents.IsValidIndex(ActiveHeroIndex) && bInitialized)
	{
		return AbilitySystemComponents[ActiveHeroIndex];
	}

	return nullptr;
}

void AZodiacPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning,TEXT("Begin Play"));

	InitializeHeroComponents();
	SelectFirstHero();
}

void AZodiacPlayerCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

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

void AZodiacPlayerCharacter::ChangeMesh(USkeletalMesh* NewMesh, const TSubclassOf<UAnimInstance> NewAnimInstance)
{
	if (USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		MeshComponent->SetSkeletalMeshAsset(NewMesh);
		MeshComponent->SetAnimInstanceClass(NewAnimInstance);	
	}
}

void AZodiacPlayerCharacter::ChangeHero(int32 NewHeroIndex)
{
	if (HeroComponents.IsValidIndex(NewHeroIndex))
	{
		if (ActiveHeroIndex != NewHeroIndex)
		{
			HeroComponents[ActiveHeroIndex]->DeactivateHero();
			HeroComponents[NewHeroIndex]->ActivateHero();

			ActiveHeroIndex = NewHeroIndex;
		}	
	}
}

void AZodiacPlayerCharacter::SetActiveHeroIndex(int32 NewIndex)
{
	UE_LOG(LogTemp, Warning, TEXT("set active index as %d"), NewIndex);
	int32 OldIndex = ActiveHeroIndex;
	ActiveHeroIndex = NewIndex;

	if (HasAuthority())
	{
		OnRep_ActiveHeroIndex(OldIndex);
	}
}

void AZodiacPlayerCharacter::InitializeHeroComponents()
{
	HeroComponents.Reset(2);
	AbilitySystemComponents.Reset(2);

	HeroComponents.Add(HeroComponent1);
	UZodiacAbilitySystemComponent* HeroASC1 = HeroComponent1->InitializeAbilitySystemComponent();
	check(HeroASC1);
	AbilitySystemComponents.Add(HeroASC1);

	UE_LOG(LogTemp, Warning, TEXT("hero actor components added for %s"), *HeroComponent1->HeroName.ToString());
	
	HeroComponents.Add(HeroComponent2);
	UZodiacAbilitySystemComponent* HeroASC2 = HeroComponent2->InitializeAbilitySystemComponent();
	check(HeroASC2);
	AbilitySystemComponents.Add(HeroASC2);

	UE_LOG(LogTemp, Warning, TEXT("hero actor components added for %s"), *HeroComponent2->HeroName.ToString());

	bInitialized = true;
}

void AZodiacPlayerCharacter::SelectFirstHero()
{
	HeroComponent1->DeactivateHero();
	HeroComponent2->DeactivateHero();
	
	if (HasAuthority())
	{
		ActiveHeroIndex = 0;
		OnRep_ActiveHeroIndex(INDEX_NONE);
	}
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
		
		if (const UZodiacInputConfig* InputConfig = InputData->InputConfig)
		{
			// The Zodiac Input Component has some additional functions to map Gameplay Tags to an Input Action.
			// If you want this functionality but still want to change your input component class, make it a subclass
			// of the UZodiacInputComponent or modify this component accordingly.
			UZodiacInputComponent* ZodiacIC = Cast<UZodiacInputComponent>(InputComponent);
			if (ensureMsgf(ZodiacIC, TEXT("Unexpected Input Component class! The Gameplay Abilities will not be bound to their inputs. Change the input component to UZodiacInputComponent or a subclass of it.")))
			{
				// Add the key mappings that may have been set by the player
				ZodiacIC->AddInputMappings(InputConfig, Subsystem);

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
	UE_LOG(LogTemp, Warning, TEXT("OnRep NewIndex: %d, OldIndex: %d on NetMode: %d"), ActiveHeroIndex, OldIndex, GetNetMode());
	
	if (ActiveHeroIndex != OldIndex)
	{
		if (HeroComponents.IsValidIndex(OldIndex))
		{
			HeroComponents[OldIndex]->DeactivateHero();
		}

		if(HeroComponents.IsValidIndex(ActiveHeroIndex))
		{
			HeroComponents[ActiveHeroIndex]->ActivateHero();
		}
	}
}
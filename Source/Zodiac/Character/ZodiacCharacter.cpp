// the.quiet.string@gmail.com

#include "ZodiacCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroData.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ZodiacCharacterChangeComponent.h"
#include "ZodiacInputData.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacCombatSet.h"
#include "AbilitySystem/Attributes/ZodiacHealthSet.h"
#include "Input/ZodiacInputComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacCharacter)

AZodiacCharacter::AZodiacCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacAbilitySystemComponent>(this, TEXT("AbilitySysteComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	CharacterChangeComponent = ObjectInitializer.CreateDefaultSubobject<UZodiacCharacterChangeComponent>(this, TEXT("CharacterChangeComponent"));
	CharacterChangeComponent->SetIsReplicated(true);
	
	RetargetedMeshComponent = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("RetargetedMeshComponent"));
	RetargetedMeshComponent->SetupAttachment(GetMesh(), NAME_None);
	RetargetedMeshComponent->SetIsReplicated(true);
	
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

	HealthSet = CreateDefaultSubobject<UZodiacHealthSet>(TEXT("HealthSet"));
	
	HealthComponent = CreateDefaultSubobject<UZodiacHealthComponent>(TEXT("HealthComponent"));
}

void AZodiacCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	InitializeAbilitySystemComponent();
}

UZodiacAbilitySystemComponent* AZodiacCharacter::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(AbilitySystemComponent);
}

UAbilitySystemComponent* AZodiacCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AZodiacCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	AbilitySystemComponent->AbilityInputTagPressed(InputTag);
}

void AZodiacCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	AbilitySystemComponent->AbilityInputTagReleased(InputTag);
}

void AZodiacCharacter::Input_Move(const FInputActionValue& InputActionValue)
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

void AZodiacCharacter::Input_Crouch(const FInputActionValue& InputActionValue)
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

void AZodiacCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Crouching, 1);
	}
	
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

void AZodiacCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_Crouching, 0);
	}
	
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);
}

bool AZodiacCharacter::CanJumpInternal_Implementation() const
{
	// same as ACharacter's implementation but without the crouch check
	return JumpIsAllowedInternal();
}

void AZodiacCharacter::Input_ChangeCharacter(const int32 NewSlotIndex, const FGameplayTag SlotActionTag)
{
	FGameplayEventData EventData;
	EventData.EventMagnitude = NewSlotIndex;
	
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, SlotActionTag, EventData);
}

USkeletalMeshComponent* AZodiacCharacter::GetRetargetedMeshComponent()
{
	return RetargetedMeshComponent;
}

UZodiacCharacterChangeComponent* AZodiacCharacter::GetCharacterChangeComponent()
{
	return CharacterChangeComponent;
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

void AZodiacCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->GetOwnedGameplayTags(TagContainer);	
	}
}

bool AZodiacCharacter::HasMatchingGameplayTag(FGameplayTag TagToCheck) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasMatchingGameplayTag(TagToCheck);	
	}

	return false;
}

bool AZodiacCharacter::HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAllMatchingGameplayTags(TagContainer);	
	}
	
	return false;
}

bool AZodiacCharacter::HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const
{
	if (AbilitySystemComponent)
	{
		return AbilitySystemComponent->HasAnyMatchingGameplayTags(TagContainer);
	}

	return false;
}


TArray<UZodiacHeroData*> AZodiacCharacter::GetHeroes()
{
	return Heroes;
}

void AZodiacCharacter::AddStartingAbilities()
{
	if (StartingAbilities.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : StartingAbilities)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}
}

void AZodiacCharacter::OnManaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("Mana changed from %.1f to %.1f"), OldValue, NewValue);
}

void AZodiacCharacter::InitializeAbilitySystemComponent()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AddStartingAbilities();
	
	HealthComponent->InitializeWithAbilitySystem(AbilitySystemComponent);
	
	if (AbilitySystemComponent->GetAttributeSet(UZodiacCombatSet::StaticClass()))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UZodiacCombatSet::GetManaAttribute()).AddUObject(this, &ThisClass::OnManaChanged);
	}
}
// the.quiet.string@gmail.com


#include "ZodiacCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHealthComponent.h"
#include "ZodiacHeroComponent.h"
#include "ZodiacLogChannels.h"
#include "ZodiacPawnData.h"
#include "ZodiacPawnExtensionComponent.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ZodiacCombatSet.h"
#include "Input/ZodiacInputComponent.h"
#include "Player/ZodiacPlayerState.h"


AZodiacCharacter::AZodiacCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

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
	
	PawnExtComponent = CreateDefaultSubobject<UZodiacPawnExtensionComponent>(TEXT("PawnExtensionComponent"));
	PawnExtComponent->RegisterAndCall_OnAbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAbilitySystemInitialized));

	HealthComponent = CreateDefaultSubobject<UZodiacHealthComponent>(TEXT("HealthComponent"));

	HeroComponent = CreateDefaultSubobject<UZodiacHeroComponent>(TEXT("HeroComponent"));
}

AZodiacPlayerState* AZodiacCharacter::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(GetPlayerState(), ECastCheckedType::NullAllowed);
}

UZodiacAbilitySystemComponent* AZodiacCharacter::GetZodiacAbilitySystemComponent() const
{
	return Cast<UZodiacAbilitySystemComponent>(GetAbilitySystemComponent());
}

void AZodiacCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializePlayerInput();
}

void AZodiacCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UE_LOG(LogTemp, Warning, TEXT("jump input pressed by tag"));
	GetZodiacAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
}

void AZodiacCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	GetZodiacAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
}

void AZodiacCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	PawnExtComponent->CheckPawnReadyToInitialize();
}

void AZodiacCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	PawnExtComponent->CheckPawnReadyToInitialize();
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

void AZodiacCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AZodiacCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AZodiacCharacter::InitializePlayerInput()
{
	const APlayerController* PC = GetController<APlayerController>();
	check(PC);

	const ULocalPlayer* LP = Cast<ULocalPlayer>(PC->GetLocalPlayer());
	check(LP);
	
	UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	check(Subsystem);
	
	if (PawnData)
	{
		// Add input mapping context (player input to input action)
		if (PawnData->InputMappingContexts.Num() > 0)
		{
			for (UInputMappingContext* IMC : PawnData->InputMappingContexts)
			{
				FModifyContextOptions Options = {};
				Options.bIgnoreAllPressedKeysUntilRelease = true;
				Subsystem->AddMappingContext(IMC, 0, Options);
			}
		}
		
		if (const UZodiacInputConfig* InputConfig = PawnData->InputConfig)
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
				//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_Crouch, ETriggerEvent::Triggered, this, &ThisClass::Input_Crouch, /*bLogIfNotFound=*/ false);
				//ZodiacIC->BindNativeAction(InputConfig, ZodiacGameplayTags::InputTag_AutoRun, ETriggerEvent::Triggered, this, &ThisClass::Input_AutoRun, /*bLogIfNotFound=*/ false);

				UE_LOG(LogTemp, Warning, TEXT("input config added"));
			}
		}
	}
}

UAbilitySystemComponent* AZodiacCharacter::GetAbilitySystemComponent() const
{
	AZodiacPlayerState* ZodiacPS = GetZodiacPlayerState();
	return ZodiacPS->GetZodiacAbilitySystemComponent();
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

void AZodiacCharacter::AddDefaultAbilities(UZodiacAbilitySystemComponent* ZodiacASC)
{
	if (PawnData && PawnData->DefaultAbilities.Num() > 0)
	{
		for (TObjectPtr<UZodiacAbilitySet> AbilitySet : PawnData->DefaultAbilities)
		{
			AbilitySet->GiveToAbilitySystem(ZodiacASC, nullptr);
		}
	}
}

void AZodiacCharacter::OnManaChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	float NewValue = OnAttributeChangeData.NewValue;
	float OldValue = OnAttributeChangeData.OldValue;

	UE_LOG(LogTemp, Warning, TEXT("Mana changed from %.1f to %.1f"), OldValue, NewValue);
}

void AZodiacCharacter::OnAbilitySystemInitialized()
{
	UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent();
	check(ZodiacASC);

	AddDefaultAbilities(ZodiacASC);
	
	HealthComponent->InitializeWithAbilitySystem(ZodiacASC);

	
	if (ZodiacASC->GetAttributeSet(UZodiacCombatSet::StaticClass()))
	{
		ZodiacASC->GetGameplayAttributeValueChangeDelegate(UZodiacCombatSet::GetManaAttribute()).AddUObject(this, &ThisClass::OnManaChanged);
	}
}

void AZodiacCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	PawnExtComponent->CheckPawnReadyToInitialize();
}

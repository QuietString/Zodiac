// the.quiet.string@gmail.com

#include "ZodiacHostCharacter.h"

#include "DisplayDebugHelpers.h"
#include "ZodiacCharacterMovementComponent.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroCharacter.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Engine/Canvas.h"
#include "Player/ZodiacPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostCharacter)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_INPUTTAG_HERO, "InputTag.Hero");

AZodiacHostCharacter::AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
	HeroList(this)
{
	CameraComponent = CreateDefaultSubobject<UZodiacCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(-300.0f, 0.0f, 75.0f));
}

void AZodiacHostCharacter::ToggleSprint(const bool bShouldSprint)
{
	if (UZodiacCharacterMovementComponent* ZodiacCharMoveComp = Cast<UZodiacCharacterMovementComponent>(GetCharacterMovement()))
	{
		ZodiacCharMoveComp->ToggleSprint(bShouldSprint);
	}
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
		if (UZodiacAbilitySystemComponent* ZodiacASC = Cast<UZodiacAbilitySystemComponent>(ZodiacPS->GetAbilitySystemComponent()))
		{
			InitializeAbilitySystem(ZodiacASC, ZodiacPS);
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
			InitializeAbilitySystem(ZodiacASC, ZodiacPS);
		}
	}
}

void AZodiacHostCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	CameraComponent->DetermineCameraModeDelegate.BindUObject(this, &ThisClass::DetermineCameraMode);

	if (CameraComponent->bApplyTranslationOffset)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			if (UZodiacHostAnimInstance* HostAnimInstance = Cast<UZodiacHostAnimInstance>(AnimInstance))
			{
				CameraComponent->UpdateCameraTranslationOffsetDelegate.BindUObject(HostAnimInstance, &UZodiacHostAnimInstance::GetTranslationOffset);
			}
		}	
	}
	
	if (HasAuthority())
	{
		InitializeHeroes();
	}
}

void AZodiacHostCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		ChangeHero(0);	
	}
}

void AZodiacHostCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);

	if (DebugDisplay.IsDisplayOn(FName("HostCharacter")))
	{
		if (IsLocallyControlled())
		{
			FDisplayDebugManager& DisplayDebugManager = Canvas->DisplayDebugManager;

			DisplayDebugManager.SetFont(GEngine->GetSmallFont());
			DisplayDebugManager.SetDrawColor(FColor::Green);

			{
				FGameplayTagContainer Container;
				GetHostAbilitySystemComponent()->GetOwnedGameplayTags(Container);
				TArray<FGameplayTag> TagsInArray;
				Container.GetGameplayTagArray(TagsInArray);
				FString AppendedTags = FString("Host Tags: ");
		
				for (auto& Tag : TagsInArray)
				{
					AppendedTags.Append(FString::Printf(TEXT("%s, "), *Tag.GetTagName().ToString()));
				}

				DisplayDebugManager.DrawString(AppendedTags);
			}
		
			{
				FGameplayTagContainer Container;
				GetHeroAbilitySystemComponent()->GetOwnedGameplayTags(Container);
				TArray<FGameplayTag> TagsInArray;
				Container.GetGameplayTagArray(TagsInArray);
				FString AppendedTags = FString("Hero Tags: ");
		
				for (auto& Tag : TagsInArray)
				{
					AppendedTags.Append(FString::Printf(TEXT("%s, "), *Tag.GetTagName().ToString()));
				}
				DisplayDebugManager.DrawString(AppendedTags);
			}
		}
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UZodiacAbilitySystemComponent* ZodiacASC = InputTag.MatchesTag(TAG_INPUTTAG_HERO) ? GetHeroAbilitySystemComponent() : GetHostAbilitySystemComponent();

	if (ZodiacASC)
	{
		ZodiacASC->AbilityInputTagPressed(InputTag);
		const TArray<FGameplayAbilitySpec> Specs = ZodiacASC->GetActivatableAbilities();
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	UZodiacAbilitySystemComponent* ZodiacASC = InputTag.MatchesTag(TAG_INPUTTAG_HERO) ? GetHeroAbilitySystemComponent() : GetHostAbilitySystemComponent();

	if (ZodiacASC)
	{
		ZodiacASC->AbilityInputTagReleased(InputTag);
	}
}

UAbilitySystemComponent* AZodiacHostCharacter::GetAbilitySystemComponent() const
{
	return GetZodiacAbilitySystemComponent();
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetZodiacAbilitySystemComponent() const
{
	if (AZodiacHeroCharacter* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHeroAbilitySystemComponent();
	}
	
	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetHostAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AZodiacHostCharacter::SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled)
{
	// The Same as AZodiacCharacter, just applies to host asc instead.
	if (UAbilitySystemComponent* ASC = GetHostAbilitySystemComponent())
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

void AZodiacHostCharacter::SetExtendedMovementModeTag(EZodiacExtendedMovementMode ExtendedMovementMode, bool bTagEnabled)
{
	// The Same as AZodiacCharacter, just applies to host asc instead.
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetHostAbilitySystemComponent())
	{
		const FGameplayTag* MovementModeTag = ZodiacGameplayTags::ExtendedMovementModeTagMap.Find(ExtendedMovementMode);
		
		if (MovementModeTag && MovementModeTag->IsValid())
		{
			ZodiacASC->SetLooseGameplayTagCount(*MovementModeTag, (bTagEnabled ? 1 : 0));
		}
	}
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetHeroAbilitySystemComponent() const
{
	if (AZodiacHeroCharacter* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHeroAbilitySystemComponent();
	}
	
	return nullptr;
}

USkeletalMeshComponent* AZodiacHostCharacter::GetRetargetedMesh() const
{
	if (AZodiacHeroCharacter* Hero = GetHero())
	{
		return Hero->GetMesh();
	}

	return nullptr;
}

UZodiacHealthComponent* AZodiacHostCharacter::GetHealthComponent() const
{
	if (AZodiacHeroCharacter* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHealthComponent();
	}

	return nullptr;
}

UAbilitySystemComponent* AZodiacHostCharacter::GetTraversalAbilitySystemComponent() const
{
	return GetHostAbilitySystemComponent();
}

void AZodiacHostCharacter::InitializeHeroes()
{
	int i = 0;
	for (TSubclassOf<AZodiacHeroCharacter> HeroClass : HeroClasses)
	{
		if (HeroClass)
		{
			AZodiacHeroCharacter* Hero = HeroList.AddEntry(GetWorld(), HeroClass, i++);
		}
	}
}

void AZodiacHostCharacter::ChangeHero(const int32 Index)
{
	if (AZodiacHeroCharacter* Hero = HeroList.GetHero(Index))
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

void AZodiacHostCharacter::CheckAllHeroesInitialized()
{
	if (bHasHeroInitialized)
	{
		// Already initialized.
		return;
	}
	
	if (GetHeroes().Num() != HeroClasses.Num())
	{
		// Heroes not replicated yet.
		return;
	}
	
	for (auto& Hero : GetHeroes())
	{
		if (!Hero->GetIsInitialized())
		{
			// A hero is not initialized yet.
			return;
		}
	}
	
	OnAllHeroesInitialized();
	bHasHeroInitialized = true;
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

void AZodiacHostCharacter::UpdateHeroEyeLocationOffset()
{
	if (AZodiacHeroCharacter* Hero = GetHero())
	{
		FVector HeroLocation = Hero->GetPawnViewLocation();
		FVector HostLocation = GetPawnViewLocation();

		HeroEyeLocationOffset = HeroLocation - HostLocation;
	}
}

void AZodiacHostCharacter::OnRep_ActiveHeroIndex(int32 OldIndex)
{
	if (ActiveHeroIndex != OldIndex)
	{
		if (AZodiacHeroCharacter* Hero = HeroList.GetHero(OldIndex))
		{
			Hero->Deactivate();
		}

		if (AZodiacHeroCharacter* Hero = HeroList.GetHero(ActiveHeroIndex))
		{
			Hero->Activate();

			if (bEnableCameraHeroOffset)
			{
				UpdateHeroEyeLocationOffset();				
			}
		}	
	}
}

void AZodiacHostCharacter::OnRep_HeroList()
{
	CheckAllHeroesInitialized();
}

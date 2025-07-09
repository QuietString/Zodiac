// the.quiet.string@gmail.com


#include "ZodiacHostCharacter.h"

#include "DisplayDebugHelpers.h"
#include "Character/ZodiacCharacterType.h"
#include "ZodiacGameplayTags.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Character/Hero/ZodiacHeroActor.h"
#include "Character/ZodiacHealthComponent.h"
#include "ZodiacHostExtensionComponent.h"
#include "ZodiacHostFeatureComponent.h"
#include "AbilitySystem/Host/ZodiacHostAbilitySystemComponent.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Engine/Canvas.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostCharacter)

AZodiacHostCharacter::AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UZodiacHostExtensionComponent>(ExtensionComponentName))
	, HeroList(this)
{
	HostFeatureComponent = CreateDefaultSubobject<UZodiacHostFeatureComponent>(TEXT("HostFeatureComponent"));
}

UAbilitySystemComponent* AZodiacHostCharacter::GetAbilitySystemComponent() const
{
	if (AZodiacHeroActor* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetZodiacAbilitySystemComponent();
	}
	
	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetZodiacAbilitySystemComponent() const
{
	if (AZodiacHeroActor* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetZodiacAbilitySystemComponent();
	}
	
	return nullptr;
}

UZodiacHealthComponent* AZodiacHostCharacter::GetHealthComponent() const
{
	if (AZodiacHeroActor* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetHealthComponent();
	}

	return nullptr;
}

USkeletalMeshComponent* AZodiacHostCharacter::GetRetargetedMesh() const
{
	if (AZodiacHeroActor* Hero = GetHero())
	{
		return Hero->GetMesh();
	}

	return nullptr;
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

UZodiacHostAbilitySystemComponent* AZodiacHostCharacter::GetHostAbilitySystemComponent() const
{
	if (UZodiacPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
	{
		return Cast<UZodiacHostAbilitySystemComponent>(PawnExtensionComponent->GetZodiacAbilitySystemComponent());
	}
	
	return nullptr;
}

UZodiacAbilitySystemComponent* AZodiacHostCharacter::GetHeroAbilitySystemComponent() const
{
	if (AZodiacHeroActor* Hero = HeroList.GetHero(ActiveHeroIndex))
	{
		return Hero->GetZodiacAbilitySystemComponent();
	}
	
	return nullptr;
}

UAbilitySystemComponent* AZodiacHostCharacter::GetTraversalAbilitySystemComponent() const
{
	return GetHostAbilitySystemComponent();
}

void AZodiacHostCharacter::PossessedBy(class AController* NewController)
{
	Super::PossessedBy(NewController);
}

void AZodiacHostCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void AZodiacHostCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ActiveHeroIndex);
	DOREPLIFETIME(ThisClass, HeroList);
}

void AZodiacHostCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!GetWorld() || !GetWorld()->IsGameWorld())
	{
		return;
	}

	GetMesh()->SetVisibility(false);
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

void AZodiacHostCharacter::SpawnHeroes()
{
	APlayerController* PC = GetOwner<APlayerController>();
	check(PC)
	
	HeroList.SetOwner(PC);
	
	int i = 0;
	for (auto& HeroClass : HeroClasses)
	{
		if (HeroClass)
		{
			AZodiacHeroActor* Hero = HeroList.AddEntry(GetWorld(), this, HeroClass, i++);
		}
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	UZodiacAbilitySystemComponent* ZodiacASC = InputTag.MatchesTag(ZodiacGameplayTags::InputTag_Hero) ? GetHeroAbilitySystemComponent() : GetHostAbilitySystemComponent();

	if (ZodiacASC)
	{
		ZodiacASC->AbilityInputTagPressed(InputTag);
	}
}

void AZodiacHostCharacter::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	UZodiacAbilitySystemComponent* ZodiacASC = InputTag.MatchesTag(ZodiacGameplayTags::InputTag_Hero) ? GetHeroAbilitySystemComponent() : GetHostAbilitySystemComponent();

	if (ZodiacASC)
	{
		ZodiacASC->AbilityInputTagReleased(InputTag);
	}
}

void AZodiacHostCharacter::ChangeHero(const int32 Index)
{
	if (AZodiacHeroActor* Hero = HeroList.GetHero(Index))
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

float AZodiacHostCharacter::GetAimYaw() const
{
	if (UZodiacHostAnimInstance* HostAnimInstance = Cast<UZodiacHostAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		return HostAnimInstance->AimYaw;
	}

	return 0.0f;
}

void AZodiacHostCharacter::UpdateHeroEyeLocationOffset()
{
	// if (AZodiacHeroActor* Hero = GetHero())
	// {
	// 	FVector HeroLocation = Hero->GetPawnViewLocation();
	// 	FVector HostLocation = GetPawnViewLocation();
	//
	// 	HeroEyeLocationOffset = HeroLocation - HostLocation;
	// }
}

void AZodiacHostCharacter::OnAbilitySystemInitialized()
{
	InitializeGameplayTags();
}

void AZodiacHostCharacter::OnAbilitySystemUninitialized()
{
}

void AZodiacHostCharacter::InitializeGameplayTags()
{
	// Clear tags that may be lingering on the ability system from the previous pawn.
	if (UAbilitySystemComponent* ZodiacASC = GetHostAbilitySystemComponent())
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

void AZodiacHostCharacter::OnRep_ActiveHeroIndex(int32 OldIndex)
{
	if (ActiveHeroIndex != OldIndex)
	{
		if (AZodiacHeroActor* Hero = HeroList.GetHero(OldIndex))
		{
			Hero->Deactivate();
		}

		if (AZodiacHeroActor* Hero = HeroList.GetHero(ActiveHeroIndex))
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
	if (UZodiacPawnExtensionComponent* PawnExtensionComponent = GetPawnExtensionComponent())
	{
		PawnExtensionComponent->CheckDefaultInitialization();	
	}
}

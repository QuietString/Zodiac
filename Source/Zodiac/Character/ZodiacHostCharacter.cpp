// the.quiet.string@gmail.com

#include "ZodiacHostCharacter.h"

#include "DisplayDebugHelpers.h"
#include "ZodiacGameplayTags.h"
#include "ZodiacHeroCharacter.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/ZodiacAbilitySet.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"
#include "Animation/ZodiacHostAnimInstance.h"
#include "Camera/ZodiacCameraComponent.h"
#include "Engine/Canvas.h"
#include "Player/ZodiacPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ZodiacPlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostCharacter)

AZodiacHostCharacter::AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer),
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
			//InitializeHostAbilitySystem(ZodiacASC);
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

void AZodiacHostCharacter::OnJustLanded()
{
	Super::OnJustLanded();
	
	if (UAbilitySystemComponent* HeroASC = GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle ContextHandle = HeroASC->MakeEffectContext();
		FGameplayEventData Payload;
		Payload.EventTag = ZodiacGameplayTags::Event_JustLanded;
		Payload.Target = HeroASC->GetAvatarActor();
		Payload.ContextHandle = ContextHandle;
		Payload.EventMagnitude = 1;
	
		FScopedPredictionWindow NewScopedWindow(HeroASC, true);
		HeroASC->HandleGameplayEvent(Payload.EventTag, &Payload);
	}
}

void AZodiacHostCharacter::OnJustLifted()
{
	Super::OnJustLifted();

	if (UAbilitySystemComponent* HeroASC = GetAbilitySystemComponent())
	{
		FGameplayEffectContextHandle ContextHandle = HeroASC->MakeEffectContext();
		FGameplayEventData Payload;
		Payload.EventTag = ZodiacGameplayTags::Event_JustLifted;
		Payload.Target = HeroASC->GetAvatarActor();
		Payload.ContextHandle = ContextHandle;
		Payload.EventMagnitude = 1;
	
		FScopedPredictionWindow NewScopedWindow(HeroASC, true);
		HeroASC->HandleGameplayEvent(Payload.EventTag, &Payload);
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

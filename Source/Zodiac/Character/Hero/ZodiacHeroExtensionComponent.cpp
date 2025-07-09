// the.quiet.string@gmail.com


#include "ZodiacHeroExtensionComponent.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHeroActor.h"
#include "ZodiacLogChannels.h"
#include "AbilitySystem/Hero/ZodiacHeroAbilitySystemComponent.h"
#include "ZodiacHeroData.h"
#include "Character/Host/ZodiacHostCharacter.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroExtensionComponent)

using namespace ZodiacGameplayTags;
const FName UZodiacHeroExtensionComponent::NAME_ActorFeatureName("HeroExtension");
const TArray<FGameplayTag> UZodiacHeroExtensionComponent::StateChain = {InitState_Spawned, InitState_DataAvailable,InitState_DataInitialized,InitState_GameplayReady };

UZodiacHeroExtensionComponent::UZodiacHeroExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	HeroData = nullptr;
	AbilitySystemComponent = nullptr;
}

bool UZodiacHeroExtensionComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	AZodiacHeroActor* HeroActor = GetOwner<AZodiacHeroActor>();
	AZodiacHostCharacter* HostCharacter = HeroActor->GetHostCharacter();
	
	if (!CurrentState.IsValid() && DesiredState == InitState_Spawned)
	{
		if (HeroActor && HostCharacter)
		{
			return true;
		}
	}
	if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
	{
		// Hero data is required.
		if (!HeroData)
		{
			return false;
		}

		const bool bHasAuthority = HostCharacter->HasAuthority();
		const bool bIsLocallyControlled = HostCharacter->IsLocallyControlled();

		if (bHasAuthority || bIsLocallyControlled)
		{
			// Check for being possessed by a controller.
			if (!HostCharacter->GetController<AController>())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
	{
		// Transition to initialize if all features have their data available
		return Manager->HaveAllFeaturesReachedInitState(HeroActor, InitState_DataAvailable);
	}
	else if (CurrentState == InitState_DataInitialized && DesiredState == InitState_GameplayReady)
	{
		return true;
	}

	return false;
}

void UZodiacHeroExtensionComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	// This is currently all handled by other components listening to this state change
}

void UZodiacHeroExtensionComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == AZodiacHeroActor::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == InitState_DataInitialized)
		{
			// If the extension component says all other components are initialized, try to progress to the next state
			CheckDefaultInitialization();
		}
	}
}

void UZodiacHeroExtensionComponent::CheckDefaultInitialization()
{
	ContinueInitStateChain(StateChain);
}

void UZodiacHeroExtensionComponent::SetHeroData(const UZodiacHeroData* InHeroData)
{
	check(InHeroData);

	AZodiacHeroActor* HeroActor = GetOwner<AZodiacHeroActor>();
	AZodiacHostCharacter* Host = HeroActor->GetHostCharacter();
	if (Host->GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (HeroData)
	{
		UE_LOG(LogZodiac, Error, TEXT("Trying to set HeroData [%s] on hero [%s] that already has valid HeroData [%s]."), *GetNameSafe(InHeroData),  *GetNameSafe(HeroActor), *GetNameSafe(HeroData));
		return;
	}

	HeroData = InHeroData;

	HeroActor->ForceNetUpdate();

	CheckDefaultInitialization();
}

void UZodiacHeroExtensionComponent::InitializeAbilitySystem(UZodiacHeroAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	if (AbilitySystemComponent == InASC)
	{
		// The ability system component hasn't changed.
		return;
	}

	if (AbilitySystemComponent)
	{
		// Clean up the old ability system component.
		UninitializeAbilitySystem();
	}

	AZodiacHeroActor* HeroActor = GetOwner<AZodiacHeroActor>();
	AActor* ExistingAvatar = InASC->GetAvatarActor();

	UE_LOG(LogZodiac, Verbose, TEXT("Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] "), *GetNameSafe(InASC), *GetNameSafe(HeroActor), *GetNameSafe(InOwnerActor), *GetNameSafe(ExistingAvatar));

	if ((ExistingAvatar != nullptr) && (ExistingAvatar != HeroActor))
	{
		UE_LOG(LogZodiac, Log, TEXT("Existing avatar (authority=%d)"), ExistingAvatar->HasAuthority() ? 1 : 0);

		// There is already a pawn acting as the ASC's avatar, so we need to kick it out
		// This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
		ensure(!ExistingAvatar->HasAuthority());

		if (UZodiacHeroExtensionComponent* OtherExtensionComponent = FindHeroExtensionComponent(ExistingAvatar))
		{
			OtherExtensionComponent->UninitializeAbilitySystem();
		}
	}

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, HeroActor);
	
	OnAbilitySystemInitialized.Broadcast();
}

void UZodiacHeroExtensionComponent::UninitializeAbilitySystem()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	// Uninitialize the ASC if we're still the avatar actor (otherwise another hero already did it when they became the avatar actor)
	if (AbilitySystemComponent->GetAvatarActor() == GetOwner())
	{
		FGameplayTagContainer AbilityTypesToIgnore;
		AbilityTypesToIgnore.AddTag(Ability_Behavior_SurvivesDeath);

		AbilitySystemComponent->CancelAbilities(nullptr, &AbilityTypesToIgnore);
		AbilitySystemComponent->ClearAbilityInput();
		AbilitySystemComponent->RemoveAllGameplayCues();

		if (AbilitySystemComponent->GetOwnerActor() != nullptr)
		{
			AbilitySystemComponent->SetAvatarActor(nullptr);
		}
		else
		{
			// If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
			AbilitySystemComponent->ClearActorInfo();
		}

		OnAbilitySystemUninitialized.Broadcast();
	}

	AbilitySystemComponent = nullptr;
}

void UZodiacHeroExtensionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, HeroData);
}

void UZodiacHeroExtensionComponent::OnRegister()
{
	Super::OnRegister();

	const AZodiacHeroActor* HeroActor = GetOwner<AZodiacHeroActor>();
	ensureAlwaysMsgf((HeroActor != nullptr), TEXT("ZodiacHeroExtensionComponent on [%s] can only be added to Hero actors."), *GetNameSafe(GetOwner()));

	TArray<UActorComponent*> HeroExtensionComponents;
	HeroActor->GetComponents(ThisClass::StaticClass(), HeroExtensionComponents);
	ensureAlwaysMsgf((HeroExtensionComponents.Num() == 1), TEXT("Only one ZodiacHeroExtensionComponent should exist on [%s]."), *GetNameSafe(GetOwner()));

	// Register with the init state system early, this will only work if this is a game world
	RegisterInitStateFeature();
}

void UZodiacHeroExtensionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the hero actor changes init state
	BindOnActorInitStateChanged(AZodiacHeroActor::NAME_ActorFeatureName, FGameplayTag(), false);
	
	// Notifies state manager that we have spawned, then try the rest of default initialization
	ensure(TryToChangeInitState(InitState_Spawned));
	CheckDefaultInitialization();
}

void UZodiacHeroExtensionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UninitializeAbilitySystem();
	UnregisterInitStateFeature();

	Super::EndPlay(EndPlayReason);
}

void UZodiacHeroExtensionComponent::OnRep_HeroData()
{
	CheckDefaultInitialization();
}

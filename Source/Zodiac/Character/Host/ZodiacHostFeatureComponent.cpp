// the.quiet.string@gmail.com


#include "ZodiacHostFeatureComponent.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacHostCharacter.h"
#include "ZodiacLogChannels.h"
#include "Character/ZodiacPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Character/Hero/ZodiacHeroActor.h"
#include "Misc/UObjectToken.h"
#include "Player/ZodiacPlayerController.h"
#include "Player/ZodiacPlayerState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHostFeatureComponent)

using namespace ZodiacGameplayTags;

const FName UZodiacHostFeatureComponent::NAME_ActorFeatureName("HostFeature");
const TArray<FGameplayTag> UZodiacHostFeatureComponent::StateChain = {InitState_Spawned, InitState_DataAvailable,InitState_DataInitialized, InitState_Host_HeroSpawned,InitState_Host_HeroDataAvailable,InitState_Host_HeroDataInitialized, InitState_Host_HeroGameplayReady,InitState_GameplayReady };

UZodiacHostFeatureComponent::UZodiacHostFeatureComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UZodiacHostFeatureComponent::CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const
{
	check(Manager);

	AZodiacHostCharacter* HostCharacter = GetPawn<AZodiacHostCharacter>();
	if (!CurrentState.IsValid() && DesiredState == InitState_Spawned)
	{
		// As long as we are on a valid pawn, we count as spawned
		if (HostCharacter)
		{
			return true;
		}
	}
	if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
	{
		// The player state is required.
		if (!GetPlayerState<AZodiacPlayerState>())
		{
			return false;
		}
		
		// If we're authority or autonomous, we need to wait for a controller with registered ownership of the player state.
		if (HostCharacter->GetLocalRole() != ROLE_SimulatedProxy)
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

		const bool bIsLocallyControlled = HostCharacter->IsLocallyControlled();
		const bool bIsBot = HostCharacter->IsBotControlled();

		if (bIsLocallyControlled && !bIsBot)
		{
			AZodiacPlayerController* ZodiacPC = GetController<AZodiacPlayerController>();

			// The input component and local player are required when locally controlled.
			if (!HostCharacter->InputComponent || !ZodiacPC || !ZodiacPC->GetLocalPlayer())
			{
				return false;
			}
		}

		return true;
	}
	else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
	{
		// Wait for a player state and extension component
		AZodiacPlayerState* ZodiacPS = GetPlayerState<AZodiacPlayerState>();

		return ZodiacPS && Manager->HasFeatureReachedInitState(HostCharacter, UZodiacPawnExtensionComponent::NAME_ActorFeatureName,InitState_DataInitialized);
	}
	else if (CurrentState == InitState_DataInitialized && DesiredState == InitState_Host_HeroSpawned)
	{
		return true;
	}
	else if (CurrentState == InitState_Host_HeroSpawned && DesiredState == InitState_Host_HeroDataAvailable)
	{
		if (HostCharacter->GetHeroClasses().Num() != HostCharacter->GetHeroes().Num())
		{
			return false;
		}

		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (!Manager->HaveAllFeaturesReachedInitState(Hero, InitState_Spawned))
			{
				return false;
			}
		}
		
		return true;
	}
	else if (CurrentState == InitState_Host_HeroDataAvailable && DesiredState == InitState_Host_HeroDataInitialized)
	{
		for (auto& Hero : HostCharacter->GetHeroes())
		{
			if (!Manager->HaveAllFeaturesReachedInitState(Hero, InitState_DataInitialized))
			{
				//UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Host Feature failed to change state to %s. Reason: other data is not initialized."), *DesiredState.ToString());
				return false;
			}
		}
		
		return true;
	}
	else if (CurrentState == InitState_Host_HeroDataInitialized && DesiredState == InitState_Host_HeroGameplayReady)
	{
		return true;
	}
	else if (CurrentState == InitState_Host_HeroGameplayReady && DesiredState == InitState_GameplayReady)
	{
		
		return true;
	}
	
	return false;
}

void UZodiacHostFeatureComponent::HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState)
{
	//UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Host Feature Current State: %s, Desired State: %s"), *CurrentState.ToString(), *DesiredState.ToString());

	AZodiacHostCharacter* HostCharacter = GetPawn<AZodiacHostCharacter>();
	
	if (!CurrentState.IsValid() && DesiredState == InitState_Spawned)
	{
		
	}
	else if (CurrentState == InitState_Spawned && DesiredState == InitState_DataAvailable)
	{
		
	}
	else if (CurrentState == InitState_DataAvailable && DesiredState == InitState_DataInitialized)
	{
		AZodiacPlayerState* ZodiacPS = GetPlayerState<AZodiacPlayerState>();
		if (!ensure(HostCharacter && ZodiacPS))
		{
			return;
		}

		InitializeAbilitySystem(ZodiacPS->GetZodiacAbilitySystemComponent(), ZodiacPS);
	}
	else if (CurrentState == InitState_DataInitialized && DesiredState == InitState_Host_HeroSpawned)
	{
		if (HasAuthority())
		{
			//UE_LOG_WITH_ROLE(LogZodiacFramework, Warning, TEXT("Spawn Heroes"));
			HostCharacter->SpawnHeroes();
		}
	}
	else if (CurrentState == InitState_Host_HeroSpawned && DesiredState == InitState_Host_HeroDataAvailable)
	{
		
	}
	else if (CurrentState == InitState_Host_HeroDataAvailable && DesiredState == InitState_Host_HeroDataInitialized)
	{
		
	}
	else if (CurrentState == InitState_Host_HeroDataInitialized && DesiredState == InitState_Host_HeroGameplayReady)
	{
		
	}
	else if (CurrentState == InitState_Host_HeroGameplayReady && DesiredState == InitState_GameplayReady)
	{
		if (HostCharacter->HasAuthority())
		{
			HostCharacter->ChangeHero(0);	
		}
	}
}

void UZodiacHostFeatureComponent::OnActorInitStateChanged(const FActorInitStateChangedParams& Params)
{
	if (Params.FeatureName == UZodiacPawnExtensionComponent::NAME_ActorFeatureName)
	{
		if (Params.FeatureState == InitState_DataInitialized)
		{
			// If the extension component says all other components are initialized, try to progress to the next state
			CheckDefaultInitialization();
		}
	}

	if (Params.FeatureName == NAME_ActorFeatureName && !HasReachedInitState(InitState_Host_HeroGameplayReady))
	{
		//UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("HostFeature OnActorInitStateChanged: %s"), *Params.FeatureState.ToString());
		if (AZodiacHostCharacter* HostCharacter = GetPawn<AZodiacHostCharacter>())
		{
			for (auto& Hero : HostCharacter->GetHeroes())
			{
				//UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Called Hero Check Init from HostFeature OnActorInit"));
				Hero->CheckDefaultInitialization();
			}	
		}
	}
}

void UZodiacHostFeatureComponent::CheckDefaultInitialization()
{
	//UE_LOG_WITH_ROLE(LogZodiacFramework, Log, TEXT("Host Feature check init"));
	// This will try to progress from spawned (which is only set in BeginPlay) through the data initialization stages until it gets to gameplay ready
	ContinueInitStateChain(StateChain);
}

void UZodiacHostFeatureComponent::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	APawn* Pawn = GetPawn<APawn>();
	if (UZodiacPawnExtensionComponent* PawnExtComp = UZodiacPawnExtensionComponent::FindPawnExtensionComponent(Pawn))
	{
		// The player state holds the persistent data for this player (state that persists across deaths and multiple pawns).
		// The ability system component and attribute sets live on the player state.
		PawnExtComp->InitializeAbilitySystem(InASC, InOwnerActor);
	}
}

void UZodiacHostFeatureComponent::OnRegister()
{
	Super::OnRegister();

	if (!GetPawn<APawn>())
	{
		UE_LOG(LogZodiacFramework, Error, TEXT("[UZodiacHostFeatureComponent::OnRegister] This component has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint."));

#if WITH_EDITOR
		if (GIsEditor)
		{
			static const FText Message = NSLOCTEXT("ZodiacHostFeatureComponent", "NotOnPawnError", "has been added to a blueprint whose base class is not a Pawn. To use this component, it MUST be placed on a Pawn Blueprint. This will cause a crash if you PIE!");
			static const FName HeroMessageLogName = TEXT("ZodiacHostFeatureComponent");
			
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

void UZodiacHostFeatureComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for when the pawn extension component changes init state
	BindOnActorInitStateChanged(UZodiacPawnExtensionComponent::NAME_ActorFeatureName, FGameplayTag(), false);
	BindOnActorInitStateChanged(NAME_None, FGameplayTag(), false);

	// Notifies that we are done spawning, then try the rest of initialization
	ensure(TryToChangeInitState(InitState_Spawned));
	CheckDefaultInitialization();
}

void UZodiacHostFeatureComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnregisterInitStateFeature();
	
	Super::EndPlay(EndPlayReason);
}

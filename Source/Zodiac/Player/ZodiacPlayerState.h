// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "ModularPlayerState.h"
#include "System/GameplayTagStack.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacPlayerState.generated.h"

class UZodiacHeroData;
class UZodiacAbilitySystemComponent;

/** Defines the types of client connected */
UENUM()
enum class EZodiacPlayerConnectionType : uint8
{
	// An active player
	Player = 0,

	// Spectator connected to a running game
	LiveSpectator,

	// Spectating a demo recording offline
	ReplaySpectator,

	// A deactivated player (disconnected)
	InactivePlayer
};

/**
 * Base player state class used by this project.
 */
UCLASS()
class ZODIAC_API AZodiacPlayerState : public AModularPlayerState, public IAbilitySystemInterface, public IZodiacTeamAgentInterface
{
	GENERATED_BODY()

public:
	AZodiacPlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	void SetPawnData(const UZodiacHeroData* InPawnData);
	
	//~IZodiacTeamAgentInterface interface
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	//~End of IZodiacTeamAgentInterface interface

	//~APlayerState interface
	virtual void Reset() override;
	virtual void ClientInitialize(AController* C) override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	//~End of APlayerState interface
	
	static const FName NAME_ZodiacAbilityReady;
	
protected:
	UPROPERTY(Replicated)
	TObjectPtr<const UZodiacHeroData> PawnData;
	
private:
	// The ability system component sub-object used by player characters.
	UPROPERTY(VisibleAnywhere, Category = "Zodiac|PlayerState")
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(Replicated)
	EZodiacPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;
};

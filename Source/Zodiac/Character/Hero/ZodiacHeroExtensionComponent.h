// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "ZodiacHeroExtensionComponent.generated.h"


class UZodiacHeroData;
class UZodiacHeroAbilitySystemComponent;

UCLASS(MinimalAPI)
class UZodiacHeroExtensionComponent : public UGameFrameworkComponent, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZodiacHeroExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/** The name of this overall feature, this one depends on the other named component features */
	static const FName NAME_ActorFeatureName;
	static const TArray<FGameplayTag> StateChain;
	
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	/** Returns the hero extension component if one exists on the specified actor. */
	UFUNCTION(BlueprintPure, Category = "Zodiac|Hero")
	static UZodiacHeroExtensionComponent* FindHeroExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UZodiacHeroExtensionComponent>() : nullptr); }
	
	/** Gets the hero data, which is used to specify hero properties in data */
	template <class T>
	const T* GetHeroData() const { return Cast<T>(HeroData); }
	
	void SetHeroData(const UZodiacHeroData* InHeroData);
	
	/** Should be called by the owning actor to become the avatar of the ability system. */
	void InitializeAbilitySystem(UZodiacHeroAbilitySystemComponent* InASC, AActor* InOwnerActor);

	/** Should be called by the owning actor to remove itself as the avatar of the ability system. */
	void UninitializeAbilitySystem();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UFUNCTION()
	void OnRep_HeroData();
	
protected:
	/** Delegate fired when our hero actor becomes the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

	/** Delegate fired when our hero actor is removed as the ability system's avatar actor */
	FSimpleMulticastDelegate OnAbilitySystemUninitialized;

	/** Hero data used to create the hero actor. Specified from a spawn function or on a placed instance. */
	UPROPERTY(EditInstanceOnly, ReplicatedUsing = OnRep_HeroData)
	TObjectPtr<const UZodiacHeroData> HeroData;

	/** Pointer to the ability system component that is cached for convenience. */
	UPROPERTY(Transient)
	TObjectPtr<UZodiacHeroAbilitySystemComponent> AbilitySystemComponent;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagAssetInterface.h"
#include "Components/GameFrameworkInitStateInterface.h"
#include "GameFramework/Actor.h"
#include "Teams/ZodiacTeamAgentInterface.h"
#include "ZodiacHeroActor.generated.h"

class AZodiacPlayerController;
class UZodiacHeroExtensionComponent;
class UZodiacHeroSkeletalMeshComponent;
class UZodiacHeroAnimInstance;
class UZodiacHeroAbilitySystemComponent;
class UZodiacHealthComponent;
class UZodiacHeroAbilityManagerComponent;
class AZodiacHostCharacter;
class UZodiacHeroData;
class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHeroChanged2, AZodiacHeroActor*, Hero);

UCLASS()
class ZODIAC_API AZodiacHeroActor : public AActor, public IAbilitySystemInterface, public IGameplayTagAssetInterface, public IZodiacTeamAgentInterface, public IGameFrameworkInitStateInterface
{
	GENERATED_BODY()

public:
	AZodiacHeroActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

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

	void OnOwnerChanged();
	void CheckHostDefaultInitializationNextTick();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const;
	UZodiacHeroAbilitySystemComponent* GetHeroAbilitySystemComponent() const;
	
	//~AActor interface
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of AActor interface
	
	//~IGameplayTagAssetInterface
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	virtual bool HasMatchingGameplayTag(FGameplayTag TagToCheck) const override;
	virtual bool HasAllMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	virtual bool HasAnyMatchingGameplayTags(const FGameplayTagContainer& TagContainer) const override;
	//~End of IGameplayTagAssetInterface

	//~IZodiacTeamAgentInterface
	virtual FGenericTeamId GetGenericTeamId() const override;
	//End of IZodiacTeamAgentInterface

	UFUNCTION(BlueprintCallable)
	bool IsHostLocallyControlled() const;
	
	USkeletalMeshComponent* GetMesh() const;

	UZodiacHealthComponent* GetHealthComponent() const;

	UZodiacHeroAnimInstance* GetHeroAnimInstance() const;

	void SetHostCharacter(AZodiacHostCharacter* InHostCharacter);
	
	UFUNCTION(BlueprintCallable)
	AZodiacHostCharacter* GetHostCharacter() const { return HostCharacter.Get(); }

	UFUNCTION(BlueprintCallable)
	AZodiacPlayerController* GetHostController() const;
	
	void InitializeAbilitySystem();
	void UninitializeAbilitySystem();
	void InitializeWithAbilitySystem();
	void InitializeMesh();
	void OnStatusTagChanged(FGameplayTag Tag, int Count);
	
	UFUNCTION(BlueprintCallable)
	void SetModularMesh(TSubclassOf<USkeletalMeshComponent> SkeletalMeshCompClass, FName Socket);

	UFUNCTION(BlueprintCallable)
	void ClearModularMesh();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnCloseContactStarted();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OnCloseContactFinished();
	
	void Activate();
	void Deactivate();

	FSimpleMulticastDelegate OnHeroActivated;
	FSimpleMulticastDelegate OnHeroDeactivated;
	
	UPROPERTY(BlueprintAssignable)
	FOnHeroChanged2 OnHeroActivated_BP;
	
	UPROPERTY(BlueprintAssignable)
	FOnHeroChanged2 OnHeroDeactivated_BP;

public:
	UPROPERTY(EditAnywhere, Category = "Hero")
	TObjectPtr<UZodiacHeroData> HeroData;

	UPROPERTY(EditAnywhere, Category = "Hero")
	TSubclassOf<UAnimInstance> AnimLayerClass;

protected:
	UPROPERTY(ReplicatedUsing=OnRep_HostCharacter)
	TWeakObjectPtr<AZodiacHostCharacter> HostCharacter;

	UFUNCTION()
	void OnRep_HostCharacter();
	
	// Initial use only for initialization on a client.
	UPROPERTY(ReplicatedUsing=OnRep_IsActive)
	bool bIsActive = false;

	UFUNCTION()
	void OnRep_IsActive(bool OldValue);
	
private:
	FTransform DefaultMeshTransform;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<USceneComponent> RootScene;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHeroSkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHeroAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHeroAbilityManagerComponent> AbilityManagerComponent;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> ModularMeshComponent;
};

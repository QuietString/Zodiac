// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacCharacter.h"
#include "Character/Hero/ZodiacHeroList.h"
#include "Traversal/ZodiacTraversalActorInterface.h"
#include "ZodiacHostCharacter.generated.h"

class UZodiacHostFeatureComponent;
class UZodiacHostAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacHostCharacter : public AZodiacCharacter, public IZodiacTraversalActorInterface
{
	GENERATED_BODY()

public:
	AZodiacHostCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~AZodiacCharacter interface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual UZodiacAbilitySystemComponent* GetZodiacAbilitySystemComponent() const override;
	virtual UZodiacHealthComponent* GetHealthComponent() const override;
	virtual FGenericTeamId GetGenericTeamId() const override { return static_cast<uint8>(MyTeam); }
	virtual USkeletalMeshComponent* GetRetargetedMesh() const override;
	virtual void SetMovementModeTag(EMovementMode MovementMode, uint8 CustomMovementMode, bool bTagEnabled) override;
	virtual void SetExtendedMovementModeTag(EZodiacExtendedMovementMode ExtendedMovementMode, bool bTagEnabled) override;
	//~End of AZodiacCharacter interface

	UZodiacHostAbilitySystemComponent* GetHostAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable)
	virtual UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;

	//~IZodiacTraversalActorInterface
	virtual UAbilitySystemComponent* GetTraversalAbilitySystemComponent() const override;
	//~End of IZodiacTraversalActorInterface
	
	// Return the currently active hero
	UFUNCTION(BlueprintCallable)
	AZodiacHeroActor* GetHero() const { return HeroList.GetHero(ActiveHeroIndex); }

	UFUNCTION(BlueprintPure)
	TArray<AZodiacHeroActor*> GetHeroes() const { return HeroList.GetHeroes(); }
	
	virtual void PossessedBy(class AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;

	void SpawnHeroes();
	
	virtual void Input_AbilityInputTagPressed(FGameplayTag InputTag) override;
	virtual void Input_AbilityInputTagReleased(FGameplayTag InputTag) override;
	
	UFUNCTION(BlueprintCallable)
	void ChangeHero(const int32 Index);
	
	UFUNCTION(BlueprintImplementableEvent)
	void OnAllHeroesInitialized();
	
	UFUNCTION(BlueprintPure)
	float GetAimYaw() const;
	
	void UpdateHeroEyeLocationOffset();

	FVector GetHeroEyeLocationOffset() const { return HeroEyeLocationOffset; }
	
	TArray<TSubclassOf<AZodiacHeroActor>> GetHeroClasses() const { return HeroClasses; }

protected:
	virtual void OnAbilitySystemInitialized() override;
	virtual void OnAbilitySystemUninitialized() override;
	virtual void InitializeGameplayTags() override;
	
protected:
	UPROPERTY(ReplicatedUsing=OnRep_ActiveHeroIndex, BlueprintReadOnly)
	int32 ActiveHeroIndex = INDEX_NONE;
	
	UPROPERTY(EditAnywhere)
	bool bEnableCameraHeroOffset = true;
	
	UPROPERTY(ReplicatedUsing = OnRep_HeroList)
	FZodiacHeroList2 HeroList;

	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AZodiacHeroActor>> HeroClasses;
	
private:
	UPROPERTY()
	EZodiacTeam MyTeam = EZodiacTeam::Hero;

private:
	UPROPERTY(meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacHostFeatureComponent> HostFeatureComponent;
	
	UFUNCTION()
	void OnRep_ActiveHeroIndex(int32 OldIndex);

	UFUNCTION()
	void OnRep_HeroList();

	UPROPERTY(Transient)
	FVector HeroEyeLocationOffset;
	
	bool bHasHeroInitialized = false;
};

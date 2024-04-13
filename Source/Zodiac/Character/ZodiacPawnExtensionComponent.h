// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "ZodiacPawnExtensionComponent.generated.h"

class UZodiacAbilitySystemComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FZodiacDynamicMulticastDelegate);


/**
 * UZodiacPawnExtensionComponent
 *
 *	Component used to add functionality to all Pawn classes.
 */
UCLASS()
class ZODIAC_API UZodiacPawnExtensionComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacPawnExtensionComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Returns the pawn extension component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Zodiac|Pawn")
	static UZodiacPawnExtensionComponent* FindPawnExtensionComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UZodiacPawnExtensionComponent>() : nullptr); }

	// Should be called by the owning pawn to become the avatar of the ability system.
	void InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwnerActor);

	// Call this anytime the pawn needs to check if it's ready to be initialized (pawn data assigned, possessed, etc..). 
	bool CheckPawnReadyToInitialize();

	// Register with the OnPawnReadyToInitialize delegate and broadcast if condition is already met.
	void RegisterAndCall_OnPawnReadyToInitialize(FSimpleMulticastDelegate::FDelegate Delegate);

	// Register with the OnAbilitySystemInitialized delegate and broadcast if condition is already met.
	void RegisterAndCall_OnAbilitySystemInitialized(FSimpleMulticastDelegate::FDelegate Delegate);

protected:
	virtual void OnRegister() override;

	// Delegate fired when pawn has everything needed for initialization.
	FSimpleMulticastDelegate OnPawnReadyToInitialize;
	
	// Delegate fired when our pawn becomes the ability system's avatar actor
	FSimpleMulticastDelegate OnAbilitySystemInitialized;

protected:
	// Pointer to the ability system component that is cached for convenience.
	UPROPERTY()
	UZodiacAbilitySystemComponent* AbilitySystemComponent;

	// True when the pawn has everything needed for initialization.
	int32 bPawnReadyToInitialize : 1;
};

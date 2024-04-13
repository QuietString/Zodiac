// the.quiet.string@gmail.com


#include "ZodiacPawnExtensionComponent.h"

#include "AbilitySystem/ZodiacAbilitySystemComponent.h"


UZodiacPawnExtensionComponent::UZodiacPawnExtensionComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UZodiacPawnExtensionComponent::InitializeAbilitySystem(UZodiacAbilitySystemComponent* InASC, AActor* InOwnerActor)
{
	check(InASC);
	check(InOwnerActor);

	APawn* Pawn = GetPawnChecked<APawn>();

	AbilitySystemComponent = InASC;
	AbilitySystemComponent->InitAbilityActorInfo(InOwnerActor, Pawn);

	OnAbilitySystemInitialized.Broadcast();
}

bool UZodiacPawnExtensionComponent::CheckPawnReadyToInitialize()
{
	if (bPawnReadyToInitialize)
	{
		return true;
	}

	APawn* Pawn = GetPawnChecked<APawn>();

	const bool bHasAuthority = Pawn->HasAuthority();
	const bool bIsLocallyControlled = Pawn->IsLocallyControlled();

	if (bHasAuthority || bIsLocallyControlled)
	{
		// Check for being possessed by a controller.
		if (!GetController<AController>())
		{
			return false;
		}
	}

	// Pawn is ready to initialize.
	bPawnReadyToInitialize = true;
	OnPawnReadyToInitialize.Broadcast();

	UE_LOG(LogTemp, Warning, TEXT("check pawn ready true"));

	return true;
}

void UZodiacPawnExtensionComponent::RegisterAndCall_OnPawnReadyToInitialize(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnPawnReadyToInitialize.IsBoundToObject(Delegate.GetUObject()))
	{
		OnPawnReadyToInitialize.Add(Delegate);
	}

	if (bPawnReadyToInitialize)
	{
		Delegate.Execute();
	}
}

void UZodiacPawnExtensionComponent::RegisterAndCall_OnAbilitySystemInitialized(
	FSimpleMulticastDelegate::FDelegate Delegate)
{
	if (!OnAbilitySystemInitialized.IsBoundToObject(Delegate.GetUObject()))
	{
		OnAbilitySystemInitialized.Add(Delegate);
	}

	if (AbilitySystemComponent)
	{
		Delegate.Execute();
	}
}

void UZodiacPawnExtensionComponent::OnRegister()
{
	Super::OnRegister();
}

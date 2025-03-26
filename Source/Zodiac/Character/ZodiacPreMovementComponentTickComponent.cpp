// the.quiet.string@gmail.com


#include "ZodiacPreMovementComponentTickComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacPreMovementComponentTickComponent)

UZodiacPreMovementComponentTickComponent::UZodiacPreMovementComponentTickComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UZodiacPreMovementComponentTickComponent::InitializeComponent()
{
	Super::InitializeComponent();

	OwningCharacter = GetOwner<ACharacter>();
	CharacterMovement = OwningCharacter->GetCharacterMovement();
}


void UZodiacPreMovementComponentTickComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (OwningCharacter && CharacterMovement)
	{
		CalculateBrakingDeceleration();
	}
}

void UZodiacPreMovementComponentTickComponent::CalculateBrakingDeceleration()
{
	bool bHasMovementInput = OwningCharacter->GetPendingMovementInputVector().IsZero();
	float Deceleration = bHasMovementInput ? 500.f : 2000.f;

	CharacterMovement->BrakingDecelerationWalking = Deceleration;
}


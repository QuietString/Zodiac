// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZodiacPreMovementComponentTickComponent.generated.h"


class UCharacterMovementComponent;

UCLASS()
class ZODIAC_API UZodiacPreMovementComponentTickComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZodiacPreMovementComponentTickComponent();

protected:
	virtual void InitializeComponent() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void CalculateBrakingDeceleration();
	
private:
	UPROPERTY(Transient)
	TObjectPtr<ACharacter> OwningCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CharacterMovement;
};

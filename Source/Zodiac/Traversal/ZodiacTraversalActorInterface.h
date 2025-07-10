// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZodiacTraversalActorInterface.generated.h"

class UAbilitySystemComponent;

UINTERFACE(BlueprintType, MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UZodiacTraversalActorInterface : public UInterface
{
	GENERATED_BODY()
};

class ZODIAC_API IZodiacTraversalActorInterface
{
	GENERATED_BODY()

public:
	// Since GetAbilitySystemComponent() returns ASC of a hero on AZodiacHostCharacter, needs to specify which ASC should use for traversal ability.
	UFUNCTION(BlueprintCallable)
	virtual UAbilitySystemComponent* GetTraversalAbilitySystemComponent() const=0;
};

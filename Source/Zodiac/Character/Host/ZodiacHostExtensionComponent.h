// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Character/ZodiacPawnExtensionComponent.h"
#include "ZodiacHostExtensionComponent.generated.h"


class AZodiacHostCharacter;

UCLASS()
class ZODIAC_API UZodiacHostExtensionComponent : public UZodiacPawnExtensionComponent
{
	GENERATED_BODY()

public:
	
	//~ Begin IGameFrameworkInitStateInterface interface
	//~ End IGameFrameworkInitStateInterface interface

	virtual void HandleControllerChanged() override;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "CommonGameInstance.h"
#include "ZodiacGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacGameInstance : public UCommonGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
};

// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacNumberPopComponent.h"
#include "ZodiacNumberPopComponent_NiagaraText.generated.h"


class UNiagaraComponent;
class UZodiacNumberPopStyleNiagara;

UCLASS(Blueprintable)
class ZODIAC_API UZodiacNumberPopComponent_NiagaraText : public UZodiacNumberPopComponent
{
	GENERATED_BODY()

public:
	UZodiacNumberPopComponent_NiagaraText(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void AddNumberPop(const FZodiacNumberPopRequest& NewRequest) override;

protected:
	TArray<int32> DamageNumberArray;

	/** Style patterns to attempt to apply to the incoming number pops */
	UPROPERTY(EditDefaultsOnly, Category = "Number Pop|Style")
	TObjectPtr<UZodiacNumberPopStyleNiagara> Style;

	//Niagara Component used to display the damage
	UPROPERTY(EditDefaultsOnly, Category = "Number Pop|Style")
	TObjectPtr<UNiagaraComponent> NiagaraComp;
};

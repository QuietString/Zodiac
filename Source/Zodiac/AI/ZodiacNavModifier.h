// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ZodiacNavModifier.generated.h"

class UNavArea;
class UBoxComponent;
class UNavModifierComponent;
class UBillboardComponent;

UCLASS()
class ZODIAC_API AZodiacNavModifier : public AActor
{
	GENERATED_BODY()

public:
	AZodiacNavModifier(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation")
	UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Navigation")
	UNavModifierComponent* NavModifierComp;

public:
	UFUNCTION(BlueprintCallable, Category="Navigation")
	void SetNavArea(TSubclassOf<UNavArea> NewAreaClass);

#if WITH_EDITORONLY_DATA
private:
	UPROPERTY()
	UBillboardComponent* SpriteComponent;
#endif
};

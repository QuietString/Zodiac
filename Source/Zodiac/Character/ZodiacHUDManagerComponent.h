// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZodiacHUDManagerComponent.generated.h"

class AZodiacHostCharacter;
class UZodiacHeroData;
class UZodiacReticleWidgetBase;

USTRUCT(BlueprintType, DisplayName = "HUD Message Reticle Changed")
struct FZodiacHUDMessage_ReticleChanged
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AActor> Instigator = nullptr;
	
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UZodiacReticleWidgetBase> Reticle = nullptr;
};

UCLASS()
class ZODIAC_API UZodiacHUDManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZodiacHUDManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void OnRegister() override;
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	void OnHeroActivated();

protected:
	void SendChangeReticleMessage();
};

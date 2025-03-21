// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Actor.h"
#include "ZodiacHeroActor.generated.h"

class USkeletalMeshComponent;
class UAbilitySystemComponent;

UCLASS()
class ZODIAC_API AZodiacHeroActor : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AZodiacHeroActor(const FObjectInitializer& ObjectInitializer);
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; }
	
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	
};

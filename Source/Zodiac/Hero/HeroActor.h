// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HeroActor.generated.h"

class UZodiacAbilitySystemComponent;
class UZodiacAbilitySet;

UCLASS()
class ZODIAC_API AHeroActor : public AActor
{
	GENERATED_BODY()

public:
	AHeroActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;

	void InitializeAbilitySystem(AActor* InOwnerActor);
	
public:
	UPROPERTY(EditAnywhere)
	TArray<TObjectPtr<UZodiacAbilitySet>> AbilitySets;
	
private:
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<USkeletalMeshComponent> Mesh;
	
	UPROPERTY(VisibleAnywhere, meta = (AllowPrivateAccess = true))
	TObjectPtr<UZodiacAbilitySystemComponent> AbilitySystemComponent;
};

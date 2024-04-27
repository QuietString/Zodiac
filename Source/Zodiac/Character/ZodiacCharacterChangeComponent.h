// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/PawnComponent.h"
#include "ZodiacCharacterChangeComponent.generated.h"


class AZodiacCharacter;
class UZodiacHeroData;

USTRUCT()
struct FCharacterMeshInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	USkeletalMesh* RetargetedMesh;

	UPROPERTY(EditAnywhere)
	UAnimInstance* AnimInstance;
};

UCLASS(meta=(BlueprintSpawnableComponent))
class UZodiacCharacterChangeComponent : public UPawnComponent
{
	GENERATED_BODY()

public:
	UZodiacCharacterChangeComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void ChangeCharacter(USkeletalMesh* NewMesh);

protected:

	AZodiacCharacter* GetZodiacCharacter();

	USkeletalMeshComponent* GetRetargetedMeshComponent();
	
	UFUNCTION()
	void OnRep_RetargetedMesh();

protected:

	UPROPERTY(ReplicatedUsing=OnRep_RetargetedMesh)
	USkeletalMesh* RetargetedMesh;

	void ChangeMesh();

	//UPROPERTY()
	//UAnimInstance* AnimInstance;
};

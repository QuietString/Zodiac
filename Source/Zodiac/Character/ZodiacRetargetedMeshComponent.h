// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "ZodiacRetargetedMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacRetargetedMeshComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UZodiacRetargetedMeshComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	void ChangeMesh(USkeletalMesh* NewMesh);

protected:
	UFUNCTION()
	void OnRep_RetargetedMesh();

	UFUNCTION(Server, Reliable)
	void ServerChangeMesh(USkeletalMesh* NewMesh);

protected:
	UPROPERTY(ReplicatedUsing=OnRep_RetargetedMesh)
	USkeletalMesh* RetargetedMesh;
};

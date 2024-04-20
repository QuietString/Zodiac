// the.quiet.string@gmail.com


#include "ZodiacRetargetedMeshComponent.h"

#include "Net/UnrealNetwork.h"

UZodiacRetargetedMeshComponent::UZodiacRetargetedMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacRetargetedMeshComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UZodiacRetargetedMeshComponent, RetargetedMesh, COND_None);
}

void UZodiacRetargetedMeshComponent::ChangeMesh(USkeletalMesh* NewMesh)
{
	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			RetargetedMesh = NewMesh;
		}
		else
		{
			ServerChangeMesh(NewMesh);
		}
	}
}

void UZodiacRetargetedMeshComponent::OnRep_RetargetedMesh()
{
	SetSkeletalMeshAsset(RetargetedMesh);
	UE_LOG(LogTemp, Log, TEXT("Mesh updated to: %s"), *RetargetedMesh->GetName());
}

void UZodiacRetargetedMeshComponent::ServerChangeMesh_Implementation(USkeletalMesh* NewMesh)
{
	RetargetedMesh = NewMesh;
	//OnRep_RetargetedMesh();
}

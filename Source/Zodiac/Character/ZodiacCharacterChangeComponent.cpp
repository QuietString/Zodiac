// the.quiet.string@gmail.com


#include "ZodiacCharacterChangeComponent.h"

#include "ZodiacCharacter.h"
#include "Net/UnrealNetwork.h"


UZodiacCharacterChangeComponent::UZodiacCharacterChangeComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZodiacCharacterChangeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZodiacCharacterChangeComponent, RetargetedMesh);
}

AZodiacCharacter* UZodiacCharacterChangeComponent::GetZodiacCharacter()
{
	if (AActor* Owner = GetOwner())
	{
		if (AZodiacCharacter* ZodiacCharacter = Cast<AZodiacCharacter>(Owner))
		{
			return ZodiacCharacter;
		}
	}

	return nullptr;
}

USkeletalMeshComponent* UZodiacCharacterChangeComponent::GetRetargetedMeshComponent()
{
	if (AZodiacCharacter* ZodiacCharacter = GetZodiacCharacter())
	{
		return ZodiacCharacter->GetRetargetedMeshComponent();
	}

	return nullptr;
}

void UZodiacCharacterChangeComponent::ChangeCharacter(USkeletalMesh* NewMesh)
{
	if (AActor* Owner = GetOwner())
	{
		if (Owner->HasAuthority())
		{
			RetargetedMesh = NewMesh;
			OnRep_RetargetedMesh();
		}
	}
}

void UZodiacCharacterChangeComponent::OnRep_RetargetedMesh()
{
	if (USkeletalMeshComponent* RetargetedMeshComp = GetRetargetedMeshComponent())
	{
		RetargetedMeshComp->SetSkeletalMeshAsset(RetargetedMesh);
	}
}

void UZodiacCharacterChangeComponent::ChangeMesh()
{
	if (USkeletalMeshComponent* RetargetedMeshComp = GetRetargetedMeshComponent())
	{
		RetargetedMeshComp->SetSkeletalMeshAsset(RetargetedMesh);
	}
}
// the.quiet.string@gmail.com


#include "ZodiacCharacterChangeComponent.h"

#include "ZodiacCharacter.h"
#include "ZodiacHeroData.h"
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
	UE_LOG(LogTemp, Warning, TEXT("character change"));

	if (AActor* Owner = GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("got owner"));

		if (Owner->HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("has authority and mesh changed"));

			RetargetedMesh = NewMesh;
		}
	}
	else
	{
		ServerChangeMesh(NewMesh);
	}
}

void UZodiacCharacterChangeComponent::OnRep_RetargetedMesh()
{
	UE_LOG(LogTemp, Warning, TEXT("onrep"));
	if (USkeletalMeshComponent* RetargetedMeshComp = GetRetargetedMeshComponent())
	{
		UE_LOG(LogTemp, Warning, TEXT("set skeletal mesh"));

		RetargetedMeshComp->SetSkeletalMeshAsset(RetargetedMesh);
	}
}

void UZodiacCharacterChangeComponent::ServerChangeMesh_Implementation(USkeletalMesh* NewMesh)
{
	UE_LOG(LogTemp, Warning, TEXT("server change mesh"));

	RetargetedMesh = NewMesh;
}

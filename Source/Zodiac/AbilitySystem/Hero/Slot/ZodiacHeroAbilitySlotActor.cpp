// the.quiet.string@gmail.com


#include "ZodiacHeroAbilitySlotActor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZodiacHeroAbilitySlotActor)

AZodiacHeroAbilitySlotActor::AZodiacHeroAbilitySlotActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicates = true;

	PrimaryActorTick.bStartWithTickEnabled = false;
	Mesh = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>(this, TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh;

	SocketSource = EZodiacHeroAbilitySlotSocketSourceType::AttachedParent;
}

FVector AZodiacHeroAbilitySlotActor::GetSocketLocation(FName InSocketName) const
{
	switch (SocketSource)
	{
	case EZodiacHeroAbilitySlotSocketSourceType::Self:
		if (Mesh)
		{
			return Mesh->GetSocketLocation(InSocketName);
		}
	case EZodiacHeroAbilitySlotSocketSourceType::AttachedParent:
		if (USceneComponent* ParentComp = GetRootComponent()->GetAttachParent())
		{
			return ParentComp->GetSocketLocation(InSocketName);
		}
		
	default: return FVector();
	}
}

FVector AZodiacHeroAbilitySlotActor::GetSocketLocation() const
{
	return GetSocketLocation(SocketName);
}

void AZodiacHeroAbilitySlotActor::OnRep_Owner()
{
	Super::OnRep_Owner();
	K2_OnRep_Owner();
}

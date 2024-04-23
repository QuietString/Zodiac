// the.quiet.string@gmail.com


#include "ZodiacEffectArea.h"

#include "AbilitySystemComponent.h"
#include "Character/ZodiacCharacter.h"


AZodiacEffectArea::AZodiacEffectArea()
{
	PrimaryActorTick.bCanEverTick = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AZodiacEffectArea::OnBeginOverlap);
}

void AZodiacEffectArea::BeginPlay()
{
	Super::BeginPlay();
	
}

void AZodiacEffectArea::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AZodiacCharacter* Character = Cast<AZodiacCharacter>(OtherActor);
	if (Character)
	{
		UAbilitySystemComponent* ASC = Character->GetAbilitySystemComponent();
		if (ASC)
		{
			FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DamageEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle GEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				//UE_LOG(LogTemp, Warning, TEXT("GE applied"));
			}
		}
	}
}
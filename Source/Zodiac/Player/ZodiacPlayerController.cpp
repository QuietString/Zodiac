// the.quiet.string@gmail.com


#include "ZodiacPlayerController.h"

#include "ZodiacGameplayTags.h"
#include "ZodiacPlayerState.h"
#include "AbilitySystem/ZodiacAbilitySystemComponent.h"

AZodiacPlayerController::AZodiacPlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

AZodiacPlayerState* AZodiacPlayerController::GetZodiacPlayerState() const
{
	return CastChecked<AZodiacPlayerState>(PlayerState, ECastCheckedType::NullAllowed);
}

UZodiacAbilitySystemComponent* AZodiacPlayerController::GetZodiacAbilitySystemComponent() const
{
	const AZodiacPlayerState* ZodiacPS = GetZodiacPlayerState();
	return (ZodiacPS ? ZodiacPS->GetZodiacAbilitySystemComponent() : nullptr);
}

void AZodiacPlayerController::SetIsAutoRunning(const bool bEnabled)
{
	const bool bIsAutoRunning = GetIsAutoRunning();
	if (bEnabled != bIsAutoRunning)
	{
		if (!bEnabled)
		{
			OnEndAutoRun();
		}
		else
		{
			OnStartAutoRun();
		}
	}
}

bool AZodiacPlayerController::GetIsAutoRunning() const
{
	bool bIsAutoRunning = false;
	if (const UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		bIsAutoRunning = ZodiacASC->GetTagCount(ZodiacGameplayTags::Status_AutoRunning) > 0;
	}
	return bIsAutoRunning;
}

void AZodiacPlayerController::OnStartAutoRun()
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_AutoRunning, 1);
		//K2_OnStartAutoRun();
	}
}

void AZodiacPlayerController::OnEndAutoRun()
{
	if (UZodiacAbilitySystemComponent* ZodiacASC = GetZodiacAbilitySystemComponent())
	{
		ZodiacASC->SetLooseGameplayTagCount(ZodiacGameplayTags::Status_AutoRunning, 0);
		//K2_OnEndAutoRun();
	}
}

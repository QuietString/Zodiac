// the.quiet.string@gmail.com

#pragma once

#include "CoreMinimal.h"
#include "ZodiacAttributeSet.h"
#include "ZodiacHealthSet.generated.h"

/**
 * 
 */
UCLASS()
class ZODIAC_API UZodiacHealthSet : public UZodiacAttributeSet
{
	GENERATED_BODY()

public:
	UZodiacHealthSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	ATTRIBUTE_ACCESSORS(UZodiacHealthSet, Health);

	mutable FZodiacAttributeEvent OnOutOfHealth;
	
protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_Health, BlueprintReadOnly, meta=(AllowPrivateAccess = true), Category = "Attributes")
	FGameplayAttributeData Health;

	bool bOutOfHealth;
};

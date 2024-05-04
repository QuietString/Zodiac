// the.quiet.string@gmail.com

#pragma once

#include "ZodiacSkillDefinition.generated.h"

template <typename T> class TSubclassOf;


// Represents a fragment of an skill definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZODIAC_API UZodiacSkillFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UZodiacSkillDefinition* Instance) const {}
};


UCLASS(Blueprintable, Const, Abstract)
class UZodiacSkillDefinition : public UObject
{
	GENERATED_BODY()

public:
	UZodiacSkillDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UZodiacSkillFragment>> Fragments;

public:
	const UZodiacSkillFragment* FindFragmentByClass(TSubclassOf<UZodiacSkillFragment> FragmentClass) const;
};
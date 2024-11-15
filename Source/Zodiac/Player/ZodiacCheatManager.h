// the.quiet.string@gmail.com
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ZodiacCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif

class UZodiacAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogZodiacCheat, Log, All);

UCLASS(Within = PlayerController, MinimalAPI)
class UZodiacCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UZodiacCheatManager();

	virtual void InitCheatManager() override;

	// Helper function to write text to the console and to the log.
	static void CheatOutputText(const FString& TextToOutput);
	
	// Runs a cheat on the server for the owning player.
	UFUNCTION(exec)
	void Cheat(const FString& Msg);

	// Adds the dynamic tag to the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void AddTagToSelf(FString TagName);
	
	// Applies the specified damage amount to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelf(float DamageAmount);
	
	// Prevents the owning player from taking any damage.
	virtual void God() override;

protected:

	void ApplySetByCallerDamage(UZodiacAbilitySystemComponent* ZodiacASC, float DamageAmount);
	
	UZodiacAbilitySystemComponent* GetPlayerAbilitySystemComponent() const;

};

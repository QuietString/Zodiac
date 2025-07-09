// the.quiet.string@gmail.com
// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "ZodiacCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER (1 && !UE_BUILD_SHIPPING)
#endif

class AZodiacHostCharacter;
struct FGameplayTag;
class AZodiacHostCharacter;
class UZodiacHeroAbilitySystemComponent;
class UZodiacAbilitySystemComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogZodiacCheat, Log, All);

UCLASS(Within = PlayerController, MinimalAPI)
class UZodiacCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UZodiacCheatManager();

	virtual void InitCheatManager() override;

	// Runs a cheat on the server for the owning player.
	UFUNCTION(exec)
	void Cheat(const FString& Msg);

	UFUNCTION(exec)
	void CheatAll(const FString& Msg);
	
	// Adds the dynamic tag to the owning player's ability system component.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void AddTagToSelf(FString TagName);
	
	// Applies the specified damage amount to the owning player.
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void DamageSelf(float DamageAmount);

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void InfiniteAmmo();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void ChargeUltimate(float ChargeAmount);
	
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	virtual void InfiniteUltimate();
	
	// Prevents the owning player from taking any damage.
	virtual void God() override;
	
	//
	// Monster cheats
	//
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void PauseZombies();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void ResumeZombies();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void SpawnDebugAIPawns();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void KillDebugAIPawns();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void SpawnAllAIPawns();
	
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void KillAllAIPawns();
	
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void AddTagToAllMonsters(FString TagName);

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void AllMonstersInvincible();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void MonstersImmortal();

	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void DisableAIAttack();
	
	UFUNCTION(Exec, BlueprintAuthorityOnly)
	void ToggleMonsterAI();

	// Debug Mesh
	UFUNCTION(Exec)
	void ToggleMeshVisibility();

	// Day and Night
	UFUNCTION(Exec)
	void ToggleDayNight();
	
protected:
	void ApplySetByCallerDamage(UZodiacAbilitySystemComponent* ZodiacASC, float DamageAmount);
	
	UZodiacAbilitySystemComponent* GetHeroAbilitySystemComponent() const;
	AZodiacHostCharacter* GetHostCharacter() const;

	void ToggleDynamicTag(UZodiacAbilitySystemComponent* ZodiacASC, FGameplayTag TagToToggle);
};

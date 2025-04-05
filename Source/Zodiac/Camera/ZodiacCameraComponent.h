// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Camera/CameraComponent.h"
#include "Character/ZodiacHeroCharacter.h"
#include "Character/ZodiacHostCharacter.h"
#include "GameFramework/Actor.h"

#include "ZodiacCameraComponent.generated.h"

class UCanvas;
class UZodiacCameraMode;
class UZodiacCameraModeStack;
class UObject;
struct FFrame;
struct FGameplayTag;
struct FMinimalViewInfo;
template <class TClass> class TSubclassOf;

DECLARE_DELEGATE_RetVal(TSubclassOf<UZodiacCameraMode>, FZodiacCameraModeDelegate);
DECLARE_DELEGATE_RetVal(FVector, FZodiacCameraOffsetDelegate);

/**
 *	The base camera component class used by this project.
 */
UCLASS()
class UZodiacCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:

	UZodiacCameraComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the camera component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Zodiac|Camera")
	static UZodiacCameraComponent* FindCameraComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UZodiacCameraComponent>() : nullptr); }

	APlayerCameraManager* GetPlayerCameraManager();
	
	// Returns the target actor that the camera is looking at.
	virtual AActor* GetTargetActor() const { return GetOwner(); }
	
	// Delegate used to query for the best camera mode.
	FZodiacCameraModeDelegate DetermineCameraModeDelegate;

	// Delegate used to update translation offset
	FZodiacCameraOffsetDelegate UpdateCameraTranslationOffsetDelegate;
	
	// Add an offset to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	void AddFieldOfViewOffset(float FovOffset) { FieldOfViewOffset += FovOffset; }

	virtual void DrawDebug(UCanvas* Canvas) const;

	// Gets the tag associated with the top layer and the blend weight of it
	void GetBlendInfo(float& OutWeightOfTopLayer, FGameplayTag& OutTagOfTopLayer) const;

	FSimpleDelegate OnCloseContactStarted;
	FSimpleDelegate OnCloseContactFinished;
	
protected:

	virtual void OnRegister() override;
	virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView) override;

	void UpdateTranslationOffset(float DeltaTime);
	void HandleCloseContact();
	bool CheckHasCloseTarget();
	
	virtual void UpdateCameraModes();
	FVector GetHeroOffset();

public:
	UPROPERTY(EditAnywhere, Category = "Zodiac|Camera|Offset")
	bool bApplyTranslationOffset;
	
	UPROPERTY(EditAnywhere, Category = "Zodiac|Camera|Offset", meta = (EditCondition = "bApplyTranslationOffset"))
	float TranslationOffsetInterpSpeed = 120.f;

	UPROPERTY(EditAnywhere, Category = "Zodiac|Camera|Offset", meta = (EditCondition = "bApplyTranslationOffset"))
	bool bIgnoreZAxis = true;

	FVector TranslationOffset = FVector();

private:
	FVector LastTranslationOffset = FVector();
	
protected:
	// Stack used to blend the camera modes.
	UPROPERTY()
	TObjectPtr<UZodiacCameraModeStack> CameraModeStack;

	// Offset applied to the field of view.  The offset is only for one frame, it gets cleared once it is applied.
	float FieldOfViewOffset;

private:
	bool bHasCloseContact = false;
	bool bPreviousCloseContact = false;
};

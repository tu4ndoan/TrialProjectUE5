// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TRIALPROJECT_API ATPPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	FTimerHandle Respawn_TimerHandle;

	void OnKilled();

	void Respawn();
};

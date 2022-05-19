// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPTeamFightGameState.h"
#include "TPTeamFightGameMode.generated.h"

/**
 * 
 */

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTPOnHasNewPlayerJoin, bool, Successful);

UCLASS()
class TRIALPROJECT_API ATPTeamFightGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ATPTeamFightGameMode();

	//FTPOnHasNewPlayerJoin OnHasNewPlayerJoinEvent;

	void PostLogin(APlayerController* NewPlayer) override;

	AActor* ChoosePlayerStart(AController* Player);

	bool ShouldSpawnAtStartSpot(AController* Player) override { return false; };

	void PlayerHit(AController* Player, AActor* PlayerBeingHit, float Damage);

	void PreInitializeComponents() override;

	void DefaultTimer();

	void HandleMatchHasStarted() override;

	float MatchTime;

	FTimerHandle DefaultTimerHandle;

};

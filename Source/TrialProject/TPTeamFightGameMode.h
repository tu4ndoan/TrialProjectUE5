// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "TPTeamFightGameState.h"
#include "TPTeamFightGameMode.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FOnGameModeCombinedPostLogin);

UCLASS()
class TRIALPROJECT_API ATPTeamFightGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	ATPTeamFightGameMode();

	//FTPOnHasNewPlayerJoin OnHasNewPlayerJoinEvent;

	void PostLogin(APlayerController* NewPlayer) override;

	void OnPostLogin(AController* NewPlayer) override;

	bool ShouldSpawnAtStartSpot(AController* Player) override { return false; };

	void PreInitializeComponents() override;

	void HandleMatchHasStarted() override;

	AActor* ChoosePlayerStart(AController* Player);

	void DefaultTimer();

	void PlayerHit(AController* Player, AActor* PlayerBeingHit, float Damage);

	float MatchTime;

	FTimerHandle DefaultTimerHandle;

	FOnGameModeCombinedPostLogin& OnGameModeCombinedPostLogin() { return OnGameModeCombinedPostLoginDelegate; }

private:
	FOnGameModeCombinedPostLogin OnGameModeCombinedPostLoginDelegate;

};

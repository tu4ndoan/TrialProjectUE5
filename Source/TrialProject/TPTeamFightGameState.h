// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "TPTeamFightGameState.generated.h"

/**
 * 
 */
UCLASS()
class TRIALPROJECT_API ATPTeamFightGameState : public AGameState
{
	GENERATED_BODY()

public:
	ATPTeamFightGameState();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int TeamAScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	int TeamBScore;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_TotalHits)
	int TotalHits;

	UFUNCTION()
	void OnRep_TotalHits();

	void PlayerHit();
};

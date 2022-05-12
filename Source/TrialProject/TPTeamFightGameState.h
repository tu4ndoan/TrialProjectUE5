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

protected:

	UPROPERTY(BlueprintReadOnly, Replicated)
	float TeamAScore;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float TeamBScore;

public:

	UFUNCTION(BlueprintCallable, Category = "TrialProject | Team")
	FORCEINLINE float GetTeamAScore() const { return TeamAScore; }

	UFUNCTION(BlueprintCallable, Category = "TrialProject | Team")
	FORCEINLINE float GetTeamBScore() const { return TeamBScore; }

	void SetTeamAScore(float Value);

	void SetTeamBScore(float Value);

};

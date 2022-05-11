// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "TPPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TRIALPROJECT_API ATPPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:

	ATPPlayerState();
	// player state to work with player's state in the game

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated)
	bool bTeamB;

	UFUNCTION()
	bool IsTeamB() const { return bTeamB; }
};

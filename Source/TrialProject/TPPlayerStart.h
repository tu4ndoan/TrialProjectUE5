// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "TPPlayerStart.generated.h"

/**
 * 
 */
UCLASS()
class TRIALPROJECT_API ATPPlayerStart : public APlayerStart
{
	GENERATED_BODY()

public:

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Defaults, Replicated)
	bool bTeamB;
};

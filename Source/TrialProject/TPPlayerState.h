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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bTeamB;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated)
	bool bIsDead;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float TotalSwingAttempt;

	UPROPERTY(BlueprintReadOnly, Replicated)
	float TotalHit;

public:

	/** Interface */

	UFUNCTION(BlueprintCallable, Category = "TrialProject | PlayerInfo")
	FORCEINLINE bool IsTeamB() const { return bTeamB; }

	void SetTeamB(bool IsTeamB);

	UFUNCTION(BlueprintCallable, Category = "TrialProject | PlayerInfo")
	FORCEINLINE	bool IsDead() const { return bIsDead; }

	void SetIsDead(bool IsDead);

	UFUNCTION(BlueprintCallable, Category = "TrialProject | PlayerInfo")
	FORCEINLINE	float GetTotalSwingAttempt() const { return TotalSwingAttempt; }

	void SetTotalSwingAttempt(float Value);

	UFUNCTION(BlueprintCallable, Category = "TrialProject | PlayerInfo")
	FORCEINLINE	float GetTotalHit() const { return TotalHit; }

	void SetTotalHit(float Value);

	/** End of Interface */

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "TrialProject | PlayerInfo")
	void TPSetPlayerName(const FString &InName);

};

// Fill out your copyright notice in the Description page of Project Settings.

#include "TPPlayerController.h"
#include "TPPlayerState.h"
#include "TPTeamFightGameMode.h"
#include "TrialProject.h"



void ATPPlayerController::OnKilled()
{
	UnPossess();
	GetWorld()->GetTimerManager().SetTimer(Respawn_TimerHandle, this, &ATPPlayerController::Respawn, 5.f, false);
}

void ATPPlayerController::Respawn()
{
	GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>()->RestartPlayer(this);
}

void ATPPlayerController::Test()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("match has ended"));
}
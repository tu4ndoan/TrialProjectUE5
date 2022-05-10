// Fill out your copyright notice in the Description page of Project Settings.

#include "TPPlayerController.h"
#include "TPTeamFightGameMode.h"
#include "TrialProject.h"



void ATPPlayerController::OnKilled()
{
	UnPossess();
	GetWorld()->GetTimerManager().SetTimer(Respawn_TimerHandle, this, &ATPPlayerController::Respawn, 5.f, false);
}

void ATPPlayerController::Respawn()
{
	ATPTeamFightGameMode* GM = GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>();
	if (GM)
	{
		APawn* NewPawn = GM->SpawnDefaultPawnFor(this, GM->ChoosePlayerStart(this));
		Possess(NewPawn);
	}
}
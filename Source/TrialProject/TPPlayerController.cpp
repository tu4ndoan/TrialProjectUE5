// Fill out your copyright notice in the Description page of Project Settings.

#include "TPPlayerController.h"
#include "TPPlayerState.h"
#include "TrialProjectCharacter.h"
#include "TPTeamFightGameMode.h"
#include "TrialProject.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"


void ATPPlayerController::BeginPlay()
{
	Super::BeginPlay();

}

void ATPPlayerController::OnKilled()
{
	UnPossess();
	GetWorld()->GetTimerManager().SetTimer(Respawn_TimerHandle, this, &ATPPlayerController::Respawn, 5.f, false);
}

void ATPPlayerController::Respawn()
{
	if (ATPTeamFightGameMode* GM = GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>())
	{
		GM->RestartPlayer(this);
	}
}
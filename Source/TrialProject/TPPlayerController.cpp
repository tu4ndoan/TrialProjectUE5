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

	/** get all actor of class ATrialProjectCharacter */
	for (TActorIterator<ATrialProjectCharacter> StartItr(GetWorld()); StartItr; ++StartItr)
	{
		if (StartItr)
		{
			if (UAnimMontage* CurrentMontage = StartItr->CurrentActiveMontage)
			{
				float CurrentMontage_Position = StartItr->CurrentActiveMontage_Position;
				if (CurrentMontage)
					StartItr->GetMesh()->GetAnimInstance()->Montage_Play(CurrentMontage, 1.0f, EMontagePlayReturnType::MontageLength, CurrentMontage_Position, true);

			}
		}
	}
}

void ATPPlayerController::OnKilled()
{
	UnPossess();
	GetWorld()->GetTimerManager().SetTimer(Respawn_TimerHandle, this, &ATPPlayerController::Respawn, 5.f, false);
}

void ATPPlayerController::Respawn()
{
	GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>()->RestartPlayer(this);
}
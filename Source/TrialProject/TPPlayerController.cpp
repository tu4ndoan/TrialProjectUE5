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
		if (GetWorld() == NULL)
			break;
		if (StartItr)
		{
			if (UAnimMontage* CurrentMontage = StartItr->CurrentActiveMontage)
			{
				FCharacterAnimation CharacterAnimation = StartItr->CharacterAnimationStruct;
				float CurrentMontagePosition = GetWorld()->GetTimeSeconds() - CharacterAnimation.StartTime;

				if (CharacterAnimation.Montage != NULL)
				{
					UE_LOG(LogTemp, Warning, TEXT("%s"), *CharacterAnimation.Montage->GetName());
				}

				if (CharacterAnimation.StartTime != NULL)
				{
					UE_LOG(LogTemp, Warning, TEXT("%f"), CharacterAnimation.StartTime);
				}

				//UE_LOG(LogTemp, Warning, TEXT("%s %f %f %f"), *CharacterAnimation.Montage->GetName(), GetWorld()->GetTimeSeconds(), CharacterAnimation.StartTime, CurrentMontagePosition);

				//StartItr->GetMesh()->GetAnimInstance()->Montage_Play(CharacterAnimation.Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.6f, true);
				/*
				if (UAnimMontage* CurrentMontage = StartItr->CurrentActiveMontage)
				{
					// only see animation, not the attack, but the sphere trace is server function so if you join before the sphere trace cast, you'll also get attacked
					StartItr->GetMesh()->GetAnimInstance()->Montage_Play(CurrentMontage, 1.0f, EMontagePlayReturnType::MontageLength, StartItr->CurrentActiveMontage_Position, true);
				}
				*/
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
	if (ATPTeamFightGameMode* GM = GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>())
	{
		GM->RestartPlayer(this);
	}
}
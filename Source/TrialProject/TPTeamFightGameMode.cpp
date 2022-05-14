// Fill out your copyright notice in the Description page of Project Settings.


#include "TPTeamFightGameMode.h"
#include "TPPlayerState.h"
#include "TPTeamFightGameState.h"
#include "TrialProjectCharacter.h"
#include "TPPlayerController.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TPPlayerStart.h"


ATPTeamFightGameMode::ATPTeamFightGameMode()
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	
	DefaultPawnClass = PlayerPawnClassFinder.Class;
	PlayerControllerClass = ATPPlayerController::StaticClass();
	PlayerStateClass = ATPPlayerState::StaticClass();
	GameStateClass = ATPTeamFightGameState::StaticClass();
	MatchTime = 60.f;
}

void ATPTeamFightGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		ATPPlayerState* PS = Cast<ATPPlayerState>(NewPlayer->PlayerState);
		if (PS && GameState)
		{
			uint8 NumTeamA = 0;
			uint8 NumTeamB = 0;
			for (APlayerState* It : GameState->PlayerArray)
			{
				ATPPlayerState* TPPS = Cast<ATPPlayerState>(It);
				if (TPPS)
				{
					if (TPPS->IsTeamB())
					{
						NumTeamB++;
					}
					else
					{
						NumTeamA++;
					}
				}
			}
			/** If members of team A is more than members of team B */ 
			if (NumTeamA > NumTeamB) 
			{
				PS->SetTeamB(true);
			}
			else {
				PS->SetTeamB(false);
			}
			/** End of If*/
		}
	}
}

AActor* ATPTeamFightGameMode::ChoosePlayerStart(AController* Player)
{
	if (Player)
	{
		ATPPlayerState* PS = Cast<ATPPlayerState>(Player->PlayerState);
		if (PS)
		{
			TArray<ATPPlayerStart*> Starts;
			for (TActorIterator<ATPPlayerStart> StartItr(GetWorld()); StartItr; ++StartItr)
			{
				if (StartItr->bTeamB == PS->IsTeamB())
				{
					Starts.Add(*StartItr);
				}
			}
			return Starts[FMath::RandRange(0, Starts.Num() - 1)];
		}
	}
	return NULL;
}

void ATPTeamFightGameMode::PlayerHit(AController* Player, AActor* PlayerBeingHit, float Damage)
{
	if (ATPTeamFightGameState* GS = GetGameState<ATPTeamFightGameState>())
	{
		if (ATPPlayerState* PS = Cast<ATPPlayerState>(Player->PlayerState))
		{
			PS->SetTotalHit(PS->GetTotalHit() + 1);
			PS->SetTotalDamageDone(PS->GetTotalDamageDone() + Damage); // TODO: recalculate damage if player's health before fatal hit is lower than the damage
			ATrialProjectCharacter* HitPlayer = Cast<ATrialProjectCharacter>(PlayerBeingHit);
			if (HitPlayer->GetPlayerState<ATPPlayerState>()->IsDead())
			{
				if (Cast<ATrialProjectCharacter>(PlayerBeingHit)->LastHitBy == Player)
				//if (Cast<ATrialProjectCharacter>(PlayerBeingHit)->Instigators.Last() == Player) 
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%s killed %s, +1 point for gryffindor"), *PS->GetPlayerNameCustom(), *HitPlayer->GetPlayerState<ATPPlayerState>()->GetPlayerNameCustom()));
					/** Set Score for Player who got the last hit, and set Score for the Team the Player was in */
					PS->SetScore(PS->GetScore() + 1.0f);
					if (PS->IsTeamB())
					{
						GS->SetTeamBScore(GS->GetTeamBScore() + 1.0f);
					}
					else
					{
						GS->SetTeamAScore(GS->GetTeamAScore() + 1.0f);
					}
				}
				
			}
		}
	}
}
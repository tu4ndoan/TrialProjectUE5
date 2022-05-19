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

void ATPTeamFightGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(DefaultTimerHandle, this, &ATPTeamFightGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void ATPTeamFightGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		// set team for new player
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
			if (NumTeamA > NumTeamB) 
			{
				PS->SetTeamB(true);
			}
			else {
				PS->SetTeamB(false);
			}

		}
	}
}

void ATPTeamFightGameMode::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	OnGameModeCombinedPostLoginDelegate.Broadcast();
}

void ATPTeamFightGameMode::DefaultTimer()
{
	ATPTeamFightGameState* const GS = GetGameState<ATPTeamFightGameState>();
	if (GS && GS->TimeRemaining > 0)
	{
		GS->TimeRemaining--;
		if (GS->TimeRemaining <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					ATPPlayerController* PlayerController = Cast<ATPPlayerController>(*It);

					if (PlayerController && GS)
					{
						ATPPlayerState* PlayerState = Cast<ATPPlayerState>((*It)->PlayerState);
						// TODO tell player who win the match
					}
				}
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

void ATPTeamFightGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	ATPTeamFightGameState* const MyGameState = Cast<ATPTeamFightGameState>(GameState);
	MyGameState->TimeRemaining = MatchTime;

	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		ATPPlayerController* PC = Cast<ATPPlayerController>(*It);
		if (PC)
		{
			// TODO: tell player match has started
		}
	}
}

AActor* ATPTeamFightGameMode::ChoosePlayerStart(AController* Player)
{
	/** Choose start for player according to player's team */
	if (Player)
	{
		ATPPlayerState* PS = Cast<ATPPlayerState>(Player->PlayerState);
		if (PS)
		{
			TArray<ATPPlayerStart*> Starts;
			/** loop all actors of class ATPPlayerStart in the world */
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
			PS->SetTotalDamageDone(PS->GetTotalDamageDone() + Damage);
			ATrialProjectCharacter* HitPlayer = Cast<ATrialProjectCharacter>(PlayerBeingHit);
			if (HitPlayer->GetPlayerState<ATPPlayerState>()->IsDead())
			{
				if (Cast<ATrialProjectCharacter>(PlayerBeingHit)->LastHitBy == Player) 
				{
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
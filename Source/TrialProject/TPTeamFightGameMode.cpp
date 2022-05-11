// Fill out your copyright notice in the Description page of Project Settings.


#include "TPTeamFightGameMode.h"
#include "TPPlayerState.h"
#include "TPTeamFightGameState.h"
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
					if (TPPS->bTeamB)
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
				PS->bTeamB = true;
			}
			else {
				PS->bTeamB = false;
			}
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
				if (StartItr->bTeamB == PS->bTeamB)
				{
					Starts.Add(*StartItr);
				}
			}
			return Starts[FMath::RandRange(0, Starts.Num() - 1)];
		}
	}
	return NULL;
}

void ATPTeamFightGameMode::PlayerHit()
{
	if (ATPTeamFightGameState* GS = GetGameState<ATPTeamFightGameState>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Player hit game mode"));
		GS->PlayerHit();
	}
}
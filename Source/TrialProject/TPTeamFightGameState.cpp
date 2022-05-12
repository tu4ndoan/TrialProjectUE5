// Fill out your copyright notice in the Description page of Project Settings.


#include "TPTeamFightGameState.h"
#include "TPPlayerState.h"
#include "Net/UnrealNetwork.h"

ATPTeamFightGameState::ATPTeamFightGameState()
{
	TeamAScore = 0;
	TeamBScore = 0;
}

void ATPTeamFightGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPTeamFightGameState, TeamAScore);
	DOREPLIFETIME(ATPTeamFightGameState, TeamBScore);
}

void ATPTeamFightGameState::SetTeamAScore(float Value)
{
	if (HasAuthority())
	{
		TeamAScore = Value;
	}
}

void ATPTeamFightGameState::SetTeamBScore(float Value)
{
	if (HasAuthority())
	{
		TeamBScore = Value;
	}
}
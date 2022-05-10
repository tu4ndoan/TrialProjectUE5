// Fill out your copyright notice in the Description page of Project Settings.


#include "TPTeamFightGameState.h"
#include "Net/UnrealNetwork.h"

ATPTeamFightGameState::ATPTeamFightGameState()
{
	TeamAScore = 0;
	TeamBScore = 10;
	TotalHits = 0;
}

void ATPTeamFightGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPTeamFightGameState, TotalHits);
}

void ATPTeamFightGameState::PlayerHit()
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("player hit game state"));
		++TotalHits;
		UE_LOG(LogTemp, Warning, TEXT("Server: total hits %d"), TotalHits);
	}
}

void ATPTeamFightGameState::OnRep_TotalHits()
{
	UE_LOG(LogTemp, Warning, TEXT("Client: total hits %d"), TotalHits);
}
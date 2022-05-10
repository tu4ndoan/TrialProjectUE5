// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerState.h"
#include "Net/UnrealNetwork.h"

ATPPlayerState::ATPPlayerState()
{
	bTeamB = false;
}

void ATPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerState, bTeamB);
}
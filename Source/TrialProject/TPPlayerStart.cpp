// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerStart.h"
#include "Net/UnrealNetwork.h"

void ATPPlayerStart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerStart, bTeamB);
}
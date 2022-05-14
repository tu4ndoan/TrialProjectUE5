// Fill out your copyright notice in the Description page of Project Settings.


#include "TPPlayerState.h"
#include "Net/UnrealNetwork.h"

ATPPlayerState::ATPPlayerState()
{
	bReplicates = true;
	bUseCustomPlayerNames = true;
	SetPlayerName("Default Name");

	/** Defaults */
	bTeamB = false;
	bIsDead = false;
	TotalSwingAttempt = 0.f;
	TotalHit = 0.f;
	TotalDamageDone = 0.f;
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, FString::Printf(TEXT("PlayerName is %s"), *GetPlayerNameCustom()));
}

void ATPPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATPPlayerState, bTeamB);
	DOREPLIFETIME(ATPPlayerState, bIsDead);
	DOREPLIFETIME(ATPPlayerState, TotalSwingAttempt);
	DOREPLIFETIME(ATPPlayerState, TotalHit);
	DOREPLIFETIME(ATPPlayerState, TotalDamageDone);
}

void ATPPlayerState::SetTeamB(bool IsTeamB)
{
	if (HasAuthority())
		bTeamB = IsTeamB;
}

void ATPPlayerState::SetIsDead(bool IsDead)
{
	if (HasAuthority())
		bIsDead = IsDead;
}

void ATPPlayerState::SetTotalSwingAttempt(float Value)
{
	if (HasAuthority())
		TotalSwingAttempt = Value;
}
void ATPPlayerState::SetTotalHit(float Value)
{
	if (HasAuthority())
		TotalHit = Value;
}

void ATPPlayerState::SetTotalDamageDone(float Value)
{
	if (HasAuthority())
		TotalDamageDone = Value;
}

void ATPPlayerState::TPSetPlayerName_Implementation(const FString &InName)
{
	/** Server set PLayerName and tell clients, Because for some reason the default SetPlayerName doesn't replicate */
	if (HasAuthority())
		SetPlayerName(InName);
}
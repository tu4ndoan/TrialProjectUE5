// Fill out your copyright notice in the Description page of Project Settings.


#include "TPHealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "TrialProjectCharacter.h"
#include "TPPlayerState.h"


UTPHealthComponent::UTPHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100;
	CurrentHealth = MaxHealth;
}

void UTPHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTPHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTPHealthComponent, MaxHealth);
	DOREPLIFETIME(UTPHealthComponent, CurrentHealth);
}

void UTPHealthComponent::SetCurrentHealth(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
		CurrentHealth = FMath::Clamp(Value, 0.f, MaxHealth);
}

void UTPHealthComponent::OnRep_CurrentHealth()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("HealthComponent: player %s is currently having %f HP"), *GetOwner()->GetName(), CurrentHealth));
	UE_LOG(LogTemp, Warning, TEXT("HealthComponent: player %s is currently having %f HP"), *GetOwner()->GetName(), CurrentHealth);
}

void UTPHealthComponent::OnHealthUpdate()
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		OnPlayerHealthChangedDelegate.Broadcast();
	}
}

void UTPHealthComponent::OnRep_MaxHealth()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("HealthComponent: player %s 's max HP is %f"), *GetOwner()->GetName(), MaxHealth));
	UE_LOG(LogTemp, Warning, TEXT("HealthComponent: player %s 's max HP is %f"), *GetOwner()->GetName(), MaxHealth);
}
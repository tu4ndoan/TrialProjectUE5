// Fill out your copyright notice in the Description page of Project Settings.


#include "TPHealthComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UTPHealthComponent::UTPHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	// ...
	MaxHealth = 100;
	CurrentHealth = MaxHealth;
}

/*
void UTPHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTPHealthComponent, CurrentHealth);
}
*/

// Called when the game starts
void UTPHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UTPHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (CurrentHealth <= 0)
	{
		return;
	}

	float damageApplied = CurrentHealth - Damage;
	SetCurrentHealth(damageApplied);
	UE_LOG(LogTemp, Warning, TEXT("HealthComponent | Client: Take damage %f, current health damage applied %f, current health %f/100"), Damage, damageApplied, CurrentHealth);
	return;
}

void UTPHealthComponent::SetCurrentHealth(float Value)
{
	if (GetOwnerRole() == ROLE_Authority)
		CurrentHealth = FMath::Clamp(Value, 0.f, MaxHealth);
}
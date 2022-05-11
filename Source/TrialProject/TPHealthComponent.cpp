// Fill out your copyright notice in the Description page of Project Settings.


#include "TPHealthComponent.h"

// Sets default values for this component's properties
UTPHealthComponent::UTPHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	//this->SetIsReplicated(true);
	// ...
	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	//SetIsReplicated(true);
}



// Called when the game starts
void UTPHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UTPHealthComponent::TakeDamage);
		UE_LOG(LogTemp, Warning, TEXT("HealthComp, takedamage added"));
	}
}

void UTPHealthComponent::TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - Damage;
	SetCurrentHealth(damageApplied);
	UE_LOG(LogTemp, Warning, TEXT("Client: Take damage %f, current health damage applied %f, current health %f/100"), Damage, damageApplied, CurrentHealth);
}

void UTPHealthComponent::SetCurrentHealth(float Value)
{
	CurrentHealth = FMath::Clamp(Value, 0.f, MaxHealth);
}
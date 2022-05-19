// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPHealthComponent.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRIALPROJECT_API UTPHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UTPHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	float MaxHealth;

	float CurrentHealth;

	UFUNCTION(BlueprintCallable, Category = "TrialProject | HealthComponent")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; };

	FORCEINLINE void SetCurrentHealth(float Value);

	void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};

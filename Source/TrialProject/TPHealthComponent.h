// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPHealthComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnPlayerHealthChanged);

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

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth, VisibleAnywhere, BlueprintReadOnly, Category = "TrialProject | HealthComponent")
	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, VisibleAnywhere, BlueprintReadOnly, Category = "TrialProject | HealthComponent")
	float CurrentHealth;

	FOnPlayerHealthChanged OnPlayerHealthChangedDelegate;

public:
	UFUNCTION(BlueprintCallable, Category = "TrialProject | HealthComponent")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; };

	UFUNCTION(BlueprintCallable, Category = "TrialProject | HealthComponent")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; };

	FOnPlayerHealthChanged& OnPlayerHealthChanged() { return OnPlayerHealthChangedDelegate; }

	void SetCurrentHealth(float Value);

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnHealthUpdate();

	UFUNCTION()
	void OnRep_MaxHealth();
};

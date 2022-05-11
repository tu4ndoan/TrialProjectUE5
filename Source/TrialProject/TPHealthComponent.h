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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trial Project Health Component")
	float MaxHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Trial Project Health Component")
	float CurrentHealth;

	UFUNCTION(BlueprintCallable, Category = "Trial Project Health Component")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; };

	UFUNCTION(BlueprintCallable, Category = "Trial Project Health Component")
	FORCEINLINE void SetCurrentHealth(float Value);

	UFUNCTION(BlueprintCallable)
	void TakeDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);
};

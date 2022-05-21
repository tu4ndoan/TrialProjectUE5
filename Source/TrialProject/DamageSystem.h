// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageSystem.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TRIALPROJECT_API UDamageSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDamageSystem();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	// base damage
	float BaseDamage;

	// extra damage from weapon
	float WeaponDamage;

	// damage type 
	TSubclassOf<UDamageType> DamageType;

	// apply damage 

	// take damage 

	// bind event on any damage

};

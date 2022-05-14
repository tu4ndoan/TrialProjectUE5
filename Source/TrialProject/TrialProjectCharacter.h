// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TPHealthComponent.h"
#include "TPWeapon.h"
#include "TrialProjectCharacter.generated.h"

UCLASS(config=Game)
class ATrialProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	
public:
	ATrialProjectCharacter();

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

protected:
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

/** Health System */

protected:

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "TrialProject | Health")
	void SetCurrentHealth(float healthValue);

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

public:

	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "TrialProject | Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "TrialProject | Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

/** End of Health System */

/** Anim Montage, Sound, Particles */

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* M_AttackPrimaryA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* M_AttackPrimaryB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* M_Dead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Animations)
	UAnimMontage* M_ReactToHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Sounds)
	USoundBase* S_AttackB;

/** end of Anim Montage, Sound, Particles */

/** Attack */

protected:

	FTimerHandle AttackHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Attacks, Replicated)
	bool bIsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPrimaryA, Category = Attacks)
	bool bAttackPrimaryA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPrimaryB, Category = Attacks)
	bool bAttackPrimaryB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Damage")
	float Damage;

	UFUNCTION(BlueprintCallable, Category = Attack)
	void SetAttacking(bool IsAttacking);

	UFUNCTION(BlueprintCallable, Category = Attack)
	void StopAttacking();

	// Attack A
	UFUNCTION(BlueprintCallable, Category = Attack)
	void StartAttackPrimaryA();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryA();

	UFUNCTION()
	void OnRep_AttackPrimaryA();

	// Attack B
	UFUNCTION(BlueprintCallable, Category = Attack)
	void StartAttackPrimaryB();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryB();

	UFUNCTION()
	void OnRep_AttackPrimaryB();

public:

	UFUNCTION(Server, Reliable)
	void ServerAttack(AActor* HitActor);

/** End of Attack */

/** Damage System */

protected:

	UFUNCTION(BlueprintCallable, Category = "TrialProject | Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable, Category = "TrialProject | Health")
	void PlayerDie();

public:

	UFUNCTION(Server, BlueprintCallable, Reliable)
	void SphereTrace(); // Attack A, trace a long sphere ahead, deal damage to enemy on the way (like a big thick linetrace)

	UFUNCTION(Server, Reliable)
	void SweepTrace(); // Attack B, trace a big sphere at player location, deal radial damage to enemy around and pushing enemy away

	TArray<AController*> Instigators;
	
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	//class URadialForceComponent* ForceComp;

/** End of Damage System */

};


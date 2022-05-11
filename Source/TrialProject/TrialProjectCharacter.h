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

	/** Health Component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = HealthComponent, meta = (AllowPrivateAccess = "true"))
	UTPHealthComponent* HealthComponent;
public:
	ATrialProjectCharacter();

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
	//ATPWeapon* Weapon;

	/** Property replication */
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

	/** Anim Montage, Sound, Particles */
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

	/** End of Attack */

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

	UFUNCTION(BlueprintCallable, Category = Attack)
	void SetAttacking(bool IsAttacking);

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

	UFUNCTION(BlueprintCallable, Category = Attack)
	void StopAttacking();

	UFUNCTION(BlueprintCallable, Category = Attack)
	void SphereTrace(); // Sphere Trace to notify about the hit

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/** Returns HealthComp subobject **/
	FORCEINLINE class UTPHealthComponent* GetHealthComponent() const { return HealthComponent; }
	/** Returns Sword subobject **/
	FORCEINLINE class UBoxComponent* GetSword() const { return Sword; }

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Weapon, Replicated)
	class UBoxComponent* Sword;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
	TSubclassOf<class ATPProjectile> ProjectileClass;

	/** Delay between shots in seconds. Used to control fire rate for our test projectile, but also to prevent an overflow of server functions from binding SpawnProjectile directly to input.*/
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay")
	float FireRate;

	/** If true, we are in the process of firing projectiles. */
	bool bIsFiringWeapon;

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StartFire();

	/** Function for ending weapon fire. Once this is called, the player can use StartFire again.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
	void StopFire();

	/** Server function for spawning projectiles.*/
	UFUNCTION(Server, Reliable)
	void HandleFire();

	/** A timer handle used for providing the fire rate delay in-between spawns.*/
	FTimerHandle FiringTimer;

	/** The player's maximum health. This is the highest that their health can be, and the value that their health starts at when spawned.*/
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHealth;

	/** The player's current health. When reduced to 0, they are considered dead.*/
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	/** RepNotify for changes made to current health.*/
	UFUNCTION()
	void OnRep_CurrentHealth();

	/** Response to health being updated. Called on the server immediately after modification, and on clients in response to a RepNotify*/
	void OnHealthUpdate();

public:
	/** Getter for Max Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }

	/** Getter for Current Health.*/
	UFUNCTION(BlueprintPure, Category = "Health")
	FORCEINLINE float GetCurrentHealth() const { return CurrentHealth; }

	/** Setter for Current Health. Clamps the value between 0 and MaxHealth and calls OnHealthUpdate. Should only be called on the server.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetCurrentHealth(float healthValue);

	/** Event for taking damage. Overridden from APawn.*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

};


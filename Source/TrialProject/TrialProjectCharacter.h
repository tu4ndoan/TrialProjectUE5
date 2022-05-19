// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TPHealthComponent.h"
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

	float MaxHealth;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;

	void SetCurrentHealth(float healthValue);

	UFUNCTION()
	void OnRep_CurrentHealth();

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimMontage* M_AttackPrimaryA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimMontage* M_AttackPrimaryB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimMontage* M_Dead;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimMontage* M_ReactToHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimationAsset* Anim_Death;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	UAnimationAsset* Anim_OnHit;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TrialProject | Effects")
	USoundBase* S_AttackB;

/** end of Anim Montage, Sound, Particles */

/** Attack */

protected:

	FTimerHandle AttackHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Attacks")
	bool bIsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPrimaryA, Category = "TrialProject | Attacks")
	bool bAttackPrimaryA;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPrimaryB, Category = "TrialProject | Attacks")
	bool bAttackPrimaryB;

	TSubclassOf<UDamageType> DamageType;

	float Damage;

	void SetAttacking(bool IsAttacking);

	void StopAttacking();

	// Attack A
	void StartAttackPrimaryA();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryA();

	UFUNCTION()
	void OnRep_AttackPrimaryA();

	// Attack B
	void StartAttackPrimaryB();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryB();

	UFUNCTION()
	void OnRep_AttackPrimaryB();

/** End of Attack */

/** Damage System */

protected:

	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(NetMulticast, Reliable, Category = "TrialProject | Health")
	void PlayerDie();

public:

	UFUNCTION(Server, Reliable)
	void SphereTrace();

	UFUNCTION(Server, Reliable)
	void SweepTrace();

/** End of Damage System */

	UFUNCTION(NetMulticast, Reliable)
	void NMC_PlayAnimMontage(UAnimMontage* InAnimMontage);

	UFUNCTION(NetMulticast, Reliable)
	void NMC_PlayAnimation(UAnimationAsset* InAnimationAsset);

/** Animation */

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Animation")
	UAnimMontage* CurrentActiveMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Animation")
	float CurrentActiveMontage_Position; // the current state of the animation playing

	UFUNCTION(Server, Reliable)
	void UpdateCurrentActiveMontage();

/** End of Animation */
};


// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "TPHealthComponent.h"
#include "TrialProjectCharacter.generated.h"


USTRUCT(BlueprintType)
struct FCharacterAnimation
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	UAnimMontage* Montage;

	UPROPERTY(BlueprintReadOnly)
	float StartTime;
};

USTRUCT(BlueprintType)
struct FCharacterSkill
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FString SkillID;

	UPROPERTY(BlueprintReadOnly)
	UAnimMontage* Montage;

	UPROPERTY(BlueprintReadOnly)
	FVector Start;

	UPROPERTY(BlueprintReadOnly)
	FVector End;

	UPROPERTY(BlueprintReadOnly)
	float Radius;
};

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

/** Anim Montage, Sound, Particles */

public:

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
	FTimerHandle RespawnTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Attacks")
	bool bIsAttacking;

	void CastLineTrace();

	void CastSphereTrace();

	void SetAttacking(bool IsAttacking);

	void StopAttacking();

	// Attack A
	void StartAttackPrimaryA();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryA(class UAnimMontage* MontageToPlay);

	// Attack B
	void StartAttackPrimaryB();

	UFUNCTION(Server, Reliable)
	void AttackPrimaryB(class UAnimMontage* MontageToPlay);

/** End of Attack */

/** Damage System */

protected:

	float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	TSubclassOf<UDamageType> DamageType;

	float Damage;

	void RequestRespawn();

/** End of Damage System */

/** Animation */

public:
	UPROPERTY(ReplicatedUsing = OnRep_CharacterAnimation)
	bool bRepCharacterAnimationStruct;

	UPROPERTY(Replicated)
	FCharacterAnimation CharacterAnimationStruct;

	UFUNCTION()
	void OnRep_CharacterAnimation();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Animation")
	UAnimMontage* CurrentActiveMontage;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Replicated, Category = "TrialProject | Animation")
	float CurrentActiveMontage_Position; // the current state of the animation playing

private:

	UFUNCTION(Server, Reliable)
	void UpdateCurrentActiveMontage();

	/** multicast function called by server functions AttackPrimaryA and AttackPrimaryB, play effects (animation, sound, particle) for the attacks */
	UFUNCTION(NetMulticast, Reliable)
	void NMC_PlayAnimMontage(UAnimMontage* InAnimMontage);

/** End of Animation */

public:
/** Health System */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "TrialProject | Health Component")
	class UTPHealthComponent* HealthComponent;
	/** handle player-side logic for health system (like animation, display) */
	UFUNCTION()
	void OnHealthUpdate();

	UFUNCTION(BlueprintImplementableEvent)
	void SetHealthBarPercent(float InCurrentHealth);

	UFUNCTION(NetMulticast, Reliable, Category = "TrialProject | Animation")
	void PlayerDie();
/** End of Health System */
};


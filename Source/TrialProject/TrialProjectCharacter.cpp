// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrialProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/BoxComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "TPTeamFightGameMode.h"
#include "TPPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TPPlayerState.h"
#include "TPProjectile.h"

//////////////////////////////////////////////////////////////////////////
// ATrialProjectCharacter

ATrialProjectCharacter::ATrialProjectCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// health system
	//HealthComponent = CreateDefaultSubobject<UTPHealthComponent>(TEXT("HealthComp"));
	//HealthComponent->SetIsReplicated(true);
	//bReplicates = true;
	
	/* Box Collision for the Sword
	Sword = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	USkeletalMeshComponent* mesh = Cast<USkeletalMeshComponent>(GetMesh());
	Sword->SetupAttachment(Cast<USceneComponent>(mesh));
	Sword->SetupAttachment(GetMesh(), TEXT("sword_bottom"));
	*/

	/** Default Variables */
	// Player Attack
	bIsAttacking = false;
	bAttackPrimaryA = false;
	bAttackPrimaryB = false;

	// Player Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;

	// TODO Player Mana

	DamageType = UDamageType::StaticClass();
	DamageType.GetDefaultObject()->DamageImpulse = 10000;
	Damage = 10.0f;
}

//////////////////////////////////////////////////////////////////////////
// Input

void ATrialProjectCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ATrialProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ATrialProjectCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ATrialProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ATrialProjectCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ATrialProjectCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ATrialProjectCharacter::TouchStopped);

	// custom
	PlayerInputComponent->BindAction("AttackPrimaryA", IE_Pressed, this, &ATrialProjectCharacter::StartAttackPrimaryA);
	PlayerInputComponent->BindAction("AttackPrimaryB", IE_Pressed, this, &ATrialProjectCharacter::StartAttackPrimaryB);
}

void ATrialProjectCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATrialProjectCharacter, CurrentHealth);
	DOREPLIFETIME(ATrialProjectCharacter, bIsAttacking);
	DOREPLIFETIME(ATrialProjectCharacter, bAttackPrimaryA);
	DOREPLIFETIME(ATrialProjectCharacter, bAttackPrimaryB);
}

void ATrialProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ATrialProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ATrialProjectCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATrialProjectCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ATrialProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ATrialProjectCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ATrialProjectCharacter::SetAttacking(bool IsAttacking)
{
	if (HasAuthority())
	{
		bIsAttacking = IsAttacking;
	}
}


// Attack A, using RepNotify on bAttackPrimaryA
void ATrialProjectCharacter::StopAttacking()
{
	SetAttacking(false);
}

void ATrialProjectCharacter::StartAttackPrimaryA()
{
	if (!bIsAttacking)
	{
		AttackPrimaryA();
		SphereTrace();
	}
}

void ATrialProjectCharacter::AttackPrimaryA_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bAttackPrimaryA = !bAttackPrimaryA;
		SetAttacking(true);
		OnRep_AttackPrimaryA();
	}
}

void ATrialProjectCharacter::OnRep_AttackPrimaryA()
{
	float AnimDuration = PlayAnimMontage(M_AttackPrimaryA, 1.f, NAME_None);
	LaunchCharacter(RootComponent->GetForwardVector() * 1000.0f, false, false);
	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
	if (S_AttackB != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, S_AttackB, GetActorLocation());
}

// Attack B
void ATrialProjectCharacter::StartAttackPrimaryB()
{
	if (!bIsAttacking)
	{
		AttackPrimaryB();
		SweepTrace();
	}
}

void ATrialProjectCharacter::AttackPrimaryB_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bAttackPrimaryB = !bAttackPrimaryB;
		SetAttacking(true);
		OnRep_AttackPrimaryB();
	}
}

void ATrialProjectCharacter::OnRep_AttackPrimaryB()
{
	float AnimDuration = PlayAnimMontage(M_AttackPrimaryB, 1.f, NAME_None);
	if (S_AttackB != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, S_AttackB, GetActorLocation());
	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
}


/** Sphere Trace */
void ATrialProjectCharacter::SphereTrace_Implementation()
{
	if (HasAuthority())
	{
		// increase swing attempt by 1
		ATPPlayerState* PS = GetPlayerState<ATPPlayerState>();
		if (PS)
			PS->SetTotalSwingAttempt(PS->GetTotalSwingAttempt() + 1);

		// Set overlap pawn to allow player to dash through another player
		//GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		// cast a trace to the world to retrieve hit
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation() + (RootComponent->GetForwardVector() * 500.f);
		float Radius = 50.f;
		TArray<FHitResult> HitArray;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		const bool bHitSomething = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, HitArray, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
		if (bHitSomething)
		{
			for (const FHitResult HitResult : HitArray)
			{
				if (HitResult.GetActor() != this)
				{
					/** If hit a character */
					if (ATrialProjectCharacter* HitActor = Cast<ATrialProjectCharacter>(HitResult.GetActor()))
					{
						ATPPlayerState* HitActorState = HitActor->GetPlayerState<ATPPlayerState>();
						/** If HitActor is not dead and is not teammate*/
						if (HitActorState->IsTeamB() != GetPlayerState<ATPPlayerState>()->IsTeamB())
						{
							if (HitActorState->IsDead() == false)
							{
								UGameplayStatics::ApplyDamage(HitActor, Damage, GetInstigator()->GetController(), this, DamageType);
								ATPTeamFightGameMode* GM = GetWorld() != NULL ? GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>() : NULL;
								if (GM)
								{
									GM->PlayerHit(Controller, HitActor, Damage);
								}
							}
						}
					}
				}
			}
		}
		//GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	}
}
/** End of Sphere Trace */

/** Sphere Trace */

void ATrialProjectCharacter::SweepTrace_Implementation()
{
	if (HasAuthority())
	{
		ATPPlayerState* PS = GetPlayerState<ATPPlayerState>();
		if (PS)
			PS->SetTotalSwingAttempt(PS->GetTotalSwingAttempt() + 1);
		
		FVector Start = GetActorLocation();
		FVector End = GetActorLocation();
		float Radius = 500.f;
		TArray<FHitResult> HitArray;
		TArray<AActor*> ActorsToIgnore;
		ActorsToIgnore.Add(this);

		float InnerRadius = 100.f;
		float OuterRadius = 500.f;
		float FallOffRate = 10.f;
		float MinimumDamage = 5.f;
		float BaseDamage = Damage;
		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, MinimumDamage, Start, InnerRadius, OuterRadius, FallOffRate, DamageType, ActorsToIgnore, this, GetInstigator()->GetController(), ECC_Visibility);
		const bool bHitSomething = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::None, HitArray, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
		if (bHitSomething)
		{		
			for (const FHitResult HitResult : HitArray)
			{
				if (HitResult.GetActor() != this)
				{
					/** If hit a character */
					if (ATrialProjectCharacter* HitActor = Cast<ATrialProjectCharacter>(HitResult.GetActor()))
					{
						ATPPlayerState* HitActorState = HitActor->GetPlayerState<ATPPlayerState>();
						/** If HitActor is not dead and is not teammate*/
						if (HitActorState->IsTeamB() != GetPlayerState<ATPPlayerState>()->IsTeamB())
						{
							if (HitActorState->IsDead() == false)
							{
								
								
								
								//UGameplayStatics::ApplyDamage(HitActor, Damage, GetInstigator()->GetController(), this, DamageType);
								ATPTeamFightGameMode* GM = GetWorld() != NULL ? GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>() : NULL;
								if (GM)
								{
									GM->PlayerHit(Controller, HitActor, Damage);
								}
							}
						}
					}
				}
			}
		}
	}
}

/** End of Sphere Trace */

void ATrialProjectCharacter::OnHealthUpdate()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		//FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, healthMessage);
		if (CurrentHealth <= 0)
		{
			GetPlayerState<ATPPlayerState>()->SetIsDead(true);
			
		}
	}
}

void ATrialProjectCharacter::OnRep_CurrentHealth()
{
	OnHealthUpdate();
}

void ATrialProjectCharacter::SetCurrentHealth(float healthValue)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(healthValue, 0.f, MaxHealth);
		OnHealthUpdate();
	}
}

float ATrialProjectCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!ShouldTakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser))
	{
		return 0.f;
	}
	// calculate actual damage
	const float ActualDamage = Super::TakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser);

	float damageApplied = CurrentHealth - ActualDamage;
	SetCurrentHealth(damageApplied);
	if (damageApplied == 0)
	{
		PlayerDie();
	}

	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("%s took %f damage, current health %f/100"), *GetPlayerState<ATPPlayerState>()->GetPlayerNameCustom(), DamageTaken, GetCurrentHealth()));
	
	return damageApplied;
}

void ATrialProjectCharacter::PlayerDie_Implementation()
{
	//PlayAnimMontage(M_Dead);
	GetMovementComponent()->Deactivate();
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	GetMesh()->SetSimulatePhysics(true);
	if (ATPPlayerController* PC = Cast<ATPPlayerController>(Controller))
	{
		//PC->OnKilled();
	}
}

void ATrialProjectCharacter::ServerAttack_Implementation(AActor* HitActor)
{
	if (HasAuthority())
	{
		// TODO: dynamic attack system
	}
}

// player cast sphere trace locally
// if spheretrace has valid enemy hit
	// call Server_SphereTrace (HitActor, HitInstigator, Damage)
	// or just call Server_DealDamage with validation(if player actually hit someone, validate with another sphere trace?)

void ATrialProjectCharacter::TakeRadialDamage()
{
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Take radial damage"));
}
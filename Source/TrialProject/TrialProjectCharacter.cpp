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

	bIsAttacking = false;
	bAttackPrimaryA = false;
	bAttackPrimaryB = false;
	bReplicates = true;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;

	Sword = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	USkeletalMeshComponent* mesh = Cast<USkeletalMeshComponent>(GetMesh());
	Sword->SetupAttachment(Cast<USceneComponent>(mesh));
	Sword->SetupAttachment(GetMesh(), TEXT("sword_bottom"));
	// health system
	//HealthComponent = CreateDefaultSubobject<UTPHealthComponent>(TEXT("HealthComp"));
	// damage system

	//Initialize projectile class
	ProjectileClass = ATPProjectile::StaticClass();
	//Initialize fire rate
	FireRate = 0.25f;
	bIsFiringWeapon = false;
	//Initialize the player's Health
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ATrialProjectCharacter::StartFire);
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
	AttackPrimaryA();
	//SphereTrace();
}

void ATrialProjectCharacter::AttackPrimaryA_Implementation()
{
	
	if (IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("locally controlled"));
	}
	if (GetLocalRole() == ROLE_Authority)
	{
		bAttackPrimaryA = !bAttackPrimaryA;
		SetAttacking(true);
		OnRep_AttackPrimaryA(); // if dedicated server, dont call this line
	}
}

void ATrialProjectCharacter::OnRep_AttackPrimaryA()
{
	float AnimDuration = PlayAnimMontage(M_AttackPrimaryA, 1.f, NAME_None);
	if (S_AttackB != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, S_AttackB, GetActorLocation());
	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
}

// Attack B
void ATrialProjectCharacter::StartAttackPrimaryB()
{
	AttackPrimaryB();
	SphereTrace();
}

void ATrialProjectCharacter::AttackPrimaryB_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bAttackPrimaryB = !bAttackPrimaryB;
		OnRep_AttackPrimaryB();
	}
}

void ATrialProjectCharacter::OnRep_AttackPrimaryB()
{
	// play visual and sound effects of the attack
	// TODO make it dynamic so we can pass in any other attack to use without creating more functions
	float AnimDuration = PlayAnimMontage(M_AttackPrimaryB, 1.f, NAME_None);
	if (S_AttackB != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, S_AttackB, GetActorLocation());
	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
}

void ATrialProjectCharacter::SphereTrace_Implementation()
{
	TArray<AActor*> ActorsToIgnore;
	TArray <AActor*> ChildActors;
	GetAllChildActors(ChildActors, true);
	ActorsToIgnore += ChildActors;
	ActorsToIgnore.Add(this);
	TArray<FHitResult> HitArray;
	FVector EndLocation = GetActorLocation() + (RootComponent->GetForwardVector() * 120.f);
	const bool bHitSomething = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), GetActorLocation(), EndLocation, 50.f, UEngineTypes::ConvertToTraceType(ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitArray, true, FLinearColor::Red, FLinearColor::Green, 60.0f);
	if (bHitSomething)
	{
		for (const FHitResult HitResult : HitArray)
		{
			// how to know if we hit a teammate or enemy?
			if (HitResult.GetActor() != this)
			{
				if (ATrialProjectCharacter* HitActor = Cast<ATrialProjectCharacter>(HitResult.GetActor()))
				{
					if (HitActor->GetPlayerState<ATPPlayerState>()->IsTeamB() != GetPlayerState<ATPPlayerState>()->IsTeamB()) {
						GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Hit enemy")));
						//float TakeDamage = HitActor->TakeDamage(10.f, FDamageEvent(UDamageType::StaticClass()), GetInstigator()->GetController(), this);
						if (HasAuthority())
						{
							ATPTeamFightGameMode* GM = GetWorld() != NULL ? GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>() : NULL;
							if (GM)
							{
								GM->PlayerHit();
							}
						}
						UGameplayStatics::ApplyDamage(HitActor, 14.0f, GetInstigator()->GetController(), this, DamageType);
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Hit friendly")));
					}
				}

				//UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetInstigator()->GetController(), this, DamageType);		
				GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Hit other: %s"), *HitResult.GetActor()->GetName()));
			}
		}
	}
}

void ATrialProjectCharacter::StartFire()
{
	if (!bIsFiringWeapon)
	{
		bIsFiringWeapon = true;
		UWorld* World = GetWorld();
		World->GetTimerManager().SetTimer(FiringTimer, this, &ATrialProjectCharacter::StopFire, FireRate, false);
		HandleFire();
	}
}

void ATrialProjectCharacter::StopFire()
{
	bIsFiringWeapon = false;
}

void ATrialProjectCharacter::HandleFire_Implementation()
{
	FVector spawnLocation = GetActorLocation() + (GetControlRotation().Vector() * 200.0f) + (GetActorUpVector() * 50.0f);
	FRotator spawnRotation = GetControlRotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	ATPProjectile* spawnedProjectile = GetWorld()->SpawnActor<ATPProjectile>(spawnLocation, spawnRotation, spawnParameters);
}

void ATrialProjectCharacter::OnHealthUpdate()
{
	//Client-specific functionality
	if (IsLocallyControlled())
	{
		FString healthMessage = FString::Printf(TEXT("You now have %f health remaining."), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, healthMessage);
		if (CurrentHealth <= 0)
		{
			FString deathMessage = FString::Printf(TEXT("You have been killed."));
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, deathMessage);
			ATPPlayerController* PC = GetController() != NULL ? Cast<ATPPlayerController>(GetController()) : NULL;
			if (PC != NULL)
				PC->OnKilled();
		}
	}

	//Server-specific functionality
	if (GetLocalRole() == ROLE_Authority)
	{
		FString healthMessage = FString::Printf(TEXT("%s now has %f health remaining."), *GetFName().ToString(), CurrentHealth);
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, healthMessage);
	}

	//Functions that occur on all machines. 
	/*
		Any special functionality that should occur as a result of damage or death should be placed here.
	*/
	if (CurrentHealth <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("current health = 0, ded"));
		PlayAnimMontage(M_Dead);
		ATPPlayerController* PC = GetController() == NULL ? Cast<ATPPlayerController>(Controller) : NULL;
		if (PC != NULL)
		{
			PC->OnKilled();
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
	float damageApplied = CurrentHealth - DamageTaken;
	SetCurrentHealth(damageApplied);
	float AnimDuration = PlayAnimMontage(M_ReactToHit);
	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
	
	UE_LOG(LogTemp, Warning, TEXT("Took %f damage, current health %f/100"), DamageTaken, GetCurrentHealth());

	return damageApplied;
}
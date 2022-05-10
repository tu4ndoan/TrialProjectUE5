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

	this->bGenerateOverlapEventsDuringLevelStreaming = true;

	MaxHealth = 100.f;
	CurrentHealth = MaxHealth;

	bIsAttacking = false;
	bAttackPrimaryA = false;
	bAttackPrimaryB = false;

	//GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ATrialProjectCharacter::OnPlayerOverlap);
	Sword = CreateDefaultSubobject<UBoxComponent>(TEXT("Sword"));
	FName FNWeaponSocket = TEXT("sword_bottom");
	USkeletalMeshComponent* mesh = Cast<USkeletalMeshComponent>(GetMesh());
	//const USkeletalMeshSocket* socket = mesh->GetSocketByName("sword_bottom");
	Sword->SetupAttachment(Cast<USceneComponent>(mesh));
	Sword->SetupAttachment(GetMesh(), TEXT("sword_bottom"));
	Sword->OnComponentBeginOverlap.AddDynamic(this, &ATrialProjectCharacter::OnPlayerOverlap);
	Sword->bReplicatePhysicsToAutonomousProxy = true;
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

void ATrialProjectCharacter::SetCurrentHealth(float Value)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		CurrentHealth = FMath::Clamp(Value, 0.f, MaxHealth);
		OnRep_CurrentHealth(); // if dedicated server, dont call this line
	}
}

void ATrialProjectCharacter::OnRep_CurrentHealth()
{
	PlayAnimMontage(M_ReactToHit, 1.f, NAME_None);
	if (CurrentHealth <= 0)
	{
		ATPPlayerController* PC = Cast<ATPPlayerController>(Controller);
		if (PC)
		{
			PC->OnKilled();
			PlayAnimMontage(M_Dead, 1.f, NAME_None);
		}
		Destroy();
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
}

void ATrialProjectCharacter::AttackPrimaryA_Implementation()
{
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
	bIsAttacking = true;
	float AnimDuration = PlayAnimMontage(M_AttackPrimaryB, 1.f, NAME_None);

	if (S_AttackB != NULL)
		UGameplayStatics::PlaySoundAtLocation(this, S_AttackB, GetActorLocation());

	GetWorld()->GetTimerManager().SetTimer(AttackHandle, this,  &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
}

void ATrialProjectCharacter::OnPlayerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	// if you hit someone, tell server to apply damage and record the hit
	//UGameplayStatics::ApplyPointDamage(OtherActor, 10.f, FVector::Zero(), Hit, GetInstigator()->Controller, this, UDamageType::StaticClass());
		if (const UBoxComponent* MyBox = Cast<UBoxComponent>(OverlappedComponent))
		{
			if (OtherActor != NULL && OtherActor != this && MyBox->GetName() == "Sword")
			{
				if (GetLocalRole() == ROLE_Authority)
				{
					if (ATPTeamFightGameMode* GM = GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>())
					{
						GM->PlayerHit();
						UE_LOG(LogTemp, Warning, TEXT("has GM"));
					}
					
				}
				
				UGameplayStatics::ApplyDamage(OtherActor, 10.f, GetInstigator()->Controller, this, UDamageType::StaticClass());
				//UGameplayStatics::ApplyPointDamage(OtherActor, 10.f, FVector::Zero(), Hit, GetInstigator()->Controller, this, UDamageType::StaticClass());
				UE_LOG(LogTemp, Warning, TEXT("apply damage"));
			}
		}
}

float ATrialProjectCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float damageApplied = CurrentHealth - DamageAmount;
	SetCurrentHealth(damageApplied);
	UE_LOG(LogTemp, Warning, TEXT("take damage %d"), DamageAmount);
	return damageApplied;
}

void ATrialProjectCharacter::ActivateSword()
{
	Sword->SetCollisionProfileName(TEXT("OverlapAll"));
	UE_LOG(LogTemp, Warning, TEXT("enable collision"));
}

void ATrialProjectCharacter::DeactivateSword()
{
	Sword->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UE_LOG(LogTemp, Warning, TEXT("disable collision"));
}
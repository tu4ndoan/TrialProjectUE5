// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrialProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "TPTeamFightGameMode.h"
#include "TPPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TPPlayerState.h"


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
	CameraBoom->TargetArmLength = 600.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	/** Default Variables */
	// Player Attack
	bIsAttacking = false;

	// Health Component
	HealthComponent = CreateDefaultSubobject<UTPHealthComponent>(TEXT("HealthComponent"));

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;

	if (HasAuthority())
	{
		HealthComponent->OnPlayerHealthChanged().AddUFunction(this, FName("OnHealthUpdate"));

		if (ATPTeamFightGameMode* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>() : NULL)
			GM->OnGameModeCombinedPostLogin().AddUFunction(this, FName("UpdateCurrentActiveMontage"));
	}
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

	DOREPLIFETIME(ATrialProjectCharacter, bIsAttacking);
	DOREPLIFETIME(ATrialProjectCharacter, CurrentActiveMontage);
	DOREPLIFETIME(ATrialProjectCharacter, CurrentActiveMontage_Position);
	DOREPLIFETIME(ATrialProjectCharacter, CharacterAnimationStruct);
	DOREPLIFETIME(ATrialProjectCharacter, bRepCharacterAnimationStruct);
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

// Attack A
void ATrialProjectCharacter::StopAttacking()
{
	if(HasAuthority())
		SetAttacking(false);
}

void ATrialProjectCharacter::StartAttackPrimaryA()
{
	if (!bIsAttacking && !GetMovementComponent()->IsFalling())
	{
		AttackPrimaryA(M_AttackPrimaryA);
	}
}

void ATrialProjectCharacter::AttackPrimaryA_Implementation(class UAnimMontage* MontageToPlay)
{
	if (HasAuthority())
	{
		SetAttacking(true);
		LaunchCharacter(RootComponent->GetForwardVector() * 1500.0f, false, false); // dash forward
		CastLineTrace(); // cast line trace along the dash, see what we hit
		//NMC_PlayAnimMontage(M_AttackPrimaryA);

		CharacterAnimationStruct.Montage = MontageToPlay;
		CharacterAnimationStruct.StartTime = GetWorld()->GetTimeSeconds();
		bRepCharacterAnimationStruct = !bRepCharacterAnimationStruct;
		OnRep_CharacterAnimation();
	}
}

// Attack B
void ATrialProjectCharacter::StartAttackPrimaryB()
{
	if (!bIsAttacking)
	{
		AttackPrimaryB(M_AttackPrimaryB);
	}
}

void ATrialProjectCharacter::AttackPrimaryB_Implementation(class UAnimMontage* MontageToPlay)
{
	if (HasAuthority())
	{
		SetAttacking(true);
		CastSphereTrace();
		//NMC_PlayAnimMontage(M_AttackPrimaryB);
		CharacterAnimationStruct.Montage = MontageToPlay;
		CharacterAnimationStruct.StartTime = GetWorld()->GetTimeSeconds();
		bRepCharacterAnimationStruct = !bRepCharacterAnimationStruct;
		OnRep_CharacterAnimation();
	}
}

void ATrialProjectCharacter::CastLineTrace()
{
	ATPPlayerState* PS = GetPlayerState<ATPPlayerState>();
	if (PS == NULL)
	{
		return;
	}

	// cast a trace to the world to retrieve hit
	FVector Start = GetActorLocation();
	FVector End = GetActorLocation() + (RootComponent->GetForwardVector() * 650.f);
	float Radius = 5.f;
	TArray<FHitResult> HitArray;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	const bool bLineTraceHasHit = UKismetSystemLibrary::LineTraceMulti(GetWorld(), Start, End, UEngineTypes::ConvertToTraceType(ECC_Visibility), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitArray, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
	if (bLineTraceHasHit)
	{
		for (const FHitResult HitResult : HitArray)
		{
			/** If hit a character */
			if (ATrialProjectCharacter* HitActor = Cast<ATrialProjectCharacter>(HitResult.GetActor()))
			{
				/** If HitActor is not dead and is not teammate */
				UGameplayStatics::ApplyDamage(HitActor, Damage, GetInstigator()->GetController(), this, DamageType);
			}
		}
	}
	else
	{
		PS->SetTotalSwingAttempt(PS->GetTotalSwingAttempt() + 1);
	}
}


void ATrialProjectCharacter::CastSphereTrace()
{
	ATPPlayerState* PS = GetPlayerState<ATPPlayerState>();
	if (PS == NULL)
	{
		return;
	}

	FVector Start = GetActorLocation();
	FVector End = GetActorLocation();
	float Radius = 500.f;
	TArray<FHitResult> HitArray;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	// cast a sphere trace to the world and see if we hit something
	const bool bSphereTraceHasHit = UKismetSystemLibrary::SphereTraceMulti(GetWorld(), Start, End, Radius, UEngineTypes::ConvertToTraceType(ECC_Camera), false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitArray, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
	if (bSphereTraceHasHit)
	{
		// params for radial damage and impulse
		float InnerRadius = 100.f;	// radius to take max damage
		float OuterRadius = Radius;	// radius to take less damage
		float FallOffRate = 1.f;	// 1 means linear
		float MinimumDamage = 1.f;
		float BaseDamage = Damage;	// max damage = 10 when enemy is in inner radius
		/** deal radial damage and then push other player away with linear falloff */
		UGameplayStatics::ApplyRadialDamageWithFalloff(GetWorld(), BaseDamage, MinimumDamage, Start, InnerRadius, OuterRadius, FallOffRate, DamageType, ActorsToIgnore, this, GetInstigator()->GetController(), ECC_Visibility);
		for (const FHitResult HitResult : HitArray)
		{
			/** If hit a character */
			if (ATrialProjectCharacter* HitActor = Cast<ATrialProjectCharacter>(HitResult.GetActor()))
			{
				HitActor->GetMovementComponent()->AddRadialImpulse(Start, 500.f, 2000.f, ERadialImpulseFalloff::RIF_Linear, true);
			}
		}
	}
	else
	{
		// still increase attempts if we cant hit an enemy
		PS->SetTotalSwingAttempt(PS->GetTotalSwingAttempt() + 1);
	}
}


void ATrialProjectCharacter::OnHealthUpdate()
{
	UE_LOG(LogTemp, Warning, TEXT("CALLED"));
	if (HealthComponent == NULL)
	{
		UE_LOG(LogTemp, Warning, TEXT("HEALTH COMPONENT NULL"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("HEALTH COMPONENT"));

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("1"));
		if (HealthComponent->GetCurrentHealth() <= 0)
		{
			GetPlayerState<ATPPlayerState>()->SetIsDead(true);
			PlayerDie();
			UE_LOG(LogTemp, Warning, TEXT("2"));
		}
		
	}
	
	UE_LOG(LogTemp, Warning, TEXT("%f"), HealthComponent->GetCurrentHealth());
	UE_LOG(LogTemp, Warning, TEXT("%f"), HealthComponent->CurrentHealth);
	SetHealthBarPercent(HealthComponent->GetCurrentHealth());
}

float ATrialProjectCharacter::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!ShouldTakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser))
	{
		return 0.f;
	}

	if (GetPlayerState<ATPPlayerState>()->IsTeamB() == EventInstigator->GetPlayerState<ATPPlayerState>()->IsTeamB())
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(DamageTaken, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage <= 0.f)
	{
		return 0.f;
	}

	float DamageApplied = HealthComponent->GetCurrentHealth() - ActualDamage;

	UE_LOG(LogTemp, Warning, TEXT("%f"), HealthComponent->GetCurrentHealth());
	UE_LOG(LogTemp, Warning, TEXT("%f"), ActualDamage);
	UE_LOG(LogTemp, Warning, TEXT("%f"), DamageApplied);

	

	if (HasAuthority())
	{
		if (HealthComponent)
			HealthComponent->SetCurrentHealth(DamageApplied);

		ATPTeamFightGameMode* GM = GetWorld() != NULL ? GetWorld()->GetAuthGameMode<ATPTeamFightGameMode>() : NULL;
		if (GM)
		{
			// increase player swing attempt, total hit, and total damage done
			GM->PlayerHit(EventInstigator, this, ActualDamage);
			if (ATPPlayerState* PS = EventInstigator->GetPlayerState<ATPPlayerState>())
				PS->SetTotalSwingAttempt(PS->GetTotalSwingAttempt() + 1);
		}
	}
	return DamageApplied;
}

void ATrialProjectCharacter::PlayerDie_Implementation()
{
	GetMesh()->PlayAnimation(Anim_Death, false);
	GetMovementComponent()->Deactivate();
	// TODO respawn
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, this, &ATrialProjectCharacter::RequestRespawn, 5.f, false);
}

void ATrialProjectCharacter::RequestRespawn()
{
	// TODO respawn
}

void ATrialProjectCharacter::NMC_PlayAnimMontage_Implementation(UAnimMontage* InAnimMontage)
{
	if (UAnimInstance* TAnimInstance = GetMesh()->GetAnimInstance())
	{
		float AnimDuration = TAnimInstance->Montage_Play(InAnimMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.0f, true);
		GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
	}
}

void ATrialProjectCharacter::UpdateCurrentActiveMontage_Implementation()
{
	/* tell the new join player about some informations
	
	if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
	{
		if (UAnimMontage* CurrentMontage = AI->GetCurrentActiveMontage())
		{
			CurrentActiveMontage = CurrentMontage;
			CurrentActiveMontage_Position = AI->Montage_GetPosition(CurrentActiveMontage);
		}
	}
	
	*/
	
}

void ATrialProjectCharacter::OnRep_CharacterAnimation()
{
	if (UAnimInstance* AI = GetMesh()->GetAnimInstance())
	{
		float AnimDuration = AI->Montage_Play(CharacterAnimationStruct.Montage, 1.0f, EMontagePlayReturnType::MontageLength, 0.3f, true);
		GetWorld()->GetTimerManager().SetTimer(AttackHandle, this, &ATrialProjectCharacter::StopAttacking, AnimDuration, false);
	}
}
// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrialProjectGameMode.h"
#include "TrialProjectCharacter.h"
#include "UObject/ConstructorHelpers.h"

ATrialProjectGameMode::ATrialProjectGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

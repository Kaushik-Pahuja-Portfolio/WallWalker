// Copyright Epic Games, Inc. All Rights Reserved.

#include "WallWalkerGameMode.h"
#include "WallWalkerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWallWalkerGameMode::AWallWalkerGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}

// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"

#include "PlayerCharacter.h"
#include "CustomPlayerController.h"
#include "CustomPlayerAIController.h"

ACustomGameMode::ACustomGameMode()
{
    // Set the default pawn class to your custom player character
    DefaultPawnClass = APlayerCharacter::StaticClass();

    // Set the player controller class to your custom player controller
    PlayerControllerClass = ACustomPlayerController::StaticClass();
}
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "BehaviorTree/BehaviorTree.h"

#include "DrawDebugHelpers.h"

#include "CustomPlayerAIController.generated.h"

/**
 * 
 */
UCLASS()
class DUNGEONCRAWLER_API ACustomPlayerAIController : public AAIController
{
	GENERATED_BODY()
	
private:
	APawn* ControlledPawn;

public:
	// Constructor
	ACustomPlayerAIController();

	UFUNCTION()
	// Function to move the controlled character to a specific location
	void MoveCharacterToLocation(const FVector Destination);

protected:
	virtual void OnPossess(APawn* InPawn) override;
	void BindToPlayerControllerDelegate();
};

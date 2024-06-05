// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "CustomPlayerController.generated.h"

// Event to notify AIController about the clicked location
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClickedLocation, FVector, Location);
/**
 * 
 */
UCLASS()
class DUNGEONCRAWLER_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
    public:
	ACustomPlayerController();

    // Event to notify AIController about the clicked location
    FOnClickedLocation OnClickedLocation;

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

    bool CanUpdatePath(float DeltaTime);

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* InputMappingContext;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* Move;

    //path update interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float PathUpdateCooldown = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float PathUpdateCooldownLimiter = 0.3f;

    bool bIsMouseButtonDown;
    TArray<FVector> CurrentPathPoints;
    int32 CurrentPathIndex = 0;
    UNavigationSystemV1* NavSys = nullptr;
    float DistanceThreshold = 100.0f;

    void OnMousePressed();
    void OnMouseReleased();
    void UpdateMovementDestination();
    void MoveToLocation(const FVector& Location);
};

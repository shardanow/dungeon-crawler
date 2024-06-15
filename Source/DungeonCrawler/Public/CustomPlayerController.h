// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include <EnemyCharacter.h>
#include <Kismet/GameplayStatics.h>
#include "CustomPlayerController.generated.h"

// Event to notify about the clicked location
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnClickedLocation, FVector, Location);

// Event to notify about the player attack
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerAttack, bool, isAttack);

/**
 * 
 */
UCLASS()
class DUNGEONCRAWLER_API ACustomPlayerController : public APlayerController
{
	GENERATED_BODY()
	
    public:
	ACustomPlayerController();

    // Event to notify about the clicked location
    FOnClickedLocation OnClickedLocation;

    // Event to notify about the player attack
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerAttack OnPlayerAttack;

    void HandleAnimationNotify(FName NotifyName);

protected:
    virtual void SetupInputComponent() override;
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaTime) override;

private:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputMappingContext* InputMappingContext;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    UInputAction* Move;

    //path update interval
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float PathUpdateCooldown = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    float PathUpdateCooldownLimiter = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input", meta = (AllowPrivateAccess = "true"))
    bool bCanMove = true;

    bool bIsMouseButtonDown;
    TArray<FVector> CurrentPathPoints;
    int32 CurrentPathIndex = 0;
    UNavigationSystemV1* NavSys = nullptr;
    float DistanceThreshold = 100.0f;
    float AttackDistance = 150.0f;
    float AttackCooldown = 1.5f;
    float LastAttackTime = 0.0f;
    float Damage = 30.0f;

    // Player state
    //blueprint read write
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State", meta = (AllowPrivateAccess = "true"))
    bool bIsAttacking = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State", meta = (AllowPrivateAccess = "true"))
    bool bIsMoving = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player State", meta = (AllowPrivateAccess = "true"))
    bool canMove = true;

    AEnemyCharacter* EnemyUnderCursor = nullptr;

    AEnemyCharacter* LockedEnemy = nullptr;

    void OnMousePressed();
    void OnMouseHeld();
    void OnMouseReleased();
    void UpdateMovementDestination();
    void MoveToLocation(const FVector& Location);
    void RotateToEnemy(AEnemyCharacter* Enemy);
    void ApplyDamageToEnemyActor(AEnemyCharacter* EnemyTarget, float DamageAmount);

    void HandleMouseInteraction();  // Handles the initial and continuous updates
    void UpdateLockedEnemyOrDestination();
    void MoveAlongPath(float DeltaTime);
    bool CanUpdatePath(float DeltaTime);

    void AttackTargetEnemy();

    FVector CurrentHitLocation; // New variable to store the current hit location
};

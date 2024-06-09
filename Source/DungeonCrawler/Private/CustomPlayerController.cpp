// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerController.h"

ACustomPlayerController::ACustomPlayerController()
{
    // Initialize InputMappingContext in the constructor or via the Unreal Editor
}

void ACustomPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Make the mouse cursor always visible
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;

    // Add Input Mapping Context to Enhanced Input System
    UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (InputSubsystem && InputMappingContext)
    {
        InputSubsystem->AddMappingContext(InputMappingContext, 1); // 1 is the priority level
    }
}

void ACustomPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent);
    if (EnhancedInputComp && InputMappingContext)
	{
        EnhancedInputComp->BindAction(Move, ETriggerEvent::Started, this, &ACustomPlayerController::OnMousePressed);
        EnhancedInputComp->BindAction(Move, ETriggerEvent::Completed, this, &ACustomPlayerController::OnMouseReleased);
    }
}

void ACustomPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);


    // Continuously check if cursor is still on an enemy
    FHitResult Hit;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());
        if (Enemy && bIsMouseButtonDown)
        {
            float Distance = (Enemy->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
            if (Distance <= AttackDistance)
            {
                DamageEnemyUnderCursor();
                // Stop movement here if necessary
                if (APawn* ControlledPawn = GetPawn())
                {
                   //stop movement
					UAIBlueprintHelperLibrary::SimpleMoveToLocation(ControlledPawn->GetController(), ControlledPawn->GetActorLocation());
                    RotateToEnemy(Enemy);
                }
                return;  // Skip movement updates
            }
        }
    }


    if (bIsMouseButtonDown && CanUpdatePath(DeltaTime))
    {
        UpdateMovementDestination();
    }

    if (CurrentPathPoints.Num() > 0 && CurrentPathIndex < CurrentPathPoints.Num())
    {
        APawn* ControlledPawn = GetPawn();
        if (ControlledPawn && ControlledPawn->GetController())
        {
            FVector NextPoint = CurrentPathPoints[CurrentPathIndex];
            FVector PawnLocation = ControlledPawn->GetActorLocation();
            float Distance = (NextPoint - PawnLocation).Size();

            // Consider a small threshold for reaching the point to avoid precision issues
            //float DistanceThreshold = 100.0f; // Adjust this threshold as needed for your game


            //check if point reachability
   //         FHitResult Hit;
   //         bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, PawnLocation, NextPoint, ECC_Visibility);
   //         if (bHit)
			//{
			//	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Next point is not reachable"));
			//	return;
			//}

            if (Distance >= DistanceThreshold) // Check if pawn is close enough to consider it at the point
            {
                DrawDebugSphere(GetWorld(), NextPoint, 25.0f, 12, FColor::Green, false, 3.0f);

                // Calculate direction vector and convert to rotation
                FVector Direction = (NextPoint - PawnLocation).GetSafeNormal();  // Normalize to get direction
                FRotator NewRotation = Direction.ToOrientationRotator();

                // Retrieve the current rotation and replace only the yaw component
                FRotator CurrentRotation = ControlledPawn->GetActorRotation();
                FRotator TargetRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);

                // Optionally, smooth the rotation over time
                FRotator SmoothRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 5.0f); // Adjust 5.0f to control rotation speed

                ControlledPawn->SetActorRotation(SmoothRotation);

                UAIBlueprintHelperLibrary::SimpleMoveToLocation(ControlledPawn->GetController(), NextPoint);
            }
            else
            {
                CurrentPathIndex++; // Move to next point
            }
        }
    }
}

bool ACustomPlayerController::CanUpdatePath(float DeltaTime)
{
    PathUpdateCooldown -= DeltaTime;
    if (PathUpdateCooldown <= 0)
    {
        PathUpdateCooldown = PathUpdateCooldownLimiter;  // Cooldown in seconds, adjust as necessary
        return true;
    }
    return false;
}

void ACustomPlayerController::OnMousePressed()
{
    //debud on screen
  //  GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Mouse Pressed"));
    bIsMouseButtonDown = true;

    FHitResult Hit;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit)
    {
        AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(Hit.GetActor());
        if (Enemy)
        {
            float Distance = (Enemy->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
            if (Distance <= AttackDistance)
            {
                DamageEnemyUnderCursor();  // Call damage function directly if in range
                return;  // Do not initiate movement if attacking
            }
        }

        UpdateMovementDestination();
    }

}

void ACustomPlayerController::OnMouseReleased()
{
    //debud on screen
  //  GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Mouse Released"));

    bIsMouseButtonDown = false;
}

void ACustomPlayerController::UpdateMovementDestination()
{
   // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("UpdateMovementDestination called"));

    // Get the mouse cursor position
    FHitResult Hit;
    bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (bHit)
    {
        FVector HitLocation = Hit.Location;

        DrawDebugSphere(GetWorld(), HitLocation, 25.0f, 12, FColor::Red, false, 3.0f);

        if (Hit.bBlockingHit)
        {
            // We hit something, move player here
            APawn* MyPawn = GetPawn();
            if (MyPawn)
            {
                MoveToLocation(HitLocation);
            }
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No hit result under cursor"));
    }
}

void ACustomPlayerController::MoveToLocation(const FVector& Location)
{
    APawn* ControlledPawn = GetPawn();
    if (ControlledPawn)
    {
        NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FPathFindingQuery Query;
            FNavPathSharedPtr NavPath;
            const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::Create);

            if (NavData)
            {
                Query = FPathFindingQuery(ControlledPawn, *NavData, ControlledPawn->GetActorLocation(), Location);
                
                FPathFindingResult Result = NavSys->FindPathSync(Query);

                if (Result.IsSuccessful() && Result.Path.IsValid())
                {
                    CurrentPathPoints.Empty(); // Clear any existing points
                    for (const FNavPathPoint& PathPoint : Result.Path->GetPathPoints())
                    {
                        //debug path points
                       // DrawDebugSphere(GetWorld(), PathPoint.Location, 25.0f, 12, FColor::Yellow, false, 3.0f);

                        CurrentPathPoints.Add(PathPoint.Location);
                    }
                    CurrentPathIndex = 0; // Reset path index
                }
            }
        }
    }
}

// add damage to enemy under cursor
void ACustomPlayerController::DamageEnemyUnderCursor()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime < LastAttackTime + AttackCooldown)
    {
        //debug on screen
       // GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Attack on cooldown"));

        return; // Skip this call since we're still in cooldown
    }

	FHitResult Hit;
	bool bHit = GetHitResultUnderCursor(ECC_Visibility, false, Hit);

	if (bHit)
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor)
		{
			AEnemyCharacter* Enemy = Cast<AEnemyCharacter>(HitActor);
			if (Enemy)
			{
				// Apply damage to the enemy  only if enemy actor is near player
                float Distance = (Enemy->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
                if (Distance <= AttackDistance) // Adjust this distance as needed
                {
                    // Clear any existing movement path points to stop movement
                    CurrentPathPoints.Empty(); 

                    // Apply damage to the enemy
                    UGameplayStatics::ApplyDamage(Enemy, Damage, GetInstigatorController(), this, UDamageType::StaticClass());

                    // Apply damage to the enemy only if the enemy actor is near the player
                    LastAttackTime = CurrentTime; // Update last attack time
                }
			}
		}
	}
}

//rotate player to enemy smoothly only on z axis
void ACustomPlayerController::RotateToEnemy(AEnemyCharacter* Enemy)
{
	FRotator NewRotation = (Enemy->GetActorLocation() - GetPawn()->GetActorLocation()).ToOrientationRotator();
	FRotator CurrentRotation = GetPawn()->GetActorRotation();
	FRotator TargetRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);
	FRotator SmoothRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, GetWorld()->GetDeltaSeconds(), 8.0f); // Adjust 8.0f to control rotation speed
	GetPawn()->SetActorRotation(SmoothRotation);
}
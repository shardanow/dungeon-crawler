// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerAIController.h"
#include "CustomPlayerController.h"

ACustomPlayerAIController::ACustomPlayerAIController()
{
    // Ensure the controller uses navigation
    bSetControlRotationFromPawnOrientation = false;

    // Ensure Path Following Component is created
    if (!GetPathFollowingComponent())
    {
        UE_LOG(LogTemp, Warning, TEXT("Path Following Component not found!"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Path Following Component initialized correctly."));
    }
}

void ACustomPlayerAIController::MoveCharacterToLocation(const FVector Destination)
{
    //ControlledPawn = GetPawn();

    if (ControlledPawn)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Pawn Name: %s"), *ControlledPawn->GetName()));

        // Check if the destination is within the NavMesh bounds
        UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
        if (NavSys)
        {
            FNavLocation NavLocation;
            bool bOnNavMesh = NavSys->ProjectPointToNavigation(Destination, NavLocation, FVector(0.0f, 0.0f, 0.0f), nullptr, nullptr);

            if (!bOnNavMesh)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Destination is not on the NavMesh"));
                return;
            }

            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Projected Destination: %s"), *NavLocation.Location.ToString()));

            FAIMoveRequest MoveRequest;
            MoveRequest.SetGoalLocation(NavLocation.Location);
            MoveRequest.SetAcceptanceRadius(50.0f);
            MoveRequest.SetUsePathfinding(true);
            MoveRequest.SetAllowPartialPath(true);



            FNavPathSharedPtr NavPath;

            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Move Request: %s"), *MoveRequest.GetGoalLocation().ToString()));

            EPathFollowingRequestResult::Type MoveResult = MoveTo(MoveRequest, &NavPath);

            if (MoveResult == EPathFollowingRequestResult::Failed)
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Move request failed"));
            }
            if (NavPath)
            {
                const TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
                for (const FNavPathPoint& PathPoint : PathPoints)
                {
                    DrawDebugSphere(GetWorld(), PathPoint.Location, 25.0f, 12, FColor::Yellow, true, 5.0f);
                }
            }
            else
            {
                GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("NavPath is null"));
            }
        }
        else
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Navigation system is not found"));
        }
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ControlledPawn is null"));
    }
}

void ACustomPlayerAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        ControlledPawn = InPawn;

        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Possessing: %s"), *InPawn->GetName()));
        BindToPlayerControllerDelegate();
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Possess called with null pawn"));
    }
}

void ACustomPlayerAIController::BindToPlayerControllerDelegate()
{
        APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
        //debug on screen
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Binding to Player Controller"));

        if (ACustomPlayerController* CustomPlayerController = Cast<ACustomPlayerController>(PlayerController))
        {
            //debug on screen
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Binding to Custom Player Controller"));

            CustomPlayerController->OnClickedLocation.AddDynamic(this, &ACustomPlayerAIController::MoveCharacterToLocation);
        }
}
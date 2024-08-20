// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomPlayerController.h"
#include "PlayerCharacter.h"

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
	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (InputMappingContext)
		{
			InputSubsystem->AddMappingContext(InputMappingContext, 1); // 1 is the priority level
		}
	}
}

void ACustomPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (InputMappingContext)
		{
			EnhancedInputComp->BindAction(Move, ETriggerEvent::Started, this, &ACustomPlayerController::OnMousePressed);
			EnhancedInputComp->BindAction(Move, ETriggerEvent::Ongoing, this, &ACustomPlayerController::OnMouseHeld);
			EnhancedInputComp->BindAction(Move, ETriggerEvent::Completed, this, &ACustomPlayerController::OnMouseReleased);
		}
	}
}

void ACustomPlayerController::HandleAnimationNotify(FName NotifyName)
{
	if (NotifyName == "AttackHitPlayerMale")
	{
		// Debug on screen
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AttackHitPlayerMale Anim Event Triggered"));
		// Apply damage to enemy under cursor
		ApplyDamageToEnemyActor(LockedEnemy, Damage);
	}
}

void ACustomPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bIsMouseButtonDown)
	{
		HandleMouseInteraction(); // Continuously handle mouse interaction while button is down
	}

	if (LockedEnemy)
	{
		//debug on screen sphere where the enemy is
		//DrawDebugSphere(GetWorld(), LockedEnemy->GetActorLocation(), 25.0f, 12, FColor::Blue, false, 0.5f);

					//TODO - FIX no need movement when enemy is in attack distance


		float Distance = (LockedEnemy->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
		if (Distance <= AttackDistance)
		{
			//debug on screen attack distance sphere
			//DrawDebugSphere(GetWorld(), LockedEnemy->GetActorLocation(), AttackDistance, 12, FColor::Red, false, 0.5f);

			//debug console message
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Enemy in attack distance"));

			//clear path points
			CurrentPathPoints.Empty();

			//stop movement
			UAIBlueprintHelperLibrary::SimpleMoveToLocation(GetPawn()->GetController(), GetPawn()->GetActorLocation());

			RotateToEnemy(LockedEnemy);
			AttackTargetEnemy();
			return;
		}
		else
		{
			//debug on screen move to location
		   // DrawDebugSphere(GetWorld(), LockedEnemy->GetActorLocation(), 25.0f, 12, FColor::Yellow, false, 0.5f);

			//debug console message
			//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Enemy out of attack distance"));

			RotateToEnemy(LockedEnemy);
			MoveToLocation(LockedEnemy->GetActorLocation());

			//move along the path
			MoveAlongPath(DeltaTime);
			return;
		}
	}

	// If there is a valid hit location, move towards it
	if (!CurrentHitLocation.IsZero())
	{
		if (CanUpdatePath(DeltaTime))
		{
			MoveToLocation(CurrentHitLocation);
		}
	}

	//debug on screen path points
 //   for (int32 i = 0; i < CurrentPathPoints.Num(); i++)
	//{
	//	DrawDebugSphere(GetWorld(), CurrentPathPoints[i], 25.0f, 12, FColor::Black, false, 0.5f);
	//}

	//move along the path
	MoveAlongPath(DeltaTime);
}

void ACustomPlayerController::MoveAlongPath(float DeltaTime)
{
	if (CurrentPathPoints.Num() > 0 && CurrentPathIndex < CurrentPathPoints.Num())
	{
		APawn* ControlledPawn = GetPawn();
		if (ControlledPawn && ControlledPawn->GetController())
		{
			FVector NextPoint = CurrentPathPoints[CurrentPathIndex];
			FVector PawnLocation = ControlledPawn->GetActorLocation();
			float Distance = (NextPoint - PawnLocation).Size();

			if (Distance >= DistanceThreshold)
			{
				//DrawDebugSphere(GetWorld(), NextPoint, 25.0f, 12, FColor::Green, false, 0.5f);

				FVector Direction = (NextPoint - PawnLocation).GetSafeNormal();
				FRotator TargetRotation = FRotator(0.0f, Direction.ToOrientationRotator().Yaw, 0.0f);
				FRotator SmoothRotation = FMath::RInterpTo(ControlledPawn->GetActorRotation(), TargetRotation, DeltaTime, 5.0f);

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
	bIsMouseButtonDown = true;
	HandleMouseInteraction();  // Handle initial click

	// Immediately update path on click
	if (!CurrentHitLocation.IsZero())
	{
		MoveToLocation(CurrentHitLocation);
	}

	LockedEnemy = nullptr;  // Clear the lock on mouse release
	CurrentHitLocation = FVector::ZeroVector; // Clear the current hit location
}

void ACustomPlayerController::OnMouseHeld()
{
	if (bIsMouseButtonDown)
	{
		HandleMouseInteraction();  // Handle continuous updates while holding
	}
}

void ACustomPlayerController::OnMouseReleased()
{
	bIsMouseButtonDown = false;

}

void ACustomPlayerController::HandleMouseInteraction()
{
	UpdateLockedEnemyOrDestination();
}

void ACustomPlayerController::UpdateLockedEnemyOrDestination()
{
	OnPlayerAttack.Broadcast(false);

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit)
	{
		AEnemyCharacter* HitEnemy = Cast<AEnemyCharacter>(Hit.GetActor());
		if (HitEnemy)
		{
			float Distance = (HitEnemy->GetActorLocation() - GetPawn()->GetActorLocation()).Size();
			if (Distance <= AttackDistance)
			{
				CurrentPathPoints.Empty();
				LockedEnemy = HitEnemy;
				AttackTargetEnemy();
			}
			else
			{
				LockedEnemy = HitEnemy;
				MoveToLocation(HitEnemy->GetActorLocation());
			}
		}
		else
		{
			LockedEnemy = nullptr;
			CurrentHitLocation = Hit.Location; // Store the current hit location
		}
	}
}

void ACustomPlayerController::UpdateMovementDestination()
{
	EnemyUnderCursor = nullptr;
	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit) && Hit.bBlockingHit)
	{
		FVector HitLocation = Hit.Location;
		//DrawDebugSphere(GetWorld(), HitLocation, 25.0f, 12, FColor::Red, false, 0.5f);
		CurrentHitLocation = HitLocation; // Update the current hit location
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No hit result under cursor"));
	}
}

void ACustomPlayerController::MoveToLocation(const FVector& Location)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
		if (NavSys)
		{
			const ANavigationData* NavData = NavSys->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::Create);
			if (NavData)
			{
				FPathFindingQuery Query(ControlledPawn, *NavData, ControlledPawn->GetActorLocation(), Location);
				FPathFindingResult Result = NavSys->FindPathSync(Query);

				if (Result.IsSuccessful() && Result.Path.IsValid())
				{
					CurrentPathPoints.Empty();
					for (const FNavPathPoint& PathPoint : Result.Path->GetPathPoints())
					{

						// Skip the first point if it's the current location of the player and ignore points that same coordinates
						if (!PathPoint.Location.Equals(ControlledPawn->GetActorLocation(), DistanceThreshold)
							&&
							(FMath::Abs(PathPoint.Location.X - ControlledPawn->GetActorLocation().X) > KINDA_SMALL_NUMBER ||
								FMath::Abs(PathPoint.Location.Y - ControlledPawn->GetActorLocation().Y) > KINDA_SMALL_NUMBER)
							)
						{
							CurrentPathPoints.Add(PathPoint.Location);

							//debug sphere on screen for path points
						   // DrawDebugSphere(GetWorld(), PathPoint.Location, 25.0f, 12, FColor::Blue, false, 0.5f);
						}
						else {
							continue;
						}
					}
					CurrentPathIndex = 0;
				}
			}
		}
	}
}

void ACustomPlayerController::RotateToEnemy(AEnemyCharacter* Enemy)
{
	FRotator NewRotation = (Enemy->GetActorLocation() - GetPawn()->GetActorLocation()).ToOrientationRotator();
	FRotator TargetRotation = FRotator(0.0f, NewRotation.Yaw, 0.0f);
	FRotator SmoothRotation = FMath::RInterpTo(GetPawn()->GetActorRotation(), TargetRotation, GetWorld()->GetDeltaSeconds(), 8.0f);
	GetPawn()->SetActorRotation(SmoothRotation);
}

void ACustomPlayerController::ApplyDamageToEnemyActor(AEnemyCharacter* EnemyTarget, float DamageAmount)
{
	if (EnemyTarget)
	{
		UGameplayStatics::ApplyDamage(EnemyTarget, DamageAmount, GetInstigatorController(), this, UDamageType::StaticClass());
	}
}

void ACustomPlayerController::AttackTargetEnemy()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime < LastAttackTime + AttackCooldown)
	{
		//debug on screen attack cooldown message
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Attack Cooldown"));

		OnPlayerAttack.Broadcast(false);
		return; // Skip this call since we're still in cooldown
	}

	//debug on screen try to hit enemy message
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Trying to hit enemy"));


	// Check if the enemy is within attack distance
	//FHitResult Hit;
	//if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	//{
	//    if (AActor* HitActor = Hit.GetActor())
	//    {
		   // EnemyUnderCursor = Cast<AEnemyCharacter>(HitActor);
	if (LockedEnemy && !LockedEnemy->IsDeadState())
	{
		float SquaredDistance = (LockedEnemy->GetActorLocation() - GetPawn()->GetActorLocation()).SizeSquared();
		if (SquaredDistance <= FMath::Square(AttackDistance))
		{
			RotateToEnemy(LockedEnemy);

			if (APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn()))
			{
				//TODO - change IsDamaged to IsCriticalDamaged
				if (!PlayerCharacter->IsDamaged())
				{
					CurrentPathPoints.Empty();
					OnPlayerAttack.Broadcast(true);
					//TODO - Calculate treshold for critical damage
					LastAttackTime = CurrentTime;
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Enemy Hit Triggered"));
				}
			}
		}
		/* else
		 {
			 EnemyUnderCursor = nullptr;
		 }*/
	}
	else
	{
		LockedEnemy = nullptr;
	}
	//}
   /* else
	{
		EnemyUnderCursor = nullptr;
	}*/
	//}
	OnPlayerAttack.Broadcast(false);
}
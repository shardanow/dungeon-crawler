// Fill out your copyright notice in the Description page of Project Settings.


#include "RoomGenerator.h"

// Sets default values
ARoomGenerator::ARoomGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize the Root Component
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(Root);


}

// Called when the game starts or when spawned
void ARoomGenerator::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ARoomGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ARoomGenerator::RandomRoomSizeGenerate()
{
	// Generate room dimensions based on wall lengths and widths
	NumWallsLength = FMath::CeilToInt(FMath::RandRange(MinRoomLength / FloorLength, MaxRoomLength / FloorLength));
	NumWallsWidth = FMath::CeilToInt(FMath::RandRange(MinRoomWidth / FloorWidth, MaxRoomWidth / FloorWidth));

	RoomLength = NumWallsLength * FloorLength;  // Exact multiple of wall length
	RoomWidth = NumWallsWidth * FloorWidth;  // Exact multiple of wall width

	// debug on screen
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Length: %f"), RoomLength));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Width: %f"), RoomWidth));


	//debug in console
	UE_LOG(LogTemp, Warning, TEXT("Room Length: %f"), RoomLength);
	UE_LOG(LogTemp, Warning, TEXT("Room Width: %f"), RoomWidth);
}

void ARoomGenerator::FixedRoomSizeGenerate(float NewRoomLength, float NewRoomWidth) {
	// Generate room dimensions based on wall lengths and widths
	NumWallsLength = FMath::CeilToInt(NewRoomLength / FloorLength);
	NumWallsWidth = FMath::CeilToInt(NewRoomWidth / FloorWidth);

	RoomLength = NumWallsLength * FloorLength;  // Exact multiple of wall length
	RoomWidth = NumWallsWidth * FloorLength;  // Exact multiple of wall width

	// debug on screen
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Length: %f"), RoomLength));
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Width: %f"), RoomWidth));

	//debug in console
	//UE_LOG(LogTemp, Warning, TEXT("Room Length: %f"), RoomLength);
	//UE_LOG(LogTemp, Warning, TEXT("Room Width: %f"), RoomWidth);
}

void ARoomGenerator::GenerateRoom(FVector RoomPosition)
{
	if (WallMeshes.Num() == 0 || FloorMeshes.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Mesh arrays are empty."));
		return;
	}

	//FVector Origin = GetActorLocation();

	FVector Origin = RoomPosition;

	//set current room generator actor position
	SetActorLocation(Origin);

	// Draw the room outline for visual reference
	//DrawDebugBox(GetWorld(), Origin + FVector(RoomLength / 2, RoomWidth / 2, 0), FVector(RoomLength / 2, RoomWidth / 2, 10), FColor::Emerald, true, -1, 0, 10);

	// Walls along the length of the room
	for (int i = 0; i <= RoomLength / WallLength; i++) {
		//flag to check that something is spawned on current floor socket
		bool bSomethingSpawnedWall = false;
		bool bSomethingSpawnedOppositeWall = false;

		//define tag for the wall mesh component as WallCorner if it is the first or last wall mesh component
		FName WallTag = TEXT("Wall");
		if (i == 0 || i >= (int)(RoomLength / WallLength))
		{
			WallTag = TEXT("WallCorner");

			//draw debug sphere
			//DrawDebugSphere(GetWorld(), Origin + FVector(i * WallLength, 0, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
			//DrawDebugSphere(GetWorld(), Origin + FVector(i * WallLength + WallLength, RoomWidth, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
		}

		UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
		FVector WallPosition = Origin + FVector(i * WallLength, 0, 0);
		UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, 0, 0), TEXT("wall_left_bottom_corner"), WallTag);

		//DrawDebugSphere(GetWorld(), WallPosition, 20, 12, FColor::Red, true, -1, 0, 2);

		// Adjust opposite wall position to start exactly from the corner
		UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
		FVector OppositeWallPosition = Origin + FVector(i * WallLength + WallLength, RoomWidth, 0); // Subtracting one WallLength
		UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 180, 0), TEXT("wall_right_bottom_corner"), WallTag);

		//DrawDebugSphere(GetWorld(), OppositeWallPosition, 20, 12, FColor::Blue, true, -1, 0, 2);

		// spawn torches on wall sockets
		if (!bSomethingSpawnedWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
			bSomethingSpawnedWall = true;
			SpawnedWallLightSourcesPositions.Add(WallPosition);
		}

		if (!bSomethingSpawnedOppositeWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
			bSomethingSpawnedOppositeWall = true;
			SpawnedWallLightSourcesPositions.Add(OppositeWallPosition);
		}

		//spawn wall decoration on wall socket
		if (!bSomethingSpawnedWall && WallDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], WallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center", "Decoration")) {
			DecoratedWallPositions.Add(WallPosition);
			bSomethingSpawnedWall = true;
		}

		if (!bSomethingSpawnedOppositeWall && WallDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], OppositeWallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center", "Decoration")) {
			DecoratedWallPositions.Add(OppositeWallPosition);
			bSomethingSpawnedOppositeWall = true;
		}
	}


	// Walls along the width of the room
	for (int j = 0; j <= RoomWidth / WallLength; j++) {
		//flag to check that something is spawned on current wall socket name
		bool bSomethingSpawnedWall = false;
		bool bSomethingSpawnedOppositeWall = false;

		//define tag for the wall mesh component as WallCorner if it is the first or last wall mesh component
		FName WallTag = TEXT("Wall");
		if (j == 0 || j >= (int)(RoomWidth / WallLength))
		{
			WallTag = TEXT("WallCorner");

			//draw debug sphere
			//DrawDebugSphere(GetWorld(), Origin + FVector(0, j * WallLength + WallLength, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
			//DrawDebugSphere(GetWorld(), Origin + FVector(RoomLength, j * WallLength, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
		}

		UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
		FVector WallPosition = Origin + FVector(0, j * WallLength + WallLength, 0); // Subtracting one WallLength
		UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, -90, 0), TEXT("wall_left_bottom_corner"), WallTag);

		//DrawDebugSphere(GetWorld(), WallPosition, 30, 12, FColor::Blue, true, -1, 0, 2);


		UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
		FVector OppositeWallPosition = Origin + FVector(RoomLength, j * WallLength, 0);
		UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 90, 0), TEXT("wall_right_bottom_corner"), WallTag);

		//DrawDebugSphere(GetWorld(), OppositeWallPosition, 30, 12, FColor::Red, true, -1, 0, 2);

		//debug number of wall meshes and current index
		//UE_LOG(LogTemp, Warning, TEXT("Create Wall Mesh id: %d of: %d"), j, (int)(RoomWidth / WallLength));


		// spawn torches on wall sockets
		if (!bSomethingSpawnedWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
			bSomethingSpawnedWall = true;
			SpawnedWallLightSourcesPositions.Add(WallPosition);
		}

		if (!bSomethingSpawnedOppositeWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
			bSomethingSpawnedOppositeWall = true;
			SpawnedWallLightSourcesPositions.Add(OppositeWallPosition);
		}

		//spawn wall decoration on wall socket
		if (!bSomethingSpawnedWall && WallDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], WallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center", "Decoration")) {
			DecoratedWallPositions.Add(WallPosition);
			bSomethingSpawnedWall = true;
		}

		if (!bSomethingSpawnedOppositeWall && WallDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], OppositeWallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center", "Decoration")) {
			DecoratedWallPositions.Add(OppositeWallPosition);
			bSomethingSpawnedOppositeWall = true;
		}
	}


	// Place floor sections
	for (int m = 0; m < RoomLength / FloorLength; m++) {
		for (int n = 0; n < RoomWidth / FloorWidth; n++) {
			FVector FloorPosition = Origin + FVector(m * FloorLength, n * FloorWidth, 0);
			//flag to check that something is spawned on current floor socket
			bool bSomethingSpawned = false;

			// Calculate remaining space in the room for the current position
			float RemainingLength = RoomLength - (m * FloorLength);
			float RemainingWidth = RoomWidth - (n * FloorWidth);

			// Check if the remaining space is at least half the size of a floor mesh
			if (RemainingLength >= FloorLength / 2 && RemainingWidth >= FloorWidth / 2) {
				UStaticMesh* SelectedFloorMesh = FloorMeshes[FMath::RandRange(0, FloorMeshes.Num() - 1)];
				UStaticMeshComponent* FloorMeshComponent = SpawnMesh(SelectedFloorMesh, FloorPosition, FRotator(0, 0, 0), "floor_left_bottom_corner", "Floor");

				UStaticMesh* SelectedPillarMesh = FloorPillarsMeshes[FMath::RandRange(0, FloorPillarsMeshes.Num() - 1)];

				int Margin = 0;
				// Check that the current position is not near the wall boundaries
				if (m > Margin && m < NumWallsLength - Margin - 1 && n > Margin && n < NumWallsWidth - Margin - 1) {
					// Calculate the interval for placing pillars to distribute them evenly
					int PillarIntervalLength = FMath::Max(1, (NumWallsLength - 2 * Margin) / 2);
					int PillarIntervalWidth = FMath::Max(1, (NumWallsWidth - 2 * Margin) / 2);
					// Check if we are at a calculated interval for a pillar
					if ((m - Margin) % PillarIntervalLength == PillarIntervalLength / 2 &&
						(n - Margin) % PillarIntervalWidth == PillarIntervalWidth / 2) {

						//pillar mesh component
						UStaticMeshComponent* PillarMeshComponent = SpawnMeshOnExistingMeshFloorSocket(SelectedPillarMesh, FloorMeshComponent, 100, "spawn_center_point", "IgnoreCamera", "Pillar");

						//get list of socket names
						TArray<FName> PillarDecorationSocketNames = { "pillar_center_left", "pillar_center_right", "pillar_center_front", "pillar_center_back" };

						// spawn random count of torches on pillar sockets
						for (int k = 0; k < PillarDecorationSocketNames.Num(); k++) {
							if (WallTorchesBlueprints.Num() > 0
								&& SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)],
									PillarMeshComponent,
									50,
									PillarDecorationSocketNames[k],
									"IgnoreCamera", "Pillar")
								) {
								//bSomethingSpawned = true;
								SpawnedWallLightSourcesPositions.Add(FloorPosition);
							}
						}

						//spawn pillars on floor socket
						if (!bSomethingSpawned && PillarMeshComponent) {
							//draw debug sphere
						   // DrawDebugSphere(GetWorld(), FloorPosition, 20, 12, FColor::Red, true, -1, 0, 2);

							bSomethingSpawned = true;
						}
					}
				}

				//get random socket name from the array
				TArray<FName> FloorDecorationSocketNames = { "spawn_center_point", "spawn_corner_left_top_point", "spawn_corner_right_top_point", "spawn_corner_left_bottom_point", "spawn_corner_right_bottom_point" };
				FName RandomDecorationSocketName = FloorDecorationSocketNames[FMath::RandRange(0, FloorDecorationSocketNames.Num() - 1)];

				//spawn light sources on floor socket only if no light source is spawned nearby
				FVector PotentialPosition = FloorPosition + FVector(FloorLength / 2, FloorWidth / 2, 0);

				if (!bSomethingSpawned && FloorLightSourcesBlueprints.Num() > 0
					&& !IsTooCloseToOtherLightSources(SpawnedFloorLightSourcesPositions, PotentialPosition, FloorWidth * 2)
					&& !IsTooCloseToOtherLightSources(SpawnedWallLightSourcesPositions, PotentialPosition, FloorWidth * 2)
					&& SpawnActorOnSocket(FloorLightSourcesBlueprints[FMath::RandRange(0, FloorLightSourcesBlueprints.Num() - 1)],
						FloorMeshComponent,
						FloorLightSourcesSpawnChance,
						RandomDecorationSocketName,
						"BlockAll",
						"Light")
					) {
					bSomethingSpawned = true;
					SpawnedFloorLightSourcesPositions.Add(PotentialPosition);
				}


				//spawn light sources on floor socket
	//            if (!bSomethingSpawned && FloorLightSourcesBlueprints.Num() > 0 && SpawnActorOnSocket(FloorLightSourcesBlueprints[FMath::RandRange(0, FloorLightSourcesBlueprints.Num() - 1)], FloorMeshComponent, FloorLightSourcesSpawnChance, RandomDecorationSocketName)) {
				//	bSomethingSpawned = true;
				//}

				//spawn floor decoration on floor socket
				if (!bSomethingSpawned && FloorDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshFloorSocket(FloorDecorationMeshes[FMath::RandRange(0, FloorDecorationMeshes.Num() - 1)], FloorMeshComponent, FloorDecorationSpawnChance, RandomDecorationSocketName, "BlockAll", "Decoration")) {
					//bSomethingSpawned = true;
				}

				// Spawn treasure on floor socket
				if (!bSomethingSpawned && TreasureBlueprints.Num() > 0 && SpawnActorOnSocket(TreasureBlueprints[FMath::RandRange(0, TreasureBlueprints.Num() - 1)], FloorMeshComponent, TreasureSpawnChance, "spawn_center_point", "BlockAll", "Treasure")) {
					bSomethingSpawned = true;
				}

				// Spawn enemies on floor socket
				if (!bSomethingSpawned && EnemyBlueprints.Num() > 0 && SpawnActorOnSocket(EnemyBlueprints[FMath::RandRange(0, EnemyBlueprints.Num() - 1)], FloorMeshComponent, EnemySpawnChance, "spawn_center_point", "BlockAll", "Enemy")) {
					bSomethingSpawned = true;
				}

			}
		}
	}

	// Add a PlayerStart actor randomly in the room for the player to spawn
	FVector PlayerStartLocation = Origin + FVector(FMath::RandRange(0, (int)RoomLength - (int)FloorWidth), FMath::RandRange(0, (int)RoomWidth - (int)FloorWidth), 0);
	//exclude z axis
	PlayerStartLocation.Z = WallHeight / 2.5f;
}


void ARoomGenerator::GenerateCorridor(FVector CorridorPosition, FName DisableWallSide) {
	if (WallMeshes.Num() == 0 || FloorMeshes.Num() == 0) {
		UE_LOG(LogTemp, Warning, TEXT("Mesh arrays are empty."));
		return;
	}

	//FVector Origin = GetActorLocation();

	FVector Origin = CorridorPosition;

	//calculate center based on room width and height
	FVector CenterPoint = Origin + FVector(RoomLength / 2, RoomWidth / 2, 0);


	//set current room generator actor position
	SetActorLocation(CenterPoint);

	if (DisableWallSide == "Length") {
		// Walls along the length of the room
		for (int i = 0; i <= RoomLength / WallLength; i++) {
			//flag to check that something is spawned on current floor socket
			bool bSomethingSpawnedWall = false;
			bool bSomethingSpawnedOppositeWall = false;

			//define tag for the wall mesh component as WallCorner if it is the first or last wall mesh component
			FName WallTag = TEXT("WallCorridor");
			if (i == 0 || i >= (int)(RoomLength / WallLength))
			{
				WallTag = TEXT("WallCorner");

				//draw debug sphere
				//DrawDebugSphere(GetWorld(), Origin + FVector(i * WallLength, 0, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
				//DrawDebugSphere(GetWorld(), Origin + FVector(i * WallLength + WallLength, RoomWidth, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
			}

			UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
			FVector WallPosition = Origin + FVector(i * WallLength, 0, 0);
			UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, 0, 0), TEXT("wall_left_bottom_corner"), WallTag);

			//DrawDebugSphere(GetWorld(), WallPosition, 20, 12, FColor::Red, true, -1, 0, 2);

			// Adjust opposite wall position to start exactly from the corner
			UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
			FVector OppositeWallPosition = Origin + FVector(i * WallLength + WallLength, RoomWidth, 0); // Subtracting one WallLength
			UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 180, 0), TEXT("wall_right_bottom_corner"), WallTag);

			//DrawDebugSphere(GetWorld(), OppositeWallPosition, 20, 12, FColor::Blue, true, -1, 0, 2);

			// spawn torches on wall sockets
			if (!bSomethingSpawnedWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
				bSomethingSpawnedWall = true;
				SpawnedWallLightSourcesPositions.Add(WallPosition);
			}

			if (!bSomethingSpawnedOppositeWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
				bSomethingSpawnedOppositeWall = true;
				SpawnedWallLightSourcesPositions.Add(OppositeWallPosition);
			}
		}
	}

	if (DisableWallSide == "Width") {


		// Walls along the width of the room
		for (int j = 0; j <= RoomWidth / WallLength; j++) {
			//flag to check that something is spawned on current wall socket name
			bool bSomethingSpawnedWall = false;
			bool bSomethingSpawnedOppositeWall = false;

			//define tag for the wall mesh component as WallCorner if it is the first or last wall mesh component
			FName WallTag = TEXT("WallCorridor");
			if (j == 0 || j >= (int)(RoomWidth / WallLength))
			{
				WallTag = TEXT("WallCorner");

				//draw debug sphere
				//DrawDebugSphere(GetWorld(), Origin + FVector(0, j * WallLength + WallLength, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
				//DrawDebugSphere(GetWorld(), Origin + FVector(RoomLength, j * WallLength, 0), 20, 12, FColor::Yellow, true, -1, 0, 2);
			}

			UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
			FVector WallPosition = Origin + FVector(0, j * WallLength + WallLength, 0); // Subtracting one WallLength
			UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, -90, 0), TEXT("wall_left_bottom_corner"), WallTag);

			//DrawDebugSphere(GetWorld(), WallPosition, 30, 12, FColor::Blue, true, -1, 0, 2);


			UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
			FVector OppositeWallPosition = Origin + FVector(RoomLength, j * WallLength, 0);
			UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 90, 0), TEXT("wall_right_bottom_corner"), WallTag);

			//DrawDebugSphere(GetWorld(), OppositeWallPosition, 30, 12, FColor::Red, true, -1, 0, 2);

			//debug number of wall meshes and current index
			//UE_LOG(LogTemp, Warning, TEXT("Create Wall Mesh id: %d of: %d"), j, (int)(RoomWidth / WallLength));


			// spawn torches on wall sockets
			if (!bSomethingSpawnedWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
				bSomethingSpawnedWall = true;
				SpawnedWallLightSourcesPositions.Add(WallPosition);
			}

			if (!bSomethingSpawnedOppositeWall && WallTorchesBlueprints.Num() > 0 && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center", "BlockAll", "Light")) {
				bSomethingSpawnedOppositeWall = true;
				SpawnedWallLightSourcesPositions.Add(OppositeWallPosition);
			}
		}

	}

	// Place floor sections
	for (int m = 0; m < RoomLength / FloorLength; m++) {
		for (int n = 0; n < RoomWidth / FloorWidth; n++) {
			FVector FloorPosition = Origin + FVector(m * FloorLength, n * FloorWidth, 0);
			//flag to check that something is spawned on current floor socket
			bool bSomethingSpawned = false;

			// Calculate remaining space in the room for the current position
			float RemainingLength = RoomLength - (m * FloorLength);
			float RemainingWidth = RoomWidth - (n * FloorWidth);

			// Check if the remaining space is at least half the size of a floor mesh
			if (RemainingLength >= FloorLength / 2 && RemainingWidth >= FloorWidth / 2) {
				UStaticMesh* SelectedFloorMesh = FloorMeshes[FMath::RandRange(0, FloorMeshes.Num() - 1)];
				UStaticMeshComponent* FloorMeshComponent = SpawnMesh(SelectedFloorMesh, FloorPosition, FRotator(0, 0, 0), "floor_left_bottom_corner", "Floor");

				//get random socket name from the array
				TArray<FName> FloorDecorationSocketNames = { "spawn_center_point", "spawn_corner_left_top_point", "spawn_corner_right_top_point", "spawn_corner_left_bottom_point", "spawn_corner_right_bottom_point" };
				FName RandomDecorationSocketName = FloorDecorationSocketNames[FMath::RandRange(0, FloorDecorationSocketNames.Num() - 1)];

				//spawn floor decoration on floor socket
				if (!bSomethingSpawned && FloorDecorationMeshes.Num() > 0 && SpawnMeshOnExistingMeshFloorSocket(FloorDecorationMeshes[FMath::RandRange(0, FloorDecorationMeshes.Num() - 1)], FloorMeshComponent, FloorDecorationSpawnChance, RandomDecorationSocketName, "BlockAll", "Decoration")) {
					//bSomethingSpawned = true;
				}

				// Spawn enemies on floor socket
				if (!bSomethingSpawned && EnemyBlueprints.Num() > 0 && SpawnActorOnSocket(EnemyBlueprints[FMath::RandRange(0, EnemyBlueprints.Num() - 1)], FloorMeshComponent, EnemySpawnChance, "spawn_center_point", "BlockAll", "Enemy")) {
					bSomethingSpawned = true;
				}

			}
		}
	}
}

UStaticMeshComponent* ARoomGenerator::SpawnMesh(UStaticMesh* Mesh, FVector Location, FRotator Rotation, FName SocketName, FName TagName)
{
	UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(this);
	//MeshComp->RegisterComponent();
	MeshComp->SetStaticMesh(Mesh);
	//set collision
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetCollisionProfileName("BlockAll");

	//set mesh to be movable
	MeshComp->SetMobility(EComponentMobility::Movable);

	MeshComp->SetWorldLocation(Location);
	MeshComp->SetWorldRotation(Rotation);
	MeshComp->RegisterComponent();

	//add tag to the mesh component
	MeshComp->ComponentTags.Add(TagName);

	//FAttachmentTransformRules::KeepWorldTransform
   // FAttachmentTransformRules::SnapToTargetIncludingScale
	if (SocketName != NAME_None) {
		MeshComp->AttachToComponent(Root, FAttachmentTransformRules::KeepWorldTransform, SocketName);
	}

	// Additional visual debugging to confirm orientation and placement
   // DrawDebugDirectionalArrow(GetWorld(), Location, Location + Rotation.RotateVector(FVector(100, 0, 0)), 120.0f, FColor::Cyan, true, -1, 0, 5);
   // DrawDebugDirectionalArrow(GetWorld(), Location, Location + Rotation.RotateVector(FVector(0, 100, 0)), 120.0f, FColor::Magenta, true, -1, 0, 5);

	return MeshComp;
}


bool ARoomGenerator::SpawnActorOnSocket(TSubclassOf<AActor> ActorClass, UStaticMeshComponent* Mesh, float SpawnChance, FName SocketName, FName CollisionPresetName, FName TagName)
{
	bool spawnedFlag = false;

	if (FMath::FRand() <= SpawnChance / 100.f) {
		FName SpawnSocket = SocketName; // Example socket, choose randomly as needed

		// Check if the socket exists
		if (!Mesh->DoesSocketExist(SpawnSocket)) {
			return false;
		}

		FVector SpawnLocation = Mesh->GetSocketLocation(SpawnSocket);
		FRotator SpawnRotation = Mesh->GetSocketRotation(SpawnSocket);
		// Spawn the actor
		AActor* NewActor = GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);

		if (!NewActor)
		{
			return false;
		}

		//add tag to the actor
		NewActor->Tags.Add(TagName);

		//attach actor to the socket
		NewActor->AttachToComponent(Mesh, FAttachmentTransformRules::KeepWorldTransform, SocketName);

		spawnedFlag = true;
	}

	return spawnedFlag;
}

UStaticMeshComponent* ARoomGenerator::SpawnMeshOnExistingMeshFloorSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName, FName CollisionPresetName, FName TagName)
{
	//bool spawnedFlag = false;

	if (!ExistingMeshComponent || !NewMesh)
	{
		return nullptr; // Validation to ensure there is an existing mesh component and a mesh to attach
	}


	UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this);

	// Create a new static mesh component dynamically
	if (FMath::FRand() <= SpawnChance / 100.f) {
		if (NewMeshComponent)
		{
			NewMeshComponent->SetStaticMesh(NewMesh); // Assign the new mesh

			//set collision
			NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			NewMeshComponent->SetCollisionProfileName(CollisionPresetName);

			//set mesh to be movable
			NewMeshComponent->SetMobility(EComponentMobility::Movable);

			NewMeshComponent->SetupAttachment(ExistingMeshComponent, SocketName); // Attach it to the socket on the existing mesh
			NewMeshComponent->RegisterComponent(); // Register the new component to make it active

			//add tag to the mesh component
			NewMeshComponent->ComponentTags.Add(TagName);

			//spawnedFlag = true;
		}
	}

	return NewMeshComponent;
}

bool ARoomGenerator::SpawnMeshOnExistingMeshWallSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName, FName TagName)
{
	bool spawnedFlag = false;

	if (!ExistingMeshComponent || !NewMesh)
	{
		return false; // Validation to ensure there is an existing mesh component and a mesh to attach
	}

	// Create a new static mesh component dynamically
	if (FMath::FRand() <= SpawnChance / 100.f) {
		UStaticMeshComponent* NewMeshComponent = NewObject<UStaticMeshComponent>(this);


		if (NewMeshComponent)
		{
			NewMeshComponent->SetStaticMesh(NewMesh); // Assign the new mesh

			//set collision
			NewMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			NewMeshComponent->SetCollisionProfileName("BlockAll");

			//set mesh to be movable
			NewMeshComponent->SetMobility(EComponentMobility::Movable);
			NewMeshComponent->SetupAttachment(ExistingMeshComponent, SocketName); // Attach it to the socket on the existing mesh

			NewMeshComponent->RegisterComponent(); // Register the new component to make it active

			//add tag to the mesh component
			NewMeshComponent->ComponentTags.Add(TagName);


			// Set position and rotation
			FVector SocketLocation = ExistingMeshComponent->GetSocketLocation(SocketName);
			FRotator SocketRotation = ExistingMeshComponent->GetSocketRotation(SocketName);

			// Calculate offset based on the bounding box of the mesh
			FBoxSphereBounds Bounds = NewMesh->GetBounds();
			float OffsetDistance = Bounds.BoxExtent.Y; // Assuming Y is the correct dimension to offset
			FVector Offset = SocketRotation.RotateVector(FVector(0.f, OffsetDistance, 0.f)); // Assuming Y-axis is outward

			NewMeshComponent->SetWorldLocation(SocketLocation + Offset);
			NewMeshComponent->SetWorldRotation(ExistingMeshComponent->GetSocketRotation(SocketName));

			spawnedFlag = true;
		}
	}

	return spawnedFlag;
}

bool ARoomGenerator::IsFloorCellNearWall(int32 CurrentNumWallsLength, int32 CurrentNumWallsWidth, FVector FloorPosition, float horizontalProximity)
{
	FVector RoomOrigin = GetActorLocation(); // Assuming this is the bottom-left corner of the room
	float CurrentRoomLength = CurrentNumWallsLength * FloorLength; // Total length of the room
	float CurrentRoomWidth = CurrentNumWallsWidth * FloorWidth; // Total width of the room

	// Calculate the edges of the room
	float LeftEdge = RoomOrigin.X;
	float RightEdge = RoomOrigin.X + CurrentRoomLength;
	float BottomEdge = RoomOrigin.Y;
	float TopEdge = RoomOrigin.Y + CurrentRoomWidth;

	// Check proximity to each wall
	if (FloorPosition.X - LeftEdge <= horizontalProximity || RightEdge - FloorPosition.X <= horizontalProximity ||
		FloorPosition.Y - BottomEdge <= horizontalProximity || TopEdge - FloorPosition.Y <= horizontalProximity) {
		return true;
	}

	return false;
}

void ARoomGenerator::AddPlayerStart(FVector Location)
{
	// Ensure you have access to the World object
	if (GetWorld())
	{
		APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
		if (PlayerController)
		{
			APawn* PlayerPawn = PlayerController->GetPawn();
			if (PlayerPawn)
			{
				// Set the player's position
				PlayerPawn->SetActorLocation(Location);

				// Optionally, adjust the player's orientation
				FRotator NewOrientation = FRotator(0.0f, 90.0f, 0.0f); // Adjust as needed
				PlayerPawn->SetActorRotation(NewOrientation);

				UE_LOG(LogTemp, Warning, TEXT("Player moved to %s"), *Location.ToString());
			}
		}
	}
}

bool ARoomGenerator::IsTooCloseToOtherLightSources(TArray<FVector> LightSourcesArray, FVector NewPosition, float MinDistance)
{
	for (FVector Position : LightSourcesArray)
	{
		if (FVector::Dist(NewPosition, Position) < MinDistance)
		{
			return true;
		}
	}
	return false;
}
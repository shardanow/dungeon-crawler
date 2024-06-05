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

    MinRoomLength = 2040;
    MaxRoomLength = 5080;
    MinRoomWidth = 2040;
    MaxRoomWidth = 5080;

    WallLength = 510;
    WallHeight = 510;
    WallThickness = 45;

    FloorWidth = 495;
    FloorLength = 495;
    FloorThickness = 30;

    DoorWidth = 510;
    DoorHeight = 500;
    DoorThickness = 45;

    // Initialize the spawn chances
    TreasureSpawnChance = 2;
    EnemySpawnChance = 10;
    WallTorchesSpawnChance = 10;
    WallDecorationSpawnChance = 10;
    FloorPillarsSpawnChance = 5;
    FloorDecorationSpawnChance = 2;
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

void ARoomGenerator::GenerateRoom()
{
    if (WallMeshes.Num() == 0 || FloorMeshes.Num() == 0) {
        UE_LOG(LogTemp, Warning, TEXT("Mesh arrays are empty."));   
        return;
    }

    // Generate room dimensions based on wall lengths and widths
    int32 NumWallsLength = FMath::RandRange(MinRoomLength / FloorLength, MaxRoomLength / FloorLength);
    int32 NumWallsWidth = FMath::RandRange(MinRoomWidth / FloorLength, MaxRoomWidth / FloorLength);

    float RoomLength = NumWallsLength * FloorLength;  // Exact multiple of wall length
    float RoomWidth = NumWallsWidth * FloorLength;  // Exact multiple of wall width


    // debug on screen
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Length: %f"), RoomLength));
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Room Width: %f"), RoomWidth));

    FVector Origin = GetActorLocation();

    // Draw the room outline for visual reference
    //DrawDebugBox(GetWorld(), Origin + FVector(RoomLength / 2, RoomWidth / 2, 0), FVector(RoomLength / 2, RoomWidth / 2, 10), FColor::Emerald, true, -1, 0, 10);

    // Walls along the length of the room
    for (int i = 0; i <= RoomLength / WallLength; i++) {
        //flag to check that something is spawned on current floor socket
        bool bSomethingSpawnedWall = false;
        bool bSomethingSpawnedOppositeWall = false;

        UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
        FVector WallPosition = Origin + FVector(i * WallLength, 0, 0);
        UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, 0, 0), TEXT("wall_left_bottom_corner"));
        //DrawDebugSphere(GetWorld(), WallPosition, 20, 12, FColor::Red, true, -1, 0, 2);

        // Adjust opposite wall position to start exactly from the corner
        UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
        FVector OppositeWallPosition = Origin + FVector(i * WallLength + WallLength, RoomWidth, 0); // Subtracting one WallLength
        UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 180, 0), TEXT("wall_right_bottom_corner"));
        //DrawDebugSphere(GetWorld(), OppositeWallPosition, 20, 12, FColor::Blue, true, -1, 0, 2);

        // spawn torches on wall sockets
        if (!bSomethingSpawnedWall && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center")) {
            bSomethingSpawnedWall = true;
        }
        
        if (!bSomethingSpawnedOppositeWall && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center")) {
            bSomethingSpawnedOppositeWall = true;
        }

        //spawn wall decoration on wall socket
        if (!bSomethingSpawnedWall && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], WallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center")) {
            DecoratedWallPositions.Add(WallPosition);
            bSomethingSpawnedWall = true;
        }

        if (!bSomethingSpawnedOppositeWall && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], OppositeWallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center")) {
            DecoratedWallPositions.Add(OppositeWallPosition);
            bSomethingSpawnedOppositeWall = true;
        }
    }


    // Walls along the width of the room
    for (int j = 0; j <= RoomWidth / WallLength; j++) {
        //flag to check that something is spawned on current wall socket name
        bool bSomethingSpawnedWall = false;
        bool bSomethingSpawnedOppositeWall = false;

        UStaticMesh* SelectedWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
        FVector WallPosition = Origin + FVector(0, j * WallLength + WallLength, 0); // Subtracting one WallLength
        UStaticMeshComponent* WallMeshComponent = SpawnMesh(SelectedWallMesh, WallPosition, FRotator(0, -90, 0), TEXT("wall_left_bottom_corner"));
        //DrawDebugSphere(GetWorld(), WallPosition, 20, 12, FColor::Yellow, true, -1, 0, 2);

        UStaticMesh* SelectedOppositeWallMesh = WallMeshes[FMath::RandRange(0, WallMeshes.Num() - 1)];
        FVector OppositeWallPosition = Origin + FVector(RoomLength, j * WallLength, 0);
        UStaticMeshComponent* OppositeWallMeshComponent = SpawnMesh(SelectedOppositeWallMesh, OppositeWallPosition, FRotator(0, 90, 0), TEXT("wall_right_bottom_corner"));
        //DrawDebugSphere(GetWorld(), OppositeWallPosition, 20, 12, FColor::Green, true, -1, 0, 2);

        // spawn torches on wall sockets
        if (!bSomethingSpawnedWall && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], WallMeshComponent, WallTorchesSpawnChance, "wall_center")) {
            bSomethingSpawnedWall = true;
        }

        if (!bSomethingSpawnedOppositeWall && SpawnActorOnSocket(WallTorchesBlueprints[FMath::RandRange(0, WallTorchesBlueprints.Num() - 1)], OppositeWallMeshComponent, WallTorchesSpawnChance, "wall_center")) {
            bSomethingSpawnedOppositeWall = true;
        }

        //spawn wall decoration on wall socket
        if (!bSomethingSpawnedWall && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], WallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center")) {
            DecoratedWallPositions.Add(WallPosition);
            bSomethingSpawnedWall = true;
        }

        if (!bSomethingSpawnedOppositeWall && SpawnMeshOnExistingMeshWallSocket(WallDecorationMeshes[FMath::RandRange(0, WallDecorationMeshes.Num() - 1)], OppositeWallMeshComponent, WallDecorationSpawnChance, "wall_bottom_center")) {
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
                UStaticMeshComponent* FloorMeshComponent = SpawnMesh(SelectedFloorMesh, FloorPosition, FRotator(0, 0, 0), NAME_None);

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

                        //spawn pillars on floor socket
                        if (!bSomethingSpawned && SpawnMeshOnExistingMeshFloorSocket(SelectedPillarMesh, FloorMeshComponent, 100, "spawn_center_point")) {
                            //draw debug sphere
                           // DrawDebugSphere(GetWorld(), FloorPosition, 20, 12, FColor::Red, true, -1, 0, 2);

                            bSomethingSpawned = true;
                        }
                    }
                }

                // Spawn treasure on floor socket
                if (!bSomethingSpawned && SpawnActorOnSocket(TreasureBlueprints[FMath::RandRange(0, TreasureBlueprints.Num() - 1)], FloorMeshComponent, TreasureSpawnChance, "spawn_center_point")) {
                    bSomethingSpawned = true;
                }

                // Spawn enemies on floor socket
                if (!bSomethingSpawned && SpawnActorOnSocket(EnemyBlueprints[FMath::RandRange(0, EnemyBlueprints.Num() - 1)], FloorMeshComponent, EnemySpawnChance, "spawn_center_point")) {
                    bSomethingSpawned = true;
                }


            }
        }
    }


   // Adding lights
   // URectLightComponent* RectLight = NewObject<URectLightComponent>(this);
   // RectLight->SetWorldLocation(Origin + FVector(RoomLength / 2, RoomWidth / 2, WallHeight-20)); // Adjust height as necessary
   // RectLight->SetWorldRotation(FRotator(-90, -90, 0)); // Pointing downwards
   // RectLight->SetSourceWidth(RoomWidth-50); // Slightly less than room dimensions
   // RectLight->SetSourceHeight(WallHeight-50); // Slightly less than room dimensions
   // RectLight->Intensity = 2500; // Dimmer, moody light
   // RectLight->LightColor = FColor(255, 184, 134); // Warm light
   // RectLight->BarnDoorAngle = 0; // Optional if supported
   // RectLight->BarnDoorLength = 0; // Optional if supported
   //// RectLight->SetSoftSourceEdge(75); // Soften the edge of the light source
   // RectLight->AttenuationRadius = RoomLength+ RoomWidth; // Controlled light spread
   // //RectLight->FalloffExponent = 4; // Quick decrease in intensity
   // RectLight->RegisterComponent(); // Register the component

    // Add a PlayerStart actor
    AddPlayerStart(Origin + FVector(RoomLength / 2, RoomWidth / 2, WallHeight-50));
}


UStaticMeshComponent* ARoomGenerator::SpawnMesh(UStaticMesh* Mesh, FVector Location, FRotator Rotation, FName SocketName)
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


bool ARoomGenerator::SpawnActorOnSocket(TSubclassOf<AActor> ActorClass, UStaticMeshComponent* Mesh, float SpawnChance, FName SocketName)
{
    bool spawnedFlag = false;

    if (FMath::FRand() <= SpawnChance / 100.f) {
        FName SpawnSocket = SocketName; // Example socket, choose randomly as needed
        FVector SpawnLocation = Mesh->GetSocketLocation(SpawnSocket);
        FRotator SpawnRotation = Mesh->GetSocketRotation(SpawnSocket);
        GetWorld()->SpawnActor<AActor>(ActorClass, SpawnLocation, SpawnRotation);
        spawnedFlag = true;
    }

    return spawnedFlag;
}

bool ARoomGenerator::SpawnMeshOnExistingMeshFloorSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName)
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


            spawnedFlag = true;
        }
    }

    return spawnedFlag;
}

bool ARoomGenerator::SpawnMeshOnExistingMeshWallSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName)
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

bool ARoomGenerator::IsFloorCellNearWall(int32 NumWallsLength, int32 NumWallsWidth, FVector FloorPosition, float horizontalProximity)
{
    FVector RoomOrigin = GetActorLocation(); // Assuming this is the bottom-left corner of the room
    float RoomLength = NumWallsLength * FloorLength; // Total length of the room
    float RoomWidth = NumWallsWidth * FloorWidth; // Total width of the room

    // Calculate the edges of the room
    float LeftEdge = RoomOrigin.X;
    float RightEdge = RoomOrigin.X + RoomLength;
    float BottomEdge = RoomOrigin.Y;
    float TopEdge = RoomOrigin.Y + RoomWidth;

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
// Fill out your copyright notice in the Description page of Project Settings.


#include "FloorGenerator.h"
#include "RoomGenerator.h"
#include <random>

// Sets default values
AFloorGenerator::AFloorGenerator()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GridWidth = 100;  // Adjust as needed
	GridHeight = 100; // Adjust as needed
	CellSize = 0.0f;  // Will be set based on roomInstance->FloorLength

	RoomJoinDistance = 8000.0f; // Adjust as needed
}

// Called when the game starts or when spawned
void AFloorGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Generate random number of rooms
	numberOfRooms = FMath::RandRange(numberOfRoomsMin, numberOfRoomsMax);

	//debug number of rooms
	UE_LOG(LogTemp, Warning, TEXT("Number of rooms: %d"), numberOfRooms);

	GridWidth = 200;  // Adjust as needed
	GridHeight = 200; // Adjust as needed
	CellSize = 0.0f;  // Will be set based on roomInstance->FloorLength

	RoomJoinDistance = 8000.0f; // Adjust as needed


	GenerateFloor();
}

// Called every frame
void AFloorGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFloorGenerator::GenerateFloor()
{
	isGenerating = false;
	if (isGenerating) {
		UE_LOG(LogTemp, Warning, TEXT("GenerateFloor called while already generating. Skipping to prevent recursion."));
		return;
	}

	isGenerating = true;
	UE_LOG(LogTemp, Warning, TEXT("Starting floor generation."));

	//tweak grid size according to the number of rooms
	GridWidth = numberOfRooms * 20;
	GridHeight = numberOfRooms * 20;


	for (int i = 0; i < numberOfRooms; i++) {

		// debug room number
		UE_LOG(LogTemp, Warning, TEXT("Generating room %d of %d"), i+1, numberOfRooms);

		// get the room blueprint and spawn it
		ARoomGenerator* roomGenerator = GetWorld()->SpawnActor<ARoomGenerator>(RoomBlueprint, FVector::ZeroVector, FRotator::ZeroRotator);

		//check if room generator is valid
		if (!roomGenerator)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn room generator"));
			return;
		}

		// Generate room position and create room in the world
		FVector newRoomPosition = CalculateRoomPosition(roomGenerator, RoomsData);
		ARoomGenerator* newRoom = GenerateRoomAtPosition(roomGenerator, newRoomPosition);

		if (i == 0)
		{
			newRoom->Tags.Add(FName("StartFloorRoom"));

			//debug sphere
			//DrawDebugSphere(GetWorld(), newRoomPosition, 100, 20, FColor::Blue, true);
		}
		else if (i == numberOfRooms - 1)
		{
			newRoom->Tags.Add(FName("EndFloorRoom"));

			//debug sphere
			//DrawDebugSphere(GetWorld(), newRoomPosition, 100, 20, FColor::Red, true);
		}


		// Store room info with ID
	  //  FIntPoint roomGridPos = FIntPoint(newRoomPosition.X / newRoom->RoomLength, newRoomPosition.Y / newRoom->RoomWidth);
	  //  RoomGrid.Add(roomGridPos, FRoomInfo(i, newRoom, newRoomPosition, CalculateBoundaries(newRoom)));

		// Store room info with ID
		RoomsData.Add(FRoomInfo(i, newRoom, newRoomPosition, CalculateBoundaries(newRoom))); // Store room info with ID
	}

	//move player to the first room center position
	FVector playerStartPos = RoomsData[0].RoomPosition + FVector(RoomsData[0].RoomInstance->RoomLength / 2, RoomsData[0].RoomInstance->RoomWidth / 2, 0);
	//add Z offset
	playerStartPos.Z += 100.0f;

	//move player to the first room center position
	RoomsData[0].RoomInstance->AddPlayerStart(playerStartPos);

	// Connect rooms
	FindAndConnectNearestRoom();

	isGenerating = false;
	UE_LOG(LogTemp, Warning, TEXT("Floor generation completed."));
}


ARoomGenerator* AFloorGenerator::GenerateRoomAtPosition(ARoomGenerator* roomInstance, FVector position)
{
	//ARoomGenerator* roomGenerator = GetWorld()->SpawnActor<ARoomGenerator>(ARoomGenerator::StaticClass(), position, FRotator::ZeroRotator);

	if (roomInstance)
	{
		roomInstance->GenerateRoom(position);
	}

	return roomInstance;
}

//regenerate floor
void AFloorGenerator::RegenerateFloor()
{
	// Destroy all rooms
	for (FRoomInfo& roomInfo : RoomsData)
	{
		if (roomInfo.RoomInstance)
		{
			roomInfo.RoomInstance->Destroy();
		}
	}

	// Clear room data
	RoomsData.Empty();

	// Generate new floor
	GenerateFloor();
}


TArray<FBoundary> AFloorGenerator::CalculateBoundaries(ARoomGenerator* room) {
	TArray<FBoundary> boundaries;
	FVector roomPos = room->GetActorLocation();
	float roomLength = room->RoomLength;
	float roomWidth = room->RoomWidth;

	// Assuming the room is aligned with the world axes
	boundaries.Add({ roomPos, roomPos + FVector(roomLength, 0, 0) }); // Bottom wall
	boundaries.Add({ roomPos, roomPos + FVector(0, roomWidth, 0) }); // Left wall
	boundaries.Add({ roomPos + FVector(roomLength, 0, 0), roomPos + FVector(roomLength, roomWidth, 0) }); // Top wall
	boundaries.Add({ roomPos + FVector(0, roomWidth, 0), roomPos + FVector(roomLength, roomWidth, 0) }); // Right wall

	//debug draw boundaries
	//for (const FBoundary& boundary : boundaries) {
	//	DrawDebugLine(GetWorld(), boundary.Start, boundary.End, FColor::Magenta, true);
	//}

	return boundaries;
}

void AFloorGenerator::FindAndConnectNearestRoom() {
	FVector RoomPointA;
	FVector RoomPointB;

	UStaticMeshComponent* hitComponentMeshRoomA = nullptr;
	UStaticMeshComponent* hitComponentMeshRoomB = nullptr;

	FVector hitComponentCenterMeshRoomA;
	FVector hitComponentCenterMeshRoomB;

	for (int i = 0; i < RoomsData.Num(); i++) {
		FRoomInfo& roomInfoA = RoomsData[i];
		FVector roomCenterA = roomInfoA.RoomPosition + FVector(roomInfoA.RoomInstance->RoomLength / 2, roomInfoA.RoomInstance->RoomWidth / 2, 0);

		float minDistance = FLT_MAX;

		for (int j = 0; j < RoomsData.Num(); j++) {
			//skip if same room
			if (i == j) continue;


			//get room info
			FRoomInfo& roomInfoB = RoomsData[j];

			//if some of joined A or B room id contains the same room id then skip
			if (
				roomInfoA.joinedRoomIDs.Contains(roomInfoB.RoomID)
				||
				roomInfoB.joinedRoomIDs.Contains(roomInfoA.RoomID)
				)
			{
				continue;
			}


			// Get the center of the room B
			FVector roomCenterB = roomInfoB.RoomPosition + FVector(roomInfoB.RoomInstance->RoomLength / 2, roomInfoB.RoomInstance->RoomWidth / 2, 0);

			// Assuming you have FVector roomCenterA and roomCenterB already defined
			FHitResult OutHit;
			FCollisionQueryParams CollisionParams;

			// Calculate the distance between the two rooms
			float distance = (roomCenterA - roomCenterB).Size();

			//get all decoration meshes
			TArray<UActorComponent*> DecorationsMeshes = roomInfoA.RoomInstance->GetComponentsByTag(UStaticMeshComponent::StaticClass(), "Decoration");
			//get all pillars meshes
			TArray<UActorComponent*> PillarsMeshes = roomInfoA.RoomInstance->GetComponentsByTag(UStaticMeshComponent::StaticClass(), "Pillar");

			//get all light attached actors to the room
			TArray<AActor*> AttachedActors;
			roomInfoA.RoomInstance->GetAttachedActors(AttachedActors);

			//ignore decoration meshes
			for (UActorComponent* DecorationMesh : DecorationsMeshes) {
				UStaticMeshComponent* DecorationMeshComponent = Cast<UStaticMeshComponent>(DecorationMesh);
				CollisionParams.AddIgnoredComponent(DecorationMeshComponent);
			}

			//ignore pillars meshes
			for (UActorComponent* PillarMesh : PillarsMeshes) {
				UStaticMeshComponent* PillarMeshComponent = Cast<UStaticMeshComponent>(PillarMesh);
				CollisionParams.AddIgnoredComponent(PillarMeshComponent);
			}

			//ignore actors by tag
			for (AActor* AttachedActor : AttachedActors) {
				if (AttachedActor->ActorHasTag("Light")
					|| AttachedActor->ActorHasTag("Enemy")
					|| AttachedActor->ActorHasTag("Treasure"))
				{
					CollisionParams.AddIgnoredActor(AttachedActor);
				}
			}


			//debug distance with room name
			//UE_LOG(LogTemp, Warning, TEXT("Distance between %s and %s: %f"), *roomInfoA.RoomInstance->GetName(), *roomInfoB.RoomInstance->GetName(), distance);

			if (distance < RoomJoinDistance) {
				minDistance = distance;


				//make array of closest rooms and distance
				TArray<FRoomInfo> closestRooms;
				TArray<float> closestDistances;

				// Add the distance to the array
				closestDistances.Add(distance);

				// Add the room to the array
				closestRooms.Add(roomInfoB);


				FVector RoomCenterANew = FVector(roomCenterA.X, roomCenterA.Y, roomCenterA.Z + roomInfoA.RoomInstance->WallHeight / 2);
				FVector RoomCenterBNew = FVector(roomCenterB.X, roomCenterB.Y, roomCenterB.Z + roomInfoB.RoomInstance->WallHeight / 2);

				//debug room name
				UE_LOG(LogTemp, Warning, TEXT("Room A: %s, Room B: %s"), *roomInfoA.RoomInstance->GetName(), *roomInfoB.RoomInstance->GetName());


				//calculate angle between two vectors
				FVector Direction = (RoomCenterBNew - RoomCenterANew).GetSafeNormal();
				FVector ProjectedDirection = Direction.ProjectOnTo(FVector(1, 0, 0));
				float angle = FMath::Acos(FVector::DotProduct(Direction, ProjectedDirection)) * 180 / PI;

				//debug angle
				UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), angle);


				if ((angle < 40 || angle > 60)) {
					//ignore the hit component
					bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, RoomCenterANew, RoomCenterBNew, ECC_Visibility, CollisionParams);
					if (bHit) {
						FVector intersectionPoint = OutHit.ImpactPoint;

						//make new line trace from intersection point to room center
						bool bHit2;
						FHitResult OutHit2;

						//ignore the hit component
						CollisionParams.AddIgnoredActor(roomInfoA.RoomInstance);


						// get the hit mesh component
						UStaticMeshComponent* hitComponentFirst = Cast<UStaticMeshComponent>(OutHit.Component.Get());

						FBoxSphereBounds Bounds = hitComponentFirst->Bounds;
						FVector hitFirstComponentCenter = Bounds.Origin;

						FVector AlignedPointB = GetAlignedPoint(hitFirstComponentCenter, RoomCenterBNew);



						bool isValidIntersection = true;


						if (hitComponentFirst->GetOwner()->GetName() != roomInfoA.RoomInstance->GetName()) {
							isValidIntersection = false;
						}



						if (isValidIntersection) {

							//draw debug arrow
							//DrawDebugDirectionalArrow(GetWorld(), RoomCenterANew, intersectionPoint, 120.0f, FColor::Purple, true, -1, 0, 5);


							//debug hit point
							//DrawDebugSphere(GetWorld(), intersectionPoint, 50, 20, FColor::Purple, true);



							FVector HitNormal = OutHit.ImpactNormal;

							// Example of setting the correct position and orientation
							FVector WallStartCorner; // Starting corner based on wall orientation
							FVector WallEndCorner;   // End corner based on wall orientation
							float WallWidth = 80.0f; // Assuming 10 units thick

							//debug HitNormal
							UE_LOG(LogTemp, Warning, TEXT("HitNormal of room %d is: %s"), roomInfoA.RoomID, *HitNormal.ToString());

							// Detect the wall hit based on HitNormal vector
							if (FVector::DotProduct(HitNormal, FVector(0, -1, 0)) > 0.9f) {
								WallStartCorner = FVector(roomInfoA.RoomPosition.X, roomInfoA.RoomPosition.Y + roomInfoA.RoomInstance->RoomWidth, roomInfoA.RoomPosition.Z);
								WallEndCorner = FVector(roomInfoA.RoomPosition.X + roomInfoA.RoomInstance->RoomLength, roomInfoA.RoomPosition.Y + roomInfoA.RoomInstance->RoomWidth, roomInfoA.RoomPosition.Z);

								// Debug room name and wall side
								UE_LOG(LogTemp, Warning, TEXT("Top wall hit detected of room %d."), roomInfoA.RoomID);
								// Debug dot product
								UE_LOG(LogTemp, Warning, TEXT("Top Dot Product: %f"), FVector::DotProduct(HitNormal, FVector(0, -1, 0)));
							}
							else if (FVector::DotProduct(HitNormal, FVector(1, 0, 0)) > 0.9f) {
								WallStartCorner = FVector(roomInfoA.RoomPosition.X, roomInfoA.RoomPosition.Y, roomInfoA.RoomPosition.Z);
								WallEndCorner = FVector(roomInfoA.RoomPosition.X, roomInfoA.RoomPosition.Y + roomInfoA.RoomInstance->RoomWidth, roomInfoA.RoomPosition.Z);

								// Debug room name and wall side
								UE_LOG(LogTemp, Warning, TEXT("Left wall hit detected of room %d."), roomInfoA.RoomID);
								// Debug dot product
								UE_LOG(LogTemp, Warning, TEXT("Left Dot Product: %f"), FVector::DotProduct(HitNormal, FVector(1, 0, 0)));
							}
							else if (FVector::DotProduct(HitNormal, FVector(-1, 0, 0)) > 0.9f) {
								WallStartCorner = FVector(roomInfoA.RoomPosition.X + roomInfoA.RoomInstance->RoomLength, roomInfoA.RoomPosition.Y, roomInfoA.RoomPosition.Z);
								WallEndCorner = FVector(roomInfoA.RoomPosition.X + roomInfoA.RoomInstance->RoomLength, roomInfoA.RoomPosition.Y + roomInfoA.RoomInstance->RoomWidth, roomInfoA.RoomPosition.Z);

								// Debug room name and wall side
								UE_LOG(LogTemp, Warning, TEXT("Right wall hit detected of room %d."), roomInfoA.RoomID);
								// Debug dot product
								UE_LOG(LogTemp, Warning, TEXT("Right Dot Product: %f"), FVector::DotProduct(HitNormal, FVector(-1, 0, 0)));
							}
							else if (FVector::DotProduct(HitNormal, FVector(0, 1, 0)) > 0.9f) {
								WallStartCorner = FVector(roomInfoA.RoomPosition.X, roomInfoA.RoomPosition.Y, roomInfoA.RoomPosition.Z);
								WallEndCorner = FVector(roomInfoA.RoomPosition.X + roomInfoA.RoomInstance->RoomLength, roomInfoA.RoomPosition.Y, roomInfoA.RoomPosition.Z);


								// Debug room name and wall side
								UE_LOG(LogTemp, Warning, TEXT("Bottom wall hit detected of room %d."), roomInfoA.RoomID);
								// Debug dot product
								UE_LOG(LogTemp, Warning, TEXT("Bottom Dot Product: %f"), FVector::DotProduct(HitNormal, FVector(0, 1, 0)));
							}
							else {
								// Debug room name and wall side
								UE_LOG(LogTemp, Warning, TEXT("No wall hit detected of room %d."), roomInfoA.RoomID);
								continue;
							}

							// Set the dimensions correctly based on the wall orientation
							FVector WallDimensions = FVector((WallEndCorner - WallStartCorner).Size(), WallWidth, roomInfoA.RoomInstance->WallHeight);

							// Calculate the rotation to align with the wall
							FVector DirectionVector = (WallEndCorner - WallStartCorner).GetSafeNormal();
							FQuat RotationQuat = FQuat::FindBetweenNormals(FVector::RightVector, DirectionVector);

							// Correct orientation around Z-axis if necessary
							float ZRotationDegrees = 90.0f; // Example degree for correction, adjust as needed
							FQuat ZRotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(ZRotationDegrees));
							RotationQuat *= ZRotation; // Combine the initial rotation with the Z-axis correction

							// Calculate the correct center of the box
							FVector BoxCenter = WallStartCorner + (WallEndCorner - WallStartCorner) / 2;
							BoxCenter.Z += roomInfoA.RoomInstance->WallHeight / 2;  // Adjusting height to the middle of the wall

							// Draw the debug box
							//DrawDebugBox(GetWorld(), BoxCenter, WallDimensions / 2, RotationQuat, FColor::Green, true, -1.0f, 0, 5);


							// Define the sweep box for querying mesh components
							FCollisionShape CollisionShape = FCollisionShape::MakeBox(WallDimensions / 2);
							TArray<FHitResult> OutHits;
							FCollisionQueryParams WallBoxCollisionParams(FName(TEXT("MeshQuery")), true, this);

							// Perform the sweep to get potential mesh components
							bool bHasHit = GetWorld()->SweepMultiByChannel(OutHits, BoxCenter, BoxCenter, RotationQuat, ECC_GameTraceChannel1, CollisionShape, WallBoxCollisionParams);

							TArray<UStaticMeshComponent*> MeshComponentsInsideBox;
							TArray<FVector> MeshCenters;

							if (bHasHit)
							{
								for (const FHitResult& Hit : OutHits)
								{
									UStaticMeshComponent* MeshComp = Cast<UStaticMeshComponent>(Hit.Component);
									if (MeshComp && MeshComp->ComponentHasTag("Wall"))
									{
										// Check if the component is truly within the rotated box bounds
										FBox MeshBox = MeshComp->Bounds.GetBox();
										FVector MeshCenter = MeshBox.GetCenter();

										//DrawDebugSphere(GetWorld(), MeshCenter, 50, 20, FColor::Red, true);

										// Optional: further refine check for inclusion within the oriented box
										//if (IsPointInsideOrientedBox(MeshCenter, BoxCenter, WallDimensions / 2, RotationQuat))
										//{
										MeshComponentsInsideBox.Add(MeshComp);
										MeshCenters.Add(MeshCenter);
										//}
									}
								}
							}


							int numComponents = MeshComponentsInsideBox.Num(); // Get the number of components in the array

							for (int m = 0; m < numComponents; ++m)
							{

								UStaticMeshComponent* currentComp = MeshComponentsInsideBox[m]; // Current component

								// Check if the component is truly within the rotated box bounds
								FBox MeshBox = currentComp->Bounds.GetBox();
								FVector MeshCenter = MeshBox.GetCenter();

								FVector AlignedPointRoomB = GetAlignedPoint(MeshCenter, RoomCenterBNew);

								//new line trace
								bHit2 = GetWorld()->LineTraceSingleByChannel(OutHit2, MeshCenter, AlignedPointRoomB, ECC_Visibility, CollisionParams);

								if (bHit2) {
									FVector intersectionPoint2 = OutHit2.ImpactPoint;
									//get the hit mesh component
									UStaticMeshComponent* hitComponentSecond = Cast<UStaticMeshComponent>(OutHit2.Component.Get());

									if (!hitComponentSecond) {
										continue;
									}

									FBoxSphereBounds BoundsSecond = hitComponentSecond->Bounds;
									FVector hitSecondComponentCenter = BoundsSecond.Origin;


									//get join room is valid
									if (CheckRoomCorridorJoinIsValid(RoomCenterANew, RoomCenterBNew, MeshCenter, roomInfoA.RoomInstance, roomInfoB.RoomInstance, CollisionParams)) {
										JoinRoomByCorridor(MeshCenter, intersectionPoint2, roomInfoA.RoomInstance, roomInfoB.RoomInstance, MeshCenter, hitSecondComponentCenter, currentComp, hitComponentSecond);

										//DrawDebugSphere(GetWorld(), MeshCenter, 80, 20, FColor::White, true);
										//Add the room to the joined rooms list
										roomInfoA.isJoined = true;
										roomInfoB.isJoined = true;
										roomInfoA.joinedRoomIDs.Add(roomInfoB.RoomID);
										roomInfoB.joinedRoomIDs.Add(roomInfoA.RoomID);



										//get array of static meshes near the intersection point A
										TArray<UStaticMeshComponent*> StaticMeshesA = GetStaticMeshesAtPoint(MeshCenter, 600.f, "Wall");
										TArray<UStaticMeshComponent*> StaticMeshesACorners = GetStaticMeshesAtPoint(MeshCenter, 600.f, "WallCorner");

										//join two arrays
										StaticMeshesA.Append(StaticMeshesACorners);

										//draw debug sphere
										//DrawDebugSphere(GetWorld(), MeshCenter, 600, 20, FColor::Black, true);

							
										for (UStaticMeshComponent* StaticMesh : StaticMeshesA) {
											StaticMesh->SetCollisionProfileName("IgnoreCameraAndCursor");

											StaticMesh->SetMaterial(0, CorridorOverlayMaterial);
											//debug hit point
											//DrawDebugSphere(GetWorld(), StaticMesh->GetComponentLocation(), 80, 20, FColor::Blue, true);
										}

										//get array of static meshes near the intersection point B
										TArray<UStaticMeshComponent*> StaticMeshesB = GetStaticMeshesAtPoint(hitSecondComponentCenter, 600.f, "Wall");
										TArray<UStaticMeshComponent*> StaticMeshesBCorners = GetStaticMeshesAtPoint(hitSecondComponentCenter, 600.f, "WallCorner");

										//join two arrays
										StaticMeshesB.Append(StaticMeshesBCorners);

										
										for (UStaticMeshComponent* StaticMesh : StaticMeshesB) {
											StaticMesh->SetCollisionProfileName("IgnoreCameraAndCursor");

											//apply new material to the mesh
											StaticMesh->SetMaterial(0, CorridorOverlayMaterial);

											//debug hit point
											//DrawDebugSphere(GetWorld(), StaticMesh->GetComponentLocation(), 80, 20, FColor::Blue, true);
										}

										//DrawDebugSphere(GetWorld(), hitSecondComponentCenter, 600, 20, FColor::Black, true);

										break;
									}
									else {
										//debug hit point
										//DrawDebugSphere(GetWorld(), MeshCenter, 100, 20, FColor::Black, true);
									}

								}


							}
						}



					}
				}
			}
		}
	}



	FinalizeLevel();
}

int AFloorGenerator::FindRoomIndexByID(int roomID) {
	for (int i = 0; i < RoomsData.Num(); i++) {
		if (RoomsData[i].RoomID == roomID) {
			return i;
		}
	}
	return -1; // Not found
}


void AFloorGenerator::FinalizeLevel() {
	// Logic to finalize the level setup after successful connections
	UE_LOG(LogTemp, Warning, TEXT("All rooms are interconnected! Level generation successful"));
}


bool AFloorGenerator::CheckRoomCorridorJoinIsValid(FVector RoomCenterAPoint, FVector RoomCenterBPoint, FVector HitPointCenter, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FCollisionQueryParams CollisionParams) {
	FHitResult OutHitLeft;
	FHitResult OutHitRight;

	FVector hitFirstComponentCenterLeft;
	FVector hitFirstComponentCenterRight;

	FVector HitPointDirection = (RoomCenterBPoint - RoomCenterAPoint).GetSafeNormal();

	//get the hit component nearest mesh
	if (FMath::Abs(HitPointDirection.X) > FMath::Abs(HitPointDirection.Y)) {
		hitFirstComponentCenterLeft = HitPointCenter - FVector(0, RoomInstanceA->WallLength, 0);
		hitFirstComponentCenterRight = HitPointCenter + FVector(0, RoomInstanceA->WallLength, 0);
	}
	else {
		hitFirstComponentCenterLeft = HitPointCenter - FVector(RoomInstanceA->WallLength, 0, 0);
		hitFirstComponentCenterRight = HitPointCenter + FVector(RoomInstanceA->WallLength, 0, 0);
	}

	FVector AlignedPointLeft = GetAlignedPoint(hitFirstComponentCenterLeft, RoomCenterBPoint);
	FVector AlignedPointRight = GetAlignedPoint(hitFirstComponentCenterRight, RoomCenterBPoint);

	bool bHitLeft = GetWorld()->LineTraceSingleByChannel(OutHitLeft, hitFirstComponentCenterLeft, AlignedPointLeft, ECC_Visibility, CollisionParams);
	bool bHitRight = GetWorld()->LineTraceSingleByChannel(OutHitRight, hitFirstComponentCenterRight, AlignedPointRight, ECC_Visibility, CollisionParams);


	if (bHitLeft && bHitRight)
	{
		//get the hit mesh component
		UStaticMeshComponent* hitComponentLeft = Cast<UStaticMeshComponent>(OutHitLeft.Component.Get());

		//get the hit mesh component
		UStaticMeshComponent* hitComponentRight = Cast<UStaticMeshComponent>(OutHitRight.Component.Get());


		if (hitComponentLeft && hitComponentLeft->GetOwner()->GetFName() != RoomInstanceB->GetName())
		{
			//debug arrow for hit point left and right
			//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, OutHitLeft.ImpactPoint, 120.0f, FColor::Yellow, true, -1, 0, 5);
			//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, OutHitRight.ImpactPoint, 120.0f, FColor::Yellow, true, -1, 0, 5);


			//debug left and right hit point
			//DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Red, true);
			//DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Red, true);


			//debug hit component name and room name
			//UE_LOG(LogTemp, Warning, TEXT("Hit Component Left Room Name: %s, From Room Name: %s, Target Room Name: %s"), *hitComponentLeft->GetOwner()->GetName(), *RoomInstanceA->GetName(), *RoomInstanceB->GetName());

			return false;
		}

		if (hitComponentRight && hitComponentRight->GetOwner()->GetFName() != RoomInstanceB->GetName())
		{
			//debug arrow for hit point left and right
			//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, OutHitLeft.ImpactPoint, 120.0f, FColor::Magenta, true, -1, 0, 5);
			//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, OutHitRight.ImpactPoint, 120.0f, FColor::Magenta, true, -1, 0, 5);


			//debug left and right hit point
			//DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Red, true);
			//DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Red, true);


			//UE_LOG(LogTemp, Warning, TEXT("Hit Component Right Room Name: %s, From Room Name: %s, Target Room Name: %s"), *hitComponentRight->GetOwner()->GetName(), *RoomInstanceA->GetName(), *RoomInstanceB->GetName());
			return false;
		}


		//debug arrow for hit point left and right
		//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, OutHitLeft.ImpactPoint, 120.0f, FColor::Blue, true, -1, 0, 5);
		//DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, OutHitRight.ImpactPoint, 120.0f, FColor::Blue, true, -1, 0, 5);

		//debug left and right hit point
		//DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Blue, true);
		//DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Blue, true);
	}
	else {
		return false;
	}



	return true;
}

void AFloorGenerator::JoinRoomByCorridor(FVector RoomPointA, FVector RoomPointB, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FVector hitComponentCenterMeshRoomA, FVector hitComponentCenterMeshRoomB, UStaticMeshComponent* hitComponentMeshRoomA, UStaticMeshComponent* hitComponentMeshRoomB) {
	// Calculate the width and height extents of the box
	float WidthExtent = RoomInstanceA->FloorWidth; // Desired width extent (half-size)
	float HeightExtent = RoomInstanceA->WallHeight / 2; // Desired height extent (half-size)

	//if StaticMeshRoomA or StaticMeshRoomB is not valid then return
	if (!hitComponentMeshRoomA || !hitComponentMeshRoomB) {
		return;
	}

	if (!hitComponentMeshRoomA->ComponentHasTag("Wall") || !hitComponentMeshRoomB->ComponentHasTag("Wall")) {
		return;
	}

	//debug hit point a
	//DrawDebugSphere(GetWorld(), RoomPointA, 50, 20, FColor::Green, true);

	//debug hit point b
	//DrawDebugSphere(GetWorld(), RoomPointB, 50, 20, FColor::Green, true);


	// Create a connection corridor between the two rooms
	CreateConnectionCorridor(RoomPointA, RoomPointB, WidthExtent, HeightExtent);

	// Replace the wall with a door for room A
	hitComponentMeshRoomA->SetStaticMesh(RoomInstanceA->DoorMeshes[FMath::RandRange(0, RoomInstanceA->DoorMeshes.Num() - 1)]);
	hitComponentMeshRoomA->SetCollisionProfileName("IgnoreCameraAndCursor");

	// Replace the wall with a door for room B
	hitComponentMeshRoomB->SetStaticMesh(RoomInstanceB->DoorMeshes[FMath::RandRange(0, RoomInstanceB->DoorMeshes.Num() - 1)]);
	hitComponentMeshRoomB->SetCollisionProfileName("IgnoreCameraAndCursor");

	FBoxSphereBounds BoundsSecond = hitComponentMeshRoomB->Bounds;
	FVector hitSecondComponentCenter = BoundsSecond.Origin;

	//get array of actors near the intersection point A
	TArray<AActor*> Actors = GetActorsAtPoint(hitComponentCenterMeshRoomA, 300.f, "Light");

	//destroy all actors near the intersection point A
	for (AActor* Actor : Actors) {
		Actor->Destroy();

		//debug destroy actor
		//UE_LOG(LogTemp, Warning, TEXT("Actor First Room Destroyed"));
	}

	//get array of static meshes near the intersection point A
	TArray<UStaticMeshComponent*> StaticMeshes = GetStaticMeshesAtPoint(hitComponentCenterMeshRoomA, 300.f, "Decoration");

	//destroy all static meshes near the intersection point A
	for (UStaticMeshComponent* StaticMesh : StaticMeshes) {
		StaticMesh->DestroyComponent();

		//debug destroy static mesh
		//UE_LOG(LogTemp, Warning, TEXT("Static Mesh First Room Destroyed"));
	}


	//get array of actors near the intersection point B
	TArray<AActor*> ActorsSecond = GetActorsAtPoint(hitComponentCenterMeshRoomB, 300.f, "Light");

	//destroy all actors near the intersection point B
	for (AActor* Actor : ActorsSecond) {
		Actor->Destroy();

		//debug destroy actor
		//UE_LOG(LogTemp, Warning, TEXT("Actor Second Room Destroyed"));
	}

	//get array of static meshes near the intersection point B
	TArray<UStaticMeshComponent*> StaticMeshesSecond = GetStaticMeshesAtPoint(hitComponentCenterMeshRoomB, 300.f, "Decoration");

	//destroy all static meshes near the intersection point
	for (UStaticMeshComponent* StaticMesh : StaticMeshesSecond) {
		StaticMesh->DestroyComponent();

		//debug destroy static mesh
		//UE_LOG(LogTemp, Warning, TEXT("Static Mesh Second Room Destroyed"));
	}




	//debug room id
	//UE_LOG(LogTemp, Warning, TEXT("Room %d is joined with Room %d"), *RoomInstanceA->GetName(), *RoomInstanceB->GetName());


	//// Add the room to the joined rooms list
	//roomInfoA.isJoined = true;
	//roomInfoB.isJoined = true;
	//roomInfoA.joinedRoomIDs.Add(roomInfoB.RoomID);
	//roomInfoB.joinedRoomIDs.Add(roomInfoA.RoomID);
}

void AFloorGenerator::DrawDebugBoxWithFixedSize(FVector Point1, FVector Point2, float WidthExtent, float HeightExtent)
{
	// Calculate the midpoint between the two points
	FVector Midpoint = (Point1 + Point2) / 2.0f;

	// Calculate the direction vector and normalize it
	FVector Direction = (Point2 - Point1).GetSafeNormal();

	// Calculate the length of the line segment
	float LineLength = FVector::Dist(Point1, Point2);

	// Set the extents of the box
	FVector BoxExtents(LineLength / 2.0f, WidthExtent, HeightExtent);

	// Calculate the rotation based on the direction vector
	FRotator Rotation = Direction.Rotation();
	FQuat Orientation = FQuat(Rotation);

	// Draw the debug box
	//DrawDebugBox(GetWorld(), Midpoint, BoxExtents, Orientation, FColor::White, true, 5.0f, 0, 5.0f);
}

void AFloorGenerator::CreateConnectionCorridor(FVector Point1, FVector Point2, float WidthExtent, float HeightExtent)
{
	// Calculate the midpoint between the two points
	FVector Midpoint = (Point1 + Point2) / 2.0f;

	// Calculate the direction vector and normalize it
	FVector Direction = (Point2 - Point1).GetSafeNormal();

	// Calculate the length of the line segment
	float LineLength = FVector::Dist(Point1, Point2);

	// Set the extents of the box
	FVector BoxExtents(LineLength / 2.0f, WidthExtent, HeightExtent);

	// Calculate rotation to make the forward direction of the corridor align with the direction from Point1 to Point2
	FRotator Rotation = Direction.Rotation();
	FQuat Orientation = FQuat(Rotation);

	// Determine if we need to rotate the room dimensions
	bool bShouldRotate = FMath::Abs(Direction.Y) > FMath::Abs(Direction.X);
	float RoomLength = bShouldRotate ? WidthExtent * 2 : LineLength;
	float RoomWidth = bShouldRotate ? LineLength : WidthExtent * 2;

	FName DisableWallSide = bShouldRotate ? FName("Width") : FName("Length");

	// Calculate the world position of the box center
	FVector BoxCenter = Midpoint;
	FVector BoxCenterDebug = Midpoint;
	BoxCenter.Z = 0; // Ensure Z coordinate is 0

	// Draw debug directional arrow and sphere at the midpoint
	//DrawDebugDirectionalArrow(GetWorld(), Point1, Point2, 120.0f, FColor::White, true, -1, 0, 5);
	//DrawDebugSphere(GetWorld(), BoxCenter, 50, 20, FColor::Green, true);

	// Adjust the spawn function to include the correct rotation
	ARoomGenerator* CorridorGenerator = GetWorld()->SpawnActor<ARoomGenerator>(RoomBlueprint, BoxCenter, Rotation);

	if (!CorridorGenerator)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn Corridor generator"));
		return;
	}

	// Generate the corridor size with dimensions switched if necessary
	CorridorGenerator->FixedRoomSizeGenerate(RoomLength, RoomWidth);

	// Generate the corridor at the location with correct orientation
	CorridorGenerator->GenerateCorridor(BoxCenter, DisableWallSide);

	//move corridor to the correct position
	CorridorGenerator->SetActorLocation(BoxCenter);

	// Draw the debug box
	//DrawDebugBox(GetWorld(), BoxCenterDebug, BoxExtents, Orientation, FColor::White, true, 5.0f, 0, 5.0f);
}


FVector AFloorGenerator::GetAlignedPoint(const FVector& Origin, const FVector& Target)
{
	// Determine the primary axis for alignment
	FVector PrimaryAxis = DeterminePrimaryAxis(Target - Origin);

	FVector Direction = (Target - Origin).GetSafeNormal();
	FVector ProjectedDirection = Direction.ProjectOnTo(PrimaryAxis);
	return Origin + ProjectedDirection * (Target - Origin).Size();
}

FVector AFloorGenerator::DeterminePrimaryAxis(const FVector& Vector)
{
	FVector AbsVector = Vector.GetAbs();
	if (AbsVector.X >= AbsVector.Y && AbsVector.X >= AbsVector.Z)
	{
		return FVector(1.f, 0.f, 0.f); // Align along X-axis
	}
	else if (AbsVector.Y >= AbsVector.X && AbsVector.Y >= AbsVector.Z)
	{
		return FVector(0.f, 1.f, 0.f); // Align along Y-axis
	}
	else
	{
		return FVector(0.f, 0.f, 1.f); // Align along Z-axis
	}
}

// Function to perform the actual query
TArray<UStaticMeshComponent*> AFloorGenerator::GetStaticMeshesAtPoint(FVector Point, float Radius, FName TagName)
{
	TArray<FOverlapResult> OverlapResults;
	TArray<UStaticMeshComponent*> StaticMeshComponents;

	// Setup query parameters
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = false;

	// Define the collision shape (sphere)
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);

	// Perform the overlap query
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, Point, FQuat::Identity, ECC_WorldStatic, CollisionShape, QueryParams))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Result.GetComponent());
			if (StaticMeshComponent && StaticMeshComponent->ComponentHasTag(TagName))
			{
				StaticMeshComponents.Add(StaticMeshComponent);

				//draw debug sphere with radius
				//DrawDebugSphere(GetWorld(), Point, Radius, 20, FColor::Blue, true);
			}
		}
	}

	return StaticMeshComponents;
}

TArray<AActor*> AFloorGenerator::GetActorsAtPoint(FVector Point, float Radius, FName TagName)
{
	TArray<FOverlapResult> OverlapResults;
	TArray<AActor*> ActorsAtPoint;

	// Setup query parameters
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnPhysicalMaterial = false;

	// Define the collision shape (sphere)
	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(Radius);

	// Perform the overlap query
	if (GetWorld()->OverlapMultiByChannel(OverlapResults, Point, FQuat::Identity, ECC_WorldDynamic, CollisionShape, QueryParams))
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			AActor* Actor = Result.GetActor();
			if (Actor && Actor->ActorHasTag(TagName))
			{
				ActorsAtPoint.Add(Actor);

				//draw debug sphere with radius
				//DrawDebugSphere(GetWorld(), Point, Radius, 20, FColor::Yellow, true);
			}
		}
	}

	return ActorsAtPoint;
}


void AFloorGenerator::InitializeGrid(int gridWidth, int gridHeight, float cellSize) {
	GridWidth = gridWidth;
	GridHeight = gridHeight;
	CellSize = cellSize;

	Grid.SetNum(GridWidth);
	for (int i = 0; i < GridWidth; ++i) {
		Grid[i].SetNum(GridHeight);
		for (int j = 0; j < GridHeight; ++j) {
			Grid[i][j] = false; // Initialize all cells as unoccupied
		}
	}


	//debug grid width and height as box

	DrawDebugBox(GetWorld(), FVector(GridWidth * CellSize / 2.0f, GridHeight * CellSize / 2.0f, 0.0f), FVector(GridWidth * CellSize / 2.0f, GridHeight * CellSize / 2.0f, 50.0f), FColor::White, true, -1.0f, 0, 2.0f);
}

FVector AFloorGenerator::CalculateRoomPosition(ARoomGenerator* roomInstance, TArray<FRoomInfo>& existingRooms) {
	FVector position;
	int maxAttempts = 100;
	int attempts = 0;

	roomInstance->RandomRoomSizeGenerate();

	if (CellSize == 0.0f) {
		CellSize = roomInstance->FloorLength;
		InitializeGrid(GridWidth, GridHeight, CellSize);
	}

	// Mark existing rooms and buffer zones on the grid
	for (const FRoomInfo& roomInfo : existingRooms) {
		ARoomGenerator* room = roomInfo.RoomInstance;
		FVector roomPos = roomInfo.RoomPosition;

		int startX = FMath::FloorToInt(roomPos.X / CellSize) - 1; // Expand to include buffer zone
		int startY = FMath::FloorToInt(roomPos.Y / CellSize) - 1; // Expand to include buffer zone
		int endX = startX + FMath::CeilToInt(room->RoomLength / CellSize) + 1; // Expand to include buffer zone
		int endY = startY + FMath::CeilToInt(room->RoomWidth / CellSize) + 1; // Expand to include buffer zone

		for (int x = startX; x <= endX; ++x) {
			for (int y = startY; y <= endY; ++y) {
				if (x >= 0 && y >= 0 && x < GridWidth && y < GridHeight) {
					Grid[x][y] = true; // Mark cell as occupied
				}
			}
		}
	}

	// Draw debug grid - only draw occupied cells and their immediate surroundings
	//for (int x = 0; x < GridWidth; ++x) {
	//	for (int y = 0; y < GridHeight; ++y) {
	//		if (Grid[x][y]) {
	//			for (int dx = -1; dx <= 1; ++dx) {
	//				for (int dy = -1; dy <= 1; ++dy) {
	//					int nx = x + dx;
	//					int ny = y + dy;
	//					if (nx >= 0 && ny >= 0 && nx < GridWidth && ny < GridHeight) {
	//						FVector cellCenter(nx * CellSize + CellSize / 2.0f, ny * CellSize + CellSize / 2.0f, 0.0f);
	//						FColor cellColor = Grid[nx][ny] ? FColor::Red : FColor::Green;
	//						DrawDebugBox(GetWorld(), cellCenter, FVector(CellSize / 2.0f, CellSize / 2.0f, 50.0f), cellColor, true, -1.0f, 0, 2.0f);
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

// Draw debug grid - draw one box per room and another for the surroundings
	for (const FRoomInfo& roomInfo : existingRooms) {
		ARoomGenerator* room = roomInfo.RoomInstance;
		FVector roomPos = roomInfo.RoomPosition;

		// Calculate the center of the room in the grid
		FVector roomCenter = roomPos + FVector(room->RoomLength / 2.0f, room->RoomWidth / 2.0f, 0.0f);

		// Calculate the size of the box to draw for the room (slightly larger)
		FVector roomBoxExtent = FVector((room->RoomLength / 2.0f) + CellSize, (room->RoomWidth / 2.0f) + CellSize, 50.0f);

		// Calculate the size of the box to draw for the surroundings (one cell larger than room box)
		FVector bufferBoxExtent = FVector((room->RoomLength / 2.0f) + 2 * CellSize, (room->RoomWidth / 2.0f) + 2 * CellSize, 50.0f);

		// Draw the box for the room
		//DrawDebugBox(GetWorld(), roomCenter, roomBoxExtent, FColor::Red, true, -1.0f, 0, 2.0f);

		// Draw the box for the surroundings
		//DrawDebugBox(GetWorld(), roomCenter, bufferBoxExtent, FColor::Green, true, -1.0f, 0, 2.0f);
	}

	// Directions for placement (right, left, up, down)
	TArray<FVector> directions = { FVector(1, 0, 0), FVector(-1, 0, 0), FVector(0, 1, 0), FVector(0, -1, 0) };

	// Try to place the new room on the grid
	while (attempts < maxAttempts) {
		attempts++;

		bool roomPlaced = false;

		// Shuffle directions
		for (int i = directions.Num() - 1; i > 0; i--) {
			int j = FMath::RandRange(0, i);
			directions.Swap(i, j);
		}

		for (const FRoomInfo& lastRoomInfo : existingRooms) {
			FVector basePosition = lastRoomInfo.RoomPosition;
			ARoomGenerator* lastRoom = lastRoomInfo.RoomInstance;

			for (const auto& dir : directions) {
				// Place the room with minimal buffer (1 cell size)
				FVector newPosition = basePosition + dir * (lastRoom->RoomLength + CellSize);

				int gridX = FMath::FloorToInt(newPosition.X / CellSize);
				int gridY = FMath::FloorToInt(newPosition.Y / CellSize);

				int roomLengthInCells = FMath::CeilToInt(roomInstance->RoomLength / CellSize);
				int roomWidthInCells = FMath::CeilToInt(roomInstance->RoomWidth / CellSize);

				bool canPlaceRoom = true;
				for (int x = gridX; x < gridX + roomLengthInCells && canPlaceRoom; ++x) {
					for (int y = gridY; y < gridY + roomWidthInCells && canPlaceRoom; ++y) {
						if (x >= GridWidth || y >= GridHeight || x < 0 || y < 0 || Grid[x][y]) {
							canPlaceRoom = false;
						}
					}
				}

				if (canPlaceRoom) {
					// Mark cells as occupied
					for (int x = gridX; x < gridX + roomLengthInCells; ++x) {
						for (int y = gridY; y < gridY + roomWidthInCells; ++y) {
							if (x >= 0 && y >= 0 && x < GridWidth && y < GridHeight) {
								Grid[x][y] = true;
							}
						}
					}

					// Calculate the position in the world
					position.X = gridX * CellSize;
					position.Y = gridY * CellSize;
					position.Z = 0.0f;

					// Draw debug box for the room
					FVector roomCenter = FVector(position.X + roomInstance->RoomLength / 2.0f, position.Y + roomInstance->RoomWidth / 2.0f, 0.0f);
					FVector roomExtent = FVector(roomInstance->RoomLength / 2.0f, roomInstance->RoomWidth / 2.0f, 50.0f);
					//DrawDebugBox(GetWorld(), roomCenter, roomExtent, FColor::Blue, true, -1.0f, 0, 2.0f);

					roomPlaced = true;

					//debug room position box with additionall cell size around the room
					//DrawDebugBox(GetWorld(), roomCenter, roomExtent + FVector(CellSize, CellSize, 0), FColor::Blue, true, -1.0f, 0, 5.0f);

					//debug room position and room name
					//UE_LOG(LogTemp, Warning, TEXT("Room %s placed at position: %s"), *roomInstance->GetName(), *position.ToString());

					return position;
				}
			}
		}
	}

	if (attempts >= maxAttempts && existingRooms.Num() <= 0) {
		// Draw debug sphere at zero vector
		DrawDebugSphere(GetWorld(), FVector::ZeroVector, 50.0f, 20, FColor::Green, true, -1.0f, 0, 2.0f);
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Failed to place room after %d attempts, falling back to zero vector"), maxAttempts);
	}

	return FVector::ZeroVector;
}
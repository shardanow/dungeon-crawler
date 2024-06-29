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

	GridWidth = 100;  // Adjust as needed
	GridHeight = 100; // Adjust as needed
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
	for (int i = 0; i < numberOfRooms; i++) {

		// debug room number
		UE_LOG(LogTemp, Warning, TEXT("Generating room %d of %d"), i, numberOfRooms);

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
}



bool AFloorGenerator::IsPositionValid(const FVector& position, float roomLength, float roomWidth, const TArray<FRoomInfo>& existingRooms) {
	for (const FRoomInfo& roomInfo : existingRooms) {
		if (RoomsOverlap(position, roomLength, roomWidth, roomInfo.RoomPosition, roomInfo.RoomInstance->RoomLength, roomInfo.RoomInstance->RoomWidth)) {
			return false;
		}
	}
	return true;
}



bool AFloorGenerator::RoomsOverlap(const FVector& newPos, float newLength, float newWidth, const FVector& existingPos, float existingLength, float existingWidth)
{
	FVector newCenter = newPos + FVector(newLength / 2, newWidth / 2, 0);
	FVector existingCenter = existingPos + FVector(existingLength / 2, existingWidth / 2, 0);

	// Ensure dimensions used here accurately reflect the room size without any additional buffer
	FVector newHalfExtents = FVector(newLength / 2, newWidth / 2, 50); // Assuming Z is just for visualization height
	FVector existingHalfExtents = FVector(existingLength / 2, existingWidth / 2, 50);

	// Check for overlap
	if (FMath::Abs(newCenter.X - existingCenter.X) < (newLength + existingLength) / 2 &&
		FMath::Abs(newCenter.Y - existingCenter.Y) < (newWidth + existingWidth) / 2)
	{
		// DrawDebugBox(GetWorld(), newCenter, newHalfExtents, FColor::Red, true);
		return true;  // Overlap detected
	}

	DrawDebugBox(GetWorld(), existingCenter, existingHalfExtents, FColor::Blue, true);
	return false;  // No overlap
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
	for (const FBoundary& boundary : boundaries) {
		DrawDebugLine(GetWorld(), boundary.Start, boundary.End, FColor::Magenta, true);
	}

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

		FRoomInfo* closestRoom = nullptr;
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
			UE_LOG(LogTemp, Warning, TEXT("Distance between %s and %s: %f"), *roomInfoA.RoomInstance->GetName(), *roomInfoB.RoomInstance->GetName(), distance);

			if (distance < RoomJoinDistance) {
				minDistance = distance;
				closestRoom = &roomInfoB;




				//make array of closest rooms and distance
				TArray<FRoomInfo> closestRooms;
				TArray<float> closestDistances;

				// Add the distance to the array
				closestDistances.Add(distance);

				// Add the room to the array
				closestRooms.Add(roomInfoB);

				//debug room name

				FVector RoomCenterANew = FVector(roomCenterA.X, roomCenterA.Y, roomCenterA.Z + 100.f);
				FVector RoomCenterBNew = FVector(roomCenterB.X, roomCenterB.Y, roomCenterB.Z + 100.f);

				//debug room name
				UE_LOG(LogTemp, Warning, TEXT("Room A: %s, Room B: %s"), *roomInfoA.RoomInstance->GetName(), *roomInfoB.RoomInstance->GetName());


				//calculate angle between two vectors
				FVector Direction = (RoomCenterBNew - RoomCenterANew).GetSafeNormal();
				FVector ProjectedDirection = Direction.ProjectOnTo(FVector(1, 0, 0));
				float angle = FMath::Acos(FVector::DotProduct(Direction, ProjectedDirection)) * 180 / PI;

				//debug angle
				UE_LOG(LogTemp, Warning, TEXT("Angle: %f"), angle);


				if ((angle < 40 || angle > 75)) {
					//ignore the hit component
					bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, RoomCenterANew, RoomCenterBNew, ECC_Visibility, CollisionParams);
					if (bHit) {
						FVector intersectionPoint = OutHit.ImpactPoint;

						//draw debug line
						DrawDebugLine(GetWorld(), RoomCenterANew, intersectionPoint, FColor::Purple, true, -1, 0, 5);


						//debug hit point
						DrawDebugSphere(GetWorld(), intersectionPoint, 50, 20, FColor::Purple, true);


						// get the hit mesh component
						UStaticMeshComponent* hitComponentFirst = Cast<UStaticMeshComponent>(OutHit.Component.Get());

						if (hitComponentFirst && hitComponentFirst->ComponentHasTag("WallCorner")) {
							//debug hit component tag with room id and tag
							UE_LOG(LogTemp, Warning, TEXT("Hit Corner Component %d Tag: %s"), roomInfoA.RoomID, *hitComponentFirst->ComponentTags[0].ToString());
						}


						// Check if the hit component is a wall and act accordingly based on the angle to prevent diagonal joins
						if (hitComponentFirst && hitComponentFirst->ComponentHasTag("Wall") && (angle < 40 || angle > 70)) {
							//make new line trace from intersection point to room center
							bool bHit2;
							FHitResult OutHit2;
							FHitResult OutHitLeft;
							FHitResult OutHitRight;

							FVector hitFirstComponentCenterLeft;
							FVector hitFirstComponentCenterRight;


							//ignore the hit component
							CollisionParams.AddIgnoredActor(roomInfoA.RoomInstance);

							FBoxSphereBounds Bounds = hitComponentFirst->Bounds;
							FVector hitFirstComponentCenter = Bounds.Origin;

							FVector AlignedPointB = GetAlignedPoint(hitFirstComponentCenter, RoomCenterBNew);

							//draw debug line
							// Draw the initial position and the calculated end position for verification
							//DrawDebugSphere(GetWorld(), intersectionPoint, 50, 20, FColor::Green, true);
						   // DrawDebugSphere(GetWorld(), AlignedPointB, 50, 20, FColor::Red, true);

							//draw debug line
							//DrawDebugLine(GetWorld(), intersectionPoint, AlignedPointB, FColor::Green, true, -1, 0, 5);

							//Detect the hit component and calculate center point of the hit component
						   // FVector hitComponentCenter = hitComponent->GetComponentLocation();





							//new line trace
							bHit2 = GetWorld()->LineTraceSingleByChannel(OutHit2, hitFirstComponentCenter, AlignedPointB, ECC_Visibility, CollisionParams);


							//if (bHit2)
							//{
							//	//get the hit mesh component
							//	UStaticMeshComponent* hitComponentSecondCheck = Cast<UStaticMeshComponent>(OutHit2.Component.Get());

							//	if (!hitComponentSecondCheck) {
							//		continue;
							//	}

							//	//get the hit mesh component tag
							//	if(hitComponentSecondCheck->ComponentHasTag("WallCorner")) {
							//		
							//		//choose dirrection
							//		FVector CurrentPointDirectionFixed = (RoomCenterBNew - RoomCenterANew).GetSafeNormal();

							//		//debug hit point
							//		DrawDebugSphere(GetWorld(), OutHit2.ImpactPoint, 50, 20, FColor::Red, true);

							//		//get the hit component nearest mesh
							//		if (FMath::Abs(CurrentPointDirectionFixed.X) > FMath::Abs(CurrentPointDirectionFixed.Y)) {
							//			//hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
							//			hitFirstComponentCenter = hitFirstComponentCenter - FVector(0, roomInfoA.RoomInstance->WallLength, 0);
							//		}
							//		else {
							//			hitFirstComponentCenter = hitFirstComponentCenter - FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
							//			//hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(0, roomInfoA.RoomInstance->WallLength, 0);
							//		}

							//		FVector AlignedPointBFixed = GetAlignedPoint(hitFirstComponentCenter, RoomCenterBNew);

							//		//FHitResult OutHit2;
							//		//new line trace
							//		bHit2 = GetWorld()->LineTraceSingleByChannel(OutHit2, hitFirstComponentCenter, AlignedPointBFixed, ECC_Visibility, CollisionParams);

							//		//get the hit mesh component
							//		UStaticMeshComponent* hitComponentSecondNew = Cast<UStaticMeshComponent>(OutHit2.Component.Get());


							//		//draw debug sphere
							//		DrawDebugSphere(GetWorld(), hitFirstComponentCenter, 50, 20, FColor::White, true);

							//		if (!hitComponentSecondNew) {
							//			continue;
							//		}

							//		//get the hit mesh component tag
							//		if (hitComponentSecondNew->ComponentHasTag("WallCorner")) {
							//			//debug hit point
							//			DrawDebugSphere(GetWorld(), OutHit2.ImpactPoint, 50, 20, FColor::Black, true);

							//			continue;
							//		}

							//		DrawDebugSphere(GetWorld(), OutHit2.ImpactPoint, 50, 20, FColor::Yellow, true);


							//		hitComponentFirst = hitComponentSecondNew;
							//	}
							//}

							//get join room is valid
							if (!CheckRoomCorridorJoinIsValid(RoomCenterANew, RoomCenterBNew, hitFirstComponentCenter, roomInfoA.RoomInstance, roomInfoB.RoomInstance, CollisionParams)) {
								continue;
							}


							////choose dirrection
							//FVector HitPointDirection = (RoomCenterBNew - RoomCenterANew).GetSafeNormal();

							////get the hit component nearest mesh
							//if (FMath::Abs(HitPointDirection.X) > FMath::Abs(HitPointDirection.Y)) {
							//	hitFirstComponentCenterLeft = hitFirstComponentCenter - FVector(0, roomInfoA.RoomInstance->WallLength, 0);
							//	hitFirstComponentCenterRight = hitFirstComponentCenter + FVector(0, roomInfoA.RoomInstance->WallLength, 0);
							//}
							//else {
							//	hitFirstComponentCenterLeft = hitFirstComponentCenter - FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
							//	hitFirstComponentCenterRight = hitFirstComponentCenter + FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
							//}


							//FVector AlignedPointLeft = GetAlignedPoint(hitFirstComponentCenterLeft, RoomCenterBNew);
							//FVector AlignedPointRight = GetAlignedPoint(hitFirstComponentCenterRight, RoomCenterBNew);



							//bool bHitLeft = GetWorld()->LineTraceSingleByChannel(OutHitLeft, hitFirstComponentCenterLeft, AlignedPointLeft, ECC_Visibility, CollisionParams);
							//bool bHitRight = GetWorld()->LineTraceSingleByChannel(OutHitRight, hitFirstComponentCenterRight, AlignedPointRight, ECC_Visibility, CollisionParams);



						

							//if (bHitLeft && bHitRight)
							//{

							//	//get the hit mesh component
							//	UStaticMeshComponent* hitComponentLeft = Cast<UStaticMeshComponent>(OutHitLeft.Component.Get());

							//	//get the hit mesh component
							//	UStaticMeshComponent* hitComponentRight = Cast<UStaticMeshComponent>(OutHitRight.Component.Get());


							//	if (hitComponentLeft && hitComponentLeft->GetOwner()->GetFName() != roomInfoB.RoomInstance->GetName())
							//	{
							//		//debug arrow for hit point left and right
							//		DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, AlignedPointLeft, 120.0f, FColor::Blue, true, -1, 0, 5);
							//		DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, AlignedPointRight, 120.0f, FColor::Blue, true, -1, 0, 5);


							//		//debug left and right hit point
							//		DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Blue, true);
							//		DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Blue, true);


							//		UE_LOG(LogTemp, Warning, TEXT("Hit Component Left Actor Name: %s"), *roomInfoB.RoomInstance->GetName());
							//		continue;
							//	}

							//	if (hitComponentRight && hitComponentRight->GetOwner()->GetFName() != roomInfoB.RoomInstance->GetName())
							//	{
							//		//debug arrow for hit point left and right
							//		DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, AlignedPointLeft, 120.0f, FColor::Blue, true, -1, 0, 5);
							//		DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, AlignedPointRight, 120.0f, FColor::Blue, true, -1, 0, 5);


							//		//debug left and right hit point
							//		DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Blue, true);
							//		DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Blue, true);


							//		UE_LOG(LogTemp, Warning, TEXT("Hit Component Right Actor Name: %s"), *roomInfoB.RoomInstance->GetName());
							//		continue;
							//	}
							//}




							if (bHit2) {
								FVector intersectionPoint2 = OutHit2.ImpactPoint;
								//get the hit mesh component
								UStaticMeshComponent* hitComponentSecond = Cast<UStaticMeshComponent>(OutHit2.Component.Get());

								if (!hitComponentSecond) {
									continue;
								}



								// FCollisionShape::MakeBox according to the hit component bounds
								FBoxSphereBounds BoundsTestSecond = hitComponentSecond->Bounds;
								FVector hitSecondComponentNewCenter = BoundsTestSecond.Origin;




								//get hit component actor name
								FName hitComponentActorName = hitComponentSecond->GetOwner()->GetFName();

								//draw debug line between intersection points
								//DrawDebugLine(GetWorld(), hitFirstComponentCenter, intersectionPoint2, FColor::Red, true, -1, 0, 5);

								//debug hit point
								//DrawDebugSphere(GetWorld(), intersectionPoint2, 50, 20, FColor::Red, true);


								//debug hit component actor name
								UE_LOG(LogTemp, Warning, TEXT("Hit Component Actor Name: %s"), *hitComponentSecond->GetOwner()->GetName());

								//search in the joinedRoomIDs array check if the room is already joined
								if (roomInfoA.joinedRoomIDs.Contains(roomInfoB.RoomID) || roomInfoB.joinedRoomIDs.Contains(roomInfoA.RoomID)) {
									//debug room id
									UE_LOG(LogTemp, Warning, TEXT("Room %d is already joined with Room %d"), roomInfoA.RoomID, roomInfoB.RoomID);

									// Replace the wall with a door for room A
									//hitComponent2->SetStaticMesh(roomInfoA.RoomInstance->DoorMeshes[FMath::RandRange(0, roomInfoA.RoomInstance->DoorMeshes.Num() - 1)]);

									// Replace the wall with a door for room B
									//hitComponent->SetStaticMesh(roomInfoB.RoomInstance->DoorMeshes[FMath::RandRange(0, roomInfoB.RoomInstance->DoorMeshes.Num() - 1)]);
								}


								if (hitComponentSecond->ComponentHasTag("WallCorner")) {
									//debug hit component tag
									UE_LOG(LogTemp, Warning, TEXT("Hit Corner Component %d Tag: %s"), roomInfoB.RoomID, *hitComponentSecond->ComponentTags[0].ToString());
								}

								// Calculate the width and height extents of the box
								float WidthExtent = roomInfoA.RoomInstance->FloorWidth; // Desired width extent (half-size)
								float HeightExtent = roomInfoA.RoomInstance->WallHeight / 2; // Desired height extent (half-size)



								// Check if the hit components is a wall meshes for both rooms and joined room is correct one that we are looking for
								if (hitComponentActorName == *roomInfoB.RoomInstance->GetName())
								{

									if (hitComponentSecond->ComponentHasTag("Wall")
										&& hitComponentFirst->ComponentHasTag("Wall"))
									{
										//draw debug line between intersection points
										//DrawDebugLine(GetWorld(), hitFirstComponentCenter, intersectionPoint2, FColor::Red, true, -1, 0, 5);

										//debug hit point
										//DrawDebugSphere(GetWorld(), intersectionPoint2, 50, 20, FColor::Red, true);

										FBoxSphereBounds BoundsSecond = hitComponentSecond->Bounds;
										FVector hitSecondComponentCenter = BoundsSecond.Origin;

										RoomPointA = hitFirstComponentCenter;
										RoomPointB = intersectionPoint2;

										hitComponentMeshRoomA = hitComponentFirst;
										hitComponentMeshRoomB = hitComponentSecond;

										hitComponentCenterMeshRoomA = hitFirstComponentCenter;
										hitComponentCenterMeshRoomB = hitSecondComponentCenter;
									}
									else {
										//Lets try to find the correct wall mesh that is near the intersection point on the side according to the direction


										//new line trace but with space of one wall length
										FHitResult OutHit3;

										//choose dirrection
										FVector CurrentPointDirection = (RoomCenterBNew - RoomCenterANew).GetSafeNormal();
										FVector hitFirstComponentCenterNew;

										//Create new vector with FVector(0, roomInfoA.RoomInstance->WallLength, 0) or FVector(roomInfoA.RoomInstance->WallLength, 0, 0) according to the direction

										//get the hit component nearest mesh
										if (FMath::Abs(CurrentPointDirection.X) > FMath::Abs(CurrentPointDirection.Y)) {
											//hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
											hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(0, roomInfoA.RoomInstance->WallLength, 0);
										}
										else {
											hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(roomInfoA.RoomInstance->WallLength, 0, 0);
											//hitFirstComponentCenterNew = hitFirstComponentCenter - FVector(0, roomInfoA.RoomInstance->WallLength, 0);
										}

										//get array of static meshes near the intersection point with some radius
										TArray<UStaticMeshComponent*> StaticMeshes = GetStaticMeshesAtPoint(hitFirstComponentCenterNew, 150.f, "Wall");

										//destroy all static meshes near the intersection
										for (UStaticMeshComponent* StaticMesh : StaticMeshes) {
											hitComponentMeshRoomA = StaticMesh;
										}

										if (StaticMeshes.Num() == 0)
										{
											RoomPointA = FVector::ZeroVector;
											RoomPointB = FVector::ZeroVector;
											continue;
										}


										FVector AlignedNewPointB = GetAlignedPoint(hitFirstComponentCenterNew, RoomCenterBNew);

										//draw debug sphere
										DrawDebugSphere(GetWorld(), hitFirstComponentCenterNew, 50, 20, FColor::Yellow, true);

										//debug out hit sphere
										DrawDebugSphere(GetWorld(), OutHit.ImpactPoint, 50, 20, FColor::Green, true);


										//line trace from hit component center to the room center
										bool bHit3 = GetWorld()->LineTraceSingleByChannel(OutHit3, hitFirstComponentCenterNew, AlignedNewPointB, ECC_Visibility, CollisionParams);


										//get the hit mesh component
										UStaticMeshComponent* hitComponentThird = Cast<UStaticMeshComponent>(OutHit3.Component.Get());

										if (!hitComponentThird) {

											continue;
										}

										if (!hitComponentThird->ComponentHasTag("Wall")) {
											DrawDebugSphere(GetWorld(), OutHit3.ImpactPoint, 50, 20, FColor::Red, true);
											continue;
										}

										if (!CheckRoomCorridorJoinIsValid(RoomCenterANew, RoomCenterBNew, hitFirstComponentCenterNew, roomInfoA.RoomInstance, roomInfoB.RoomInstance, CollisionParams)) {
											//debug wrong room corridor join
											UE_LOG(LogTemp, Warning, TEXT("Room Corridor Join is not valid"));
											//debug hit point
											DrawDebugSphere(GetWorld(), OutHit3.ImpactPoint, 50, 20, FColor::Black, true);
											
											continue;
										}


										//debug hit sphere
										DrawDebugSphere(GetWorld(), OutHit3.ImpactPoint, 50, 20, FColor::Cyan, true);

										RoomPointA = hitFirstComponentCenterNew;
										RoomPointB = OutHit3.ImpactPoint;

										hitComponentMeshRoomB = hitComponentThird;

										hitComponentCenterMeshRoomA = hitFirstComponentCenterNew;
										hitComponentCenterMeshRoomB = OutHit3.ImpactPoint;

									}



									if (RoomPointA != FVector::ZeroVector && RoomPointB != FVector::ZeroVector) {

										// Create a connection corridor between the two rooms
										CreateConnectionCorridor(RoomPointA, RoomPointB, WidthExtent, HeightExtent);

										// Replace the wall with a door for room A
										hitComponentMeshRoomA->SetStaticMesh(roomInfoA.RoomInstance->DoorMeshes[FMath::RandRange(0, roomInfoA.RoomInstance->DoorMeshes.Num() - 1)]);
										hitComponentMeshRoomA->SetCollisionProfileName("IgnoreCameraAndCursor");

										// Replace the wall with a door for room B
										hitComponentMeshRoomB->SetStaticMesh(roomInfoB.RoomInstance->DoorMeshes[FMath::RandRange(0, roomInfoB.RoomInstance->DoorMeshes.Num() - 1)]);
										hitComponentMeshRoomB->SetCollisionProfileName("IgnoreCameraAndCursor");

										FBoxSphereBounds BoundsSecond = hitComponentMeshRoomB->Bounds;
										FVector hitSecondComponentCenter = BoundsSecond.Origin;

										//get array of actors near the intersection point A
										TArray<AActor*> Actors = GetActorsAtPoint(hitComponentCenterMeshRoomA, 300.f, "Light");

										//destroy all actors near the intersection point A
										for (AActor* Actor : Actors) {
											Actor->Destroy();

											//debug destroy actor
											UE_LOG(LogTemp, Warning, TEXT("Actor First Room Destroyed"));
										}

										//get array of static meshes near the intersection point A
										TArray<UStaticMeshComponent*> StaticMeshes = GetStaticMeshesAtPoint(hitComponentCenterMeshRoomA, 300.f, "Decoration");

										//destroy all static meshes near the intersection point A
										for (UStaticMeshComponent* StaticMesh : StaticMeshes) {
											StaticMesh->DestroyComponent();

											//debug destroy static mesh
											UE_LOG(LogTemp, Warning, TEXT("Static Mesh First Room Destroyed"));
										}


										//get array of actors near the intersection point B
										TArray<AActor*> ActorsSecond = GetActorsAtPoint(hitComponentCenterMeshRoomB, 300.f, "Light");

										//destroy all actors near the intersection point B
										for (AActor* Actor : ActorsSecond) {
											Actor->Destroy();

											//debug destroy actor
											UE_LOG(LogTemp, Warning, TEXT("Actor Second Room Destroyed"));
										}

										//get array of static meshes near the intersection point B
										TArray<UStaticMeshComponent*> StaticMeshesSecond = GetStaticMeshesAtPoint(hitComponentCenterMeshRoomB, 300.f, "Decoration");

										//destroy all static meshes near the intersection point
										for (UStaticMeshComponent* StaticMesh : StaticMeshesSecond) {
											StaticMesh->DestroyComponent();

											//debug destroy static mesh
											UE_LOG(LogTemp, Warning, TEXT("Static Mesh Second Room Destroyed"));
										}




										//debug room id
										UE_LOG(LogTemp, Warning, TEXT("Room %d is joined with Room %d"), roomInfoA.RoomID, roomInfoB.RoomID);

										// Add the room to the joined rooms list
										roomInfoA.isJoined = true;
										roomInfoB.isJoined = true;
										roomInfoA.joinedRoomIDs.Add(roomInfoB.RoomID);
										roomInfoB.joinedRoomIDs.Add(roomInfoA.RoomID);
									}

								}






							}
						}
					}
				}
			}
		}

		if (closestRoom) {

		}
	}
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
			DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, AlignedPointLeft, 120.0f, FColor::Blue, true, -1, 0, 5);
			DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, AlignedPointRight, 120.0f, FColor::Blue, true, -1, 0, 5);


			//debug left and right hit point
			DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Blue, true);
			DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Blue, true);


			UE_LOG(LogTemp, Warning, TEXT("Hit Component Left Actor Name: %s"), *RoomInstanceB->GetName());
			return false;
		}

		if (hitComponentRight && hitComponentRight->GetOwner()->GetFName() != RoomInstanceB->GetName())
		{
			//debug arrow for hit point left and right
			DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterLeft, AlignedPointLeft, 120.0f, FColor::Blue, true, -1, 0, 5);
			DrawDebugDirectionalArrow(GetWorld(), hitFirstComponentCenterRight, AlignedPointRight, 120.0f, FColor::Blue, true, -1, 0, 5);


			//debug left and right hit point
			DrawDebugSphere(GetWorld(), OutHitLeft.ImpactPoint, 50, 20, FColor::Blue, true);
			DrawDebugSphere(GetWorld(), OutHitRight.ImpactPoint, 50, 20, FColor::Blue, true);


			UE_LOG(LogTemp, Warning, TEXT("Hit Component Right Actor Name: %s"), *RoomInstanceB->GetName());
			return false;
		}
	}

	return true;
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
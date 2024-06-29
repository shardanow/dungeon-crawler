// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RoomInfo.h"
#include "Boundary.h"
#include "FloorGenerator.generated.h"

UCLASS()
class DUNGEONCRAWLER_API AFloorGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GenerateFloor();

	void InitializeGrid(int gridWidth, int gridHeight, float cellSize);

	FVector CalculateRoomPosition(ARoomGenerator* roomInstance, TArray<FRoomInfo>& existingRooms);

	bool IsPositionValid(const FVector& position, float roomLength, float roomWidth, const TArray<FRoomInfo>& existingRooms);

	bool RoomsOverlap(const FVector& newPos, float newLength, float newWidth, const FVector& existingPos, float existingLength, float existingWidth);

	ARoomGenerator* GenerateRoomAtPosition(ARoomGenerator* roomInstance, FVector position);

	//ARoomGenerator* roomGenerator;

	//ARoomGenerator* corridorGenerator;

	//blueprint callable function
	UFUNCTION(BlueprintCallable, Category = "Floor Generation")
	void RegenerateFloor();

	void FindAndConnectNearestRoom();

	bool CheckRoomCorridorJoinIsValid(FVector RoomCenterAPoint, FVector RoomCenterBPoint, FVector HitPointCenter, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FCollisionQueryParams CollisionParams);

	void DrawDebugBoxWithFixedSize(FVector Point1, FVector Point2, float WidthExtent, float HeightExtent);

	void CreateConnectionCorridor(FVector Point1, FVector Point2, float WidthExtent, float HeightExtent);

	FVector GetAlignedPoint(const FVector& Origin, const FVector& Target);

	FVector DeterminePrimaryAxis(const FVector& Vector);

	TArray<UStaticMeshComponent*> GetStaticMeshesAtPoint(FVector Point, float Radius, FName TagName);

	TArray<AActor*> GetActorsAtPoint(FVector Point, float Radius, FName TagName);

	TArray<FBoundary> CalculateBoundaries(ARoomGenerator* room);

	//blueprint variables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Size")
	int numberOfRooms;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Size")
	int numberOfRoomsMin = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Size")
	int numberOfRoomsMax = 8;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms Data")
	TArray<FRoomInfo> RoomsData;

	TMap<FIntPoint, FRoomInfo> RoomGrid;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rooms Data")
	TArray<FRoomInfo> RoomsList;


	//room blueprint reference
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Blueprint")
	TSubclassOf<class ARoomGenerator> RoomBlueprint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Join")
	float RoomJoinDistance;


private:
	// Grid to track occupied cells
	TArray<TArray<bool>> Grid;
	int GridWidth;
	int GridHeight;
	float CellSize;
};

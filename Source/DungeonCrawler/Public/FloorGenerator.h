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

	bool isGenerating = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void GenerateFloor();

	void InitializeGrid(int gridWidth, int gridHeight, float cellSize);

	FVector CalculateRoomPosition(ARoomGenerator* roomInstance, TArray<FRoomInfo>& existingRooms);

	ARoomGenerator* GenerateRoomAtPosition(ARoomGenerator* roomInstance, FVector position);

	//blueprint callable function
	UFUNCTION(BlueprintCallable, Category = "Floor Generation")
	void RegenerateFloor();

	void FindAndConnectNearestRoom();

	bool AreAllRoomsInterconnected();

	void DFS(int roomIndex, TArray<bool>& visited);

	int FindRoomIndexByID(int roomID);

	void FinalizeLevel();

	bool CheckRoomCorridorJoinIsValid(FVector RoomCenterAPoint, FVector RoomCenterBPoint, FVector HitPointCenter, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FCollisionQueryParams CollisionParams);

	void MovePointAndRetryRoomJoin(FVector RoomCenterAPoint, FVector RoomCenterBPoint, FVector HitPointCenter, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FCollisionQueryParams CollisionParams);

	void JoinRoomByCorridor(FVector RoomPointA, FVector RoomPointB, ARoomGenerator* RoomInstanceA, ARoomGenerator* RoomInstanceB, FVector hitComponentCenterMeshRoomA, FVector hitComponentCenterMeshRoomB, UStaticMeshComponent* hitComponentMeshRoomA, UStaticMeshComponent* hitComponentMeshRoomB);

	void DestroyLevelWithLostRoomJoins();

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

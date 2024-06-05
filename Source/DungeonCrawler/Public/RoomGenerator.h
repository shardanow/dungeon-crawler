// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/BrushComponent.h"

#include "RoomGenerator.generated.h"

UCLASS()
class DUNGEONCRAWLER_API ARoomGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARoomGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

private:
    TArray<FVector> DecoratedWallPositions;

public:
    // Variables for room dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MinRoomLength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MaxRoomLength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MinRoomWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MaxRoomWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallLength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallThickness;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallHeight;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorLength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorThickness;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorWidth;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorHeight;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorThickness;

    // Arrays for meshes and Blueprints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<UStaticMesh*> WallMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<UStaticMesh*> FloorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<UStaticMesh*> DoorMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> TreasureBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> EnemyBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> WallTorchesBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<UStaticMesh*> WallDecorationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<UStaticMesh*> FloorPillarsMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> FloorDecorationBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float TreasureSpawnChance; // Percentage chance to spawn a treasure

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float EnemySpawnChance; // Percentage chance to spawn an enemy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float WallTorchesSpawnChance; // Percentage chance to spawn wall torches

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float WallDecorationSpawnChance; // Percentage chance to spawn wall decoration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float FloorPillarsSpawnChance; // Percentage chance to spawn floor pillars

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float FloorDecorationSpawnChance; // Percentage chance to spawn floor decoration

    // Function to generate the room
    UFUNCTION(BlueprintCallable, Category = "Room Generation")
    void GenerateRoom();

    UStaticMeshComponent* SpawnMesh(UStaticMesh* Mesh, FVector Location, FRotator Rotation, FName SocketName);
    bool SpawnActorOnSocket(TSubclassOf<AActor> ActorClass, UStaticMeshComponent* Mesh, float SpawnChance, FName SocketName);

    bool SpawnMeshOnExistingMeshFloorSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName);
    bool SpawnMeshOnExistingMeshWallSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName);

    bool IsFloorCellNearWall(int32 NumWallsLength, int32 NumWallsWidth, FVector FloorPosition, float horizontalProximity);

    void AddPlayerStart(FVector Location);

};

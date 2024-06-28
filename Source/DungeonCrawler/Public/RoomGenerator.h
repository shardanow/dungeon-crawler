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

    UFUNCTION(BlueprintCallable, Category = "Room Generation")
    void RandomRoomSizeGenerate();

    UFUNCTION(BlueprintCallable, Category = "Room Generation")
    void FixedRoomSizeGenerate(float NewRoomLength, float NewRoomWidth);

    UFUNCTION(BlueprintCallable, Category = "Room Generation")
    void GenerateRoom(FVector RoomPosition);

    UFUNCTION(BlueprintCallable, Category = "Room Generation")
    void GenerateCorridor(FVector RoomPosition, FName DisableWallSide);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* Root;

private:
    TArray<FVector> DecoratedWallPositions;

    TArray<FVector> SpawnedFloorLightSourcesPositions;
    TArray<FVector> SpawnedWallLightSourcesPositions;

public:
    // Variables for room dimensions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MinRoomLength = 2040;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MaxRoomLength = 5080;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MinRoomWidth = 2040;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float MaxRoomWidth = 5080;

    //current room size
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Dimensions")
    float RoomLength;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Room Dimensions")
    float RoomWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    int32 NumWallsLength;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    int32 NumWallsWidth;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallLength = 500;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallThickness = 45;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float WallHeight = 500;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorWidth = 495;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorLength = 495;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float FloorThickness = 30;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorWidth = 510;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorHeight = 500;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Dimensions")
    float DoorThickness = 45;

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
    TArray<UStaticMesh*> FloorDecorationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> FloorDecorationBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Assets")
    TArray<TSubclassOf<AActor>> FloorLightSourcesBlueprints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float TreasureSpawnChance = 5; // Percentage chance to spawn a treasure

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float EnemySpawnChance = 10; // Percentage chance to spawn an enemy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float WallTorchesSpawnChance = 10; // Percentage chance to spawn wall torches

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float WallDecorationSpawnChance = 10; // Percentage chance to spawn wall decoration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float FloorPillarsSpawnChance = 5; // Percentage chance to spawn floor pillars

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float FloorDecorationSpawnChance = 15; // Percentage chance to spawn floor decoration

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Chances")
    float FloorLightSourcesSpawnChance = 5; // Percentage chance to spawn floor light sources

    //jon room properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Room Status")
    bool bIsJoinedToAnotherRoom =  false;

    // Function to generate the room
   // UFUNCTION(BlueprintCallable, Category = "Room Generation")
   // void GenerateRoom();

    UStaticMeshComponent* SpawnMesh(UStaticMesh* Mesh, FVector Location, FRotator Rotation, FName SocketName, FName TagName = "SomeMesh");
    bool SpawnActorOnSocket(TSubclassOf<AActor> ActorClass, UStaticMeshComponent* Mesh, float SpawnChance, FName SocketName, FName CollisionPresetName = "BlockAll", FName TagName = "SomeActor");

    UStaticMeshComponent* SpawnMeshOnExistingMeshFloorSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName, FName CollisionPresetName = "BlockAll", FName TagName = "SomeMesh");
    bool SpawnMeshOnExistingMeshWallSocket(UStaticMesh* NewMesh, UStaticMeshComponent* ExistingMeshComponent, float SpawnChance, FName SocketName, FName TagName = "SomeMesh");

    bool IsFloorCellNearWall(int32 NumWallsLength, int32 NumWallsWidth, FVector FloorPosition, float horizontalProximity);

    void AddPlayerStart(FVector Location);

    bool IsTooCloseToOtherLightSources(TArray<FVector> LightSourcesArray, FVector NewPosition, float MinDistance);


};

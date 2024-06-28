// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RoomGenerator.h"
#include "Boundary.h"
#include "RoomInfo.generated.h"

USTRUCT(BlueprintType)
struct FRoomInfo
{
    GENERATED_BODY()

public:
    // Unique identifier for the room
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    int32 RoomID;

    // Pointer to the room generator instance
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    ARoomGenerator* RoomInstance;

    // Position of the room in the world
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector RoomPosition;

    //Boundaries of the room
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<FBoundary> RoomBoundaries;

    //Room isJoined
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool isJoined;

    //joined room id array
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<int32> joinedRoomIDs;

    // Default constructor
    FRoomInfo() : RoomID(-1), RoomInstance(nullptr), RoomPosition(FVector::ZeroVector) {}


    FRoomInfo(int32 InRoomID, ARoomGenerator* InRoomInstance, const FVector& InRoomPosition, const TArray<FBoundary>& InRoomBoundaries)
        : RoomID(InRoomID), RoomInstance(InRoomInstance), RoomPosition(InRoomPosition), RoomBoundaries(InRoomBoundaries) {}
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Boundary.generated.h"

USTRUCT(BlueprintType)
struct FBoundary {
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector Start; // Start point of the boundary line segment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    FVector End;   // End point of the boundary line segment

    //default constructor
    FBoundary() : Start(FVector::ZeroVector), End(FVector::ZeroVector) {}

    // Constructor for easy initialization
    FBoundary(const FVector& InStart, const FVector& InEnd)
        : Start(InStart), End(InEnd) {}
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class DUNGEONCRAWLER_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacter();

	// is the enemy dead
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Enemy")
	bool bIsDeadState;

	// Getter for bIsDead
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	bool IsDeadState() const { return bIsDeadState; }

	// Setter for bIsDead
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetIsDeadState(bool IsDead) { bIsDeadState = IsDead; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};

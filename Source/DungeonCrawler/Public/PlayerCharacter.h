// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "PlayerCharacter.generated.h"

UCLASS()
class DUNGEONCRAWLER_API APlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	// Function to handle taking damage
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);

	// Bool to change when damage is taken
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	bool bIsDamaged;

	// Getter for bIsDamaged
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Damage")
	bool IsDamaged() const { return bIsDamaged; }

	// Setter for bIsDamaged
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Damage")
	void SetIsDamaged(bool IsDamaged) { bIsDamaged = IsDamaged; }

	// Function to check if any animation montage is playing
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Animation")
	bool IsPlayingAnimationMontage() const;

	// Function to get the current animation montage name
	// ufunction is used to make the function available to Blueprints
	UFUNCTION(BlueprintCallable, Category = "Animation")
	FString GetCurrentMontageName() const;

	// Function to get animation notify name
	UFUNCTION()
	void OnAnimationNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
};

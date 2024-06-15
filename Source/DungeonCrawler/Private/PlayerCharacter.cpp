// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "Engine/Engine.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include <CustomPlayerController.h>

// Sets default values
APlayerCharacter::APlayerCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bIsDamaged = false;
}

// Called when the game starts or when spawned
void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Bind the OnTakeAnyDamage event to the OnTakeAnyDamage function
	this->OnTakeAnyDamage.AddDynamic(this, &APlayerCharacter::HandleTakeAnyDamage);

    // Bind to the animation notify event
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        // Bind to the OnPlayMontageNotifyBegin event
        AnimInstance->OnPlayMontageNotifyBegin.AddUniqueDynamic(this, &APlayerCharacter::OnAnimationNotify);
    }
}

// Called every frame
void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Function to handle taking damage
void APlayerCharacter::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
    if (Damage > 0.0f)
    {
        bIsDamaged = true;
        // Optionally, print a debug message to confirm with amount of damage taken

        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Player took %f damage"), Damage));
        }
    }
}

// Function to check if any animation montage is playing
bool APlayerCharacter::IsPlayingAnimationMontage() const
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        return AnimInstance->Montage_IsPlaying(nullptr); // nullptr checks if any montage is playing
    }
    return false;
}

// Function to get the name of the currently playing animation montage
FString APlayerCharacter::GetCurrentMontageName() const
{
    if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
    {
        if (UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
        {
            return CurrentMontage->GetName();
        }
    }
    return FString("No Montage Playing");
}

// Function to handle the attack notify
void APlayerCharacter::OnAnimationNotify(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
    ACustomPlayerController* PlayerController = Cast<ACustomPlayerController>(GetController());


    if (PlayerController)
    {
        PlayerController->HandleAnimationNotify(NotifyName);
    }
}
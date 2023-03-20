// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"

// Sets default values
AFPSAICharacter::AFPSAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensingComp"));
	// Won't attach to root as pawn sensing component is not a scene component

	PawnSensingComp->OnSeePawn.AddDynamic(this, &AFPSAICharacter::OnSeenPawn);
	PawnSensingComp->OnHearNoise.AddDynamic(this, &AFPSAICharacter::OnNoiseHeard);

	GuardState = EAIState::Idle;
}

// Called when the game starts or when spawned
void AFPSAICharacter::BeginPlay()
{
	Super::BeginPlay();
	
	OriginalRotation = GetActorRotation();
}

// Called every frame
void AFPSAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPSAICharacter::OnSeenPawn(APawn* SeenPawn)
{
	if (SeenPawn == nullptr)
	{
		return;
	}

	DrawDebugSphere(GetWorld(), SeenPawn->GetActorLocation(), 32.0f, 8, FColor::Yellow, false, 10.0f);

	ChangeGuardState(EAIState::Alerted);

	AFPSGameMode* GM = Cast<AFPSGameMode>(GetWorld()->GetAuthGameMode());
	if (GM)
	{
		GM->CompleteMission(SeenPawn, false);
	}
}

void AFPSAICharacter::OnNoiseHeard(APawn* NoiseInstigator, const FVector& Location, float Volume)
{
	// If the guard can aldready see player, you can't distract him with sound
	// Alerted state has higher priority over any other state
	if (GuardState == EAIState::Alerted) { return; }
	DrawDebugSphere(GetWorld(), Location, 32.0f, 8, FColor::Green, false, 10.0f);
	
	FVector LookAtDirection = Location - GetActorLocation();
	LookAtDirection.Normalize();

	FRotator LookAtRotation = FRotationMatrix::MakeFromX(LookAtDirection).Rotator();
	LookAtRotation.Pitch = 0.0f;
	LookAtRotation.Roll = 0.0f;

	SetActorRotation(LookAtRotation);

	GetWorldTimerManager().ClearTimer(TimerHandle_ResetOrientation);
	GetWorldTimerManager().SetTimer(TimerHandle_ResetOrientation, this, &AFPSAICharacter::ResetOrientation, 3.0f);

	ChangeGuardState(EAIState::Suspicious);
}

void AFPSAICharacter::ResetOrientation()
{
	// If gaurd can see player at new rotation do not reset rotation
	if (GuardState == EAIState::Alerted) { return; }
	SetActorRotation(OriginalRotation);
	ChangeGuardState(EAIState::Idle);
}

void AFPSAICharacter::ChangeGuardState(EAIState NewState)
{
	if (GuardState == NewState) { return; }
	GuardState = NewState;
	OnGuardStateChanged(NewState);
}
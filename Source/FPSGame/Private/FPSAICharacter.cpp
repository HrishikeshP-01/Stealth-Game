// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAICharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "DrawDebugHelpers.h"
#include "FPSGameMode.h"
/*This allows us to use the GetLifetimeReplicatedProps fn*/
#include "Net/UnrealNetwork.h"

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

	// OnGuardStateChanged(NewState);

	OnRep_GuardState();
	/* As said earlier, the OnRep_GuardState() is just a fn. We can use it anywhere.
	* This function is run automatically on clients when GuardState is updated due to 
	* the ReplicatedUsing=OnRep_GuardState() property we assigned to GuardState in header.
	* We want the OnGuardStateChanged() event to run on server as well. Even though it's only a UI change 
	* the server is Player1 in this example & there is no dedicated server so we want the event to be triggered
	* Therefore, we run the OnRep_GuardState() fn here & this will only run on server as it is inside an AI-based fn */
}

/* As the OnGuardStateChanged() event is what triggers the change in UI we call it in the OnRep_GuardState() fn.
* We only want the change in UI to be seen on the clients as every major aspect such as orientation of the AI character is aldready replicated
* Things like perception arent run on the client as it's enough to run it on the server.*/
void AFPSAICharacter::OnRep_GuardState()
{
	OnGuardStateChanged(GuardState);
}

/*We always need this function whenever we need to add a new replicated property. 
* It's not relevant to replicated functions etc. but anytime a new replicated variable is added, 
* it must be setup inside of this function to tell Unreal his replication "rules"
* These rules for example can be used to determine things like is the variable replicated in all clients or owning client or a particular client etc.*/
void AFPSAICharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Tells that the variable is replicated for all AFPSAICharacters in all clients
	DOREPLIFETIME(AFPSAICharacter, GuardState);
}
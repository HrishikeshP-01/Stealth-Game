// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "FPSGameMode.h"
#include "FPSHUD.h"
#include "FPSCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"

AFPSGameMode::AFPSGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/Blueprints/BP_Player"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AFPSHUD::StaticClass();
}

void AFPSGameMode::CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess)
{
	if (InstigatorPawn) // Check if pawn is valid
	{
		InstigatorPawn->DisableInput(nullptr);

		if (SpectatingViewpointClass)
		{
			TArray<AActor*> ReturnedActors;
			UGameplayStatics::GetAllActorsOfClass(this, SpectatingViewpointClass, ReturnedActors);
			AActor* NewViewTarget = ReturnedActors[0];

			APlayerController* PC = Cast<APlayerController>(InstigatorPawn->GetController());
			if (PC)
			{
				PC->SetViewTargetWithBlend(NewViewTarget, 0.5f, VTBlend_Cubic);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpectatingViewportClass empty. Assign class in GameMode BP"));
		}

		OnMissionCompleted(InstigatorPawn, bMissionSuccess);
	}
}
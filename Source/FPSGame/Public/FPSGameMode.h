// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FPSGameMode.generated.h"

UCLASS()
class AFPSGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	AFPSGameMode();

	// We want to call this from the extraction zone so we make it public
	void CompleteMission(APawn* InstigatorPawn, bool bMissionSuccess);

	// I want to implement some complete mission functionality via BP
	UFUNCTION(BlueprintImplementableEvent)
		void OnMissionCompleted(APawn* InstigatorPawn, bool bMissionSuccess);

protected:
	// Spectating viewpoint class is a BP class so we need some way of refering to it inside C++ code so we create TSubclassOf variable & will later set it to the BP class name
	UPROPERTY(EditDefaultsOnly, Category = "Spectating")
		TSubclassOf<AActor> SpectatingViewpointClass;
};




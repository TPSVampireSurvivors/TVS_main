// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TVS_mainGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class ATVS_mainGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	ATVS_mainGameMode();
};




// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZodiacReplicationGraphSettings.h"
#include "Misc/App.h"
#include "System/ZodiacReplicationGraph.h"

UZodiacReplicationGraphSettings::UZodiacReplicationGraphSettings()
{
	CategoryName = TEXT("Game");
	DefaultReplicationGraphClass = UZodiacReplicationGraph::StaticClass();
}
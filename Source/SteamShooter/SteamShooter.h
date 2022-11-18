// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

//new  custom Object type for skeletal meshes defined inside of editor Edit->ProjectSettings->ObjectChannels->NewObjectChannel
#define ECC_SkeletalMesh ECollisionChannel::ECC_GameTraceChannel1
//same but HitBox Channel
#define ECC_HitBox ECollisionChannel::ECC_GameTraceChannel2
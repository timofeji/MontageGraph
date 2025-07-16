// Copyright Drop Games Inc. 

#include "AnimNotifyState_MontageCollisionTrace.h"


FString UAnimNotifyState_MontageCollisionTrace::GetNotifyName_Implementation() const
{
	static const FString Name(TEXT("Collision Trace"));
	return Name;
}

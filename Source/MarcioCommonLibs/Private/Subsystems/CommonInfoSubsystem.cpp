#include "Subsystems/CommonInfoSubsystem.h"

#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableStorage.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Util/MarcioCommonLibsConfiguration.h"
#include "Util/MCLOptimize.h"

#ifndef OPTIMIZE
#pragma optimize("", off)
#endif

ACommonInfoSubsystem* ACommonInfoSubsystem::instance = nullptr;

FCriticalSection ACommonInfoSubsystem::mclCritical;

ACommonInfoSubsystem::ACommonInfoSubsystem()
{
	ReplicationPolicy = ESubsystemReplicationPolicy::SpawnLocal;
}

void ACommonInfoSubsystem::BeginPlay()
{
	Super::BeginPlay();

	instance = this;
}

void ACommonInfoSubsystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	noneItemDescriptors.Empty();
	wildCardItemDescriptors.Empty();
	anyUndefinedItemDescriptors.Empty();
	overflowItemDescriptors.Empty();
	nuclearWasteItemDescriptors.Empty();

	baseStorageTeleporterClass = nullptr;
	baseUndergroundSplitterInputClass = nullptr;
	baseUndergroundSplitterOutputClass = nullptr;
	baseModularLoadBalancerClass = nullptr;
	baseBuildableFactorySimpleProducerClass = nullptr;
	baseCounterLimiterClass = nullptr;

	storageContainerClasses.Empty();

	powerPoleClasses.Empty();
	powerPoleWallClasses.Empty();
	powerPoleWallDoubleClasses.Empty();
	powerTowerClasses.Empty();

	allTeleporters.Empty();
	allUndergroundInputBelts.Empty();

	initialized = false;

	instance = nullptr;
}

ACommonInfoSubsystem* ACommonInfoSubsystem::Get(UWorld* world)
{
	USubsystemActorManager* SubsystemActorManager = world->GetSubsystem<USubsystemActorManager>();

	return SubsystemActorManager->GetSubsystemActor<ACommonInfoSubsystem>();
}

ACommonInfoSubsystem* ACommonInfoSubsystem::Get(UObject* WorldContextObject)
{
	return Get(GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull));
}

void ACommonInfoSubsystem::Initialize
(
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_noneItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_wildcardItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_anyUndefinedItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_overflowItemDescriptors,
	const TSet<TSubclassOf<UFGItemDescriptor>>& in_nuclearWasteItemDescriptors
)
{
	instance = this;

	noneItemDescriptors = in_noneItemDescriptors;
	wildCardItemDescriptors = in_wildcardItemDescriptors;
	anyUndefinedItemDescriptors = in_anyUndefinedItemDescriptors;
	overflowItemDescriptors = in_overflowItemDescriptors;
	nuclearWasteItemDescriptors = in_nuclearWasteItemDescriptors;

	baseStorageTeleporterClass = UClass::TryFindTypeSlow<UClass>(TEXT("/StorageTeleporter/Buildables/ItemTeleporter/ItemTeleporter_Build.ItemTeleporter_Build_C"));
	baseUndergroundSplitterInputClass = UClass::TryFindTypeSlow<UClass>(TEXT("/UndergroundBelts/Build/Build_UndergroundSplitterInput.Build_UndergroundSplitterInput_C"));
	baseUndergroundSplitterOutputClass = UClass::TryFindTypeSlow<UClass>(TEXT("/UndergroundBelts/Build/Build_UndergroundSplitterOutput.Build_UndergroundSplitterOutput_C"));
	baseModularLoadBalancerClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/LoadBalancers.LBBuild_ModularLoadBalancer"));
	baseBuildableFactorySimpleProducerClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/FactoryGame.FGBuildableFactorySimpleProducer"));
	baseCounterLimiterClass = UClass::TryFindTypeSlow<UClass>(TEXT("/Script/CounterLimiter.CL_CounterLimiter"));

	AddClass(storageContainerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk1/Build_StorageContainerMk1.Build_StorageContainerMk1_C"));
	AddClass(storageContainerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/StorageContainerMk2/Build_StorageContainerMk2.Build_StorageContainerMk2_C"));

	AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk1/Build_PowerPoleMk1.Build_PowerPoleMk1_C"));
	AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk2/Build_PowerPoleMk2.Build_PowerPoleMk2_C"));
	AddClass(powerPoleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleMk3/Build_PowerPoleMk3.Build_PowerPoleMk3_C"));

	AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall.Build_PowerPoleWall_C"));
	AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall_Mk2.Build_PowerPoleWall_Mk2_C"));
	AddClass(powerPoleWallClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWall/Build_PowerPoleWall_Mk3.Build_PowerPoleWall_Mk3_C"));

	AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble.Build_PowerPoleWallDouble_C"));
	AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble_Mk2.Build_PowerPoleWallDouble_Mk2_C"));
	AddClass(powerPoleWallDoubleClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerPoleWallDouble/Build_PowerPoleWallDouble_Mk3.Build_PowerPoleWallDouble_Mk3_C"));

	AddClass(powerTowerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerTower/Build_PowerTower.Build_PowerTower_C"));
	AddClass(powerTowerClasses, TEXT("/Game/FactoryGame/Buildable/Factory/PowerTower/Build_PowerTowerPlatform.Build_PowerTowerPlatform_C"));

	// removeTeleporterDelegate.BindDynamic(this, &ACommonInfoSubsystem::removeTeleporter);

	auto buildableSubsystem = AFGBuildableSubsystem::Get(this);

	if (buildableSubsystem)
	{
		buildableSubsystem->BuildableConstructedGlobalDelegate.AddDynamic(this, &ACommonInfoSubsystem::handleBuildableConstructed);

		FScopeLock ScopeLock(&mclCritical);

		TArray<AActor*> allBuildables;
		UGameplayStatics::GetAllActorsOfClass(buildableSubsystem->GetWorld(), AFGBuildable::StaticClass(), allBuildables);

		for (auto buildableActor : allBuildables)
		{
			IsValidBuildable(Cast<AFGBuildable>(buildableActor));
		}
	}

	initialized = true;
}

void ACommonInfoSubsystem::AddClass(TSet<UClass*>& classes, const FString& classPath)
{
	if (auto foundClass = UClass::TryFindTypeSlow<UClass>(classPath))
	{
		classes.Add(foundClass);
	}
}

bool ACommonInfoSubsystem::IsStorageTeleporter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseStorageTeleporterClass && cls->IsChildOf(baseStorageTeleporterClass);
}

bool ACommonInfoSubsystem::IsPowerPole(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerPoleWall(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleWallClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerPoleWallDouble(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerPoleWallDoubleClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsPowerTower(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && powerTowerClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsStorageContainer(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && storageContainerClasses.Contains(cls);
}

bool ACommonInfoSubsystem::IsModularLoadBalancer(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseModularLoadBalancerClass && cls->IsChildOf(baseModularLoadBalancerClass);
}

bool ACommonInfoSubsystem::IsCounterLimiter(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseCounterLimiterClass && cls->IsChildOf(baseCounterLimiterClass);
}

bool ACommonInfoSubsystem::IsUndergroundSplitter(AActor* actor, TSubclassOf<AActor> cls)
{
	return IsUndergroundSplitterInput(actor, cls) || IsUndergroundSplitterOutput(actor, cls);
}

bool ACommonInfoSubsystem::IsUndergroundSplitterInput(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseUndergroundSplitterInputClass && cls->IsChildOf(baseUndergroundSplitterInputClass);
}

bool ACommonInfoSubsystem::IsUndergroundSplitterOutput(AActor* actor, TSubclassOf<AActor> cls)
{
	if (actor)
	{
		cls = actor->GetClass();
	}

	return cls && baseUndergroundSplitterOutputClass && cls->IsChildOf(baseUndergroundSplitterOutputClass);
}

void ACommonInfoSubsystem::handleBuildableConstructed(AFGBuildable* buildable)
{
	IsValidBuildable(buildable);
}

bool ACommonInfoSubsystem::IsValidBuildable(AFGBuildable* newBuildable)
{
	if (!newBuildable)
	{
		return false;
	}

	if (IsUndergroundSplitterInput(newBuildable))
	{
		if (auto underGroundBelt = Cast<AFGBuildableStorage>(newBuildable))
		{
			addUndergroundInputBelt(underGroundBelt);

			return true;
		}
	}
	else if (IsStorageTeleporter(newBuildable))
	{
		if (auto storageTeleporter = Cast<AFGBuildableFactory>(newBuildable))
		{
			addTeleporter(storageTeleporter);
		}

		return true;
	}

	return false;
}

void ACommonInfoSubsystem::addTeleporter(AFGBuildableFactory* teleporter)
{
	FScopeLock ScopeLock(&mclCritical);
	allTeleporters.Add(teleporter);

	teleporter->OnEndPlay.AddDynamic(this, &ACommonInfoSubsystem::removeTeleporter);
}

void ACommonInfoSubsystem::removeTeleporter(AActor* teleporter, EEndPlayReason::Type reason)
{
	FScopeLock ScopeLock(&mclCritical);
	allTeleporters.Remove(Cast<AFGBuildableFactory>(teleporter));

	teleporter->OnEndPlay.RemoveDynamic(this, &ACommonInfoSubsystem::removeTeleporter);
}

void ACommonInfoSubsystem::addUndergroundInputBelt(AFGBuildableStorage* undergroundInputBelt)
{
	FScopeLock ScopeLock(&ACommonInfoSubsystem::mclCritical);
	allUndergroundInputBelts.Add(undergroundInputBelt);

	undergroundInputBelt->OnEndPlay.AddDynamic(this, &ACommonInfoSubsystem::removeUndergroundInputBelt);
}

void ACommonInfoSubsystem::removeUndergroundInputBelt(AActor* undergroundInputBelt, EEndPlayReason::Type reason)
{
	FScopeLock ScopeLock(&ACommonInfoSubsystem::mclCritical);
	allUndergroundInputBelts.Remove(Cast<AFGBuildableStorage>(undergroundInputBelt));

	undergroundInputBelt->OnEndPlay.RemoveDynamic(this, &ACommonInfoSubsystem::removeUndergroundInputBelt);
}

#ifndef OPTIMIZE
#pragma optimize("", on)
#endif

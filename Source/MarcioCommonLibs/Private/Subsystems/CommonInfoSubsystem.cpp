#include "Subsystems/CommonInfoSubsystem.h"

#include "Buildables/FGBuildableFactory.h"
#include "Buildables/FGBuildableStorage.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Logic/EfficiencyCheckerLogic.h"
#include "Subsystem/SubsystemActorManager.h"
#include "Util/MarcioCommonLibsConfiguration.h"
#include "Util/MCLOptimize.h"

#ifndef OPTIMIZE
#pragma optimize("", off)
#endif

ACommonInfoSubsystem* ACommonInfoSubsystem::instance = nullptr;

FCriticalSection ACommonInfoSubsystem::mclCritical;

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

	allTeleporters.Empty();
	allUndergroundInputBelts.Empty();

	initialized = false;

	instance = nullptr;
}

ACommonInfoSubsystem* ACommonInfoSubsystem::Get(/*UWorld* world*/)
{
	return instance;
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

	auto commonInfoSubsystem = ACommonInfoSubsystem::Get();

	if (commonInfoSubsystem->baseUndergroundSplitterInputClass && newBuildable->IsA(commonInfoSubsystem->baseUndergroundSplitterInputClass))
	{
		if (auto underGroundBelt = Cast<AFGBuildableStorage>(newBuildable))
		{
			addUndergroundInputBelt(underGroundBelt);

			return true;
		}
	}
	else if ( /*!UMarcioCommonLibsConfiguration::configuration.ignoreStorageTeleporter &&*/
		commonInfoSubsystem->baseStorageTeleporterClass &&
		newBuildable->IsA(commonInfoSubsystem->baseStorageTeleporterClass))
	{
		if (auto storageTeleporter = Cast<AFGBuildableFactory>(newBuildable))
		{
			commonInfoSubsystem->addTeleporter(storageTeleporter);
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

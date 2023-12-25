#pragma once
#include "GameFramework/Actor.h"
#include "Resources/FGItemDescriptor.h"

#include "MarcioCommonLibsUtils.generated.h"

UCLASS(BlueprintType)
class MARCIOCOMMONLIBS_API UMarcioCommonLibsUtils : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static void DumpUnknownClass
	(
		UObject* obj,
		const FString& linePrefix = TEXT(""),
		const FString& filePrefix = TEXT(""),
		const FString& fileSuffix = TEXT(""),
		bool includeDateTime = true
	);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static class AActor* GetHitActor(const FHitResult& hit);

	UFUNCTION(BlueprintCallable, Category="MarcioCommonLibsUtils")
	static void DumpInformation(AActor* worldContext, TSubclassOf<class UFGEquipmentDescriptor> itemDescriptorClass);

	static FString getEnumItemName(const TCHAR* name, int value);
};

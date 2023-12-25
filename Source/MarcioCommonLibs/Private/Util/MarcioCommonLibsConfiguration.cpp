#include "Util/MarcioCommonLibsConfiguration.h"
#include "Util/MCLLogging.h"
#include "Util/MCLOptimize.h"

#ifndef OPTIMIZE
#pragma optimize("", off)
#endif

FMarcioCommonLibs_ConfigStruct UMarcioCommonLibsConfiguration::configuration;

void UMarcioCommonLibsConfiguration::SetMarcioCommonLibsConfiguration(const FMarcioCommonLibs_ConfigStruct& in_configuration)
{
	configuration = in_configuration;

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	MCL_LOG_Display(TEXT("setConfiguration"));

	MCL_LOG_Display(TEXT("logLevel = "), configuration.logLevel);

	MCL_LOG_Display(TEXT("==="));
}

void UMarcioCommonLibsConfiguration::GetMarcioCommonLibsConfiguration(int& out_logLevel)
{
	out_logLevel = configuration.logLevel;
}

int UMarcioCommonLibsConfiguration::GetLogLevelMCL()
{
	return configuration.logLevel;
}

#ifndef OPTIMIZE
#pragma optimize("", on)
#endif

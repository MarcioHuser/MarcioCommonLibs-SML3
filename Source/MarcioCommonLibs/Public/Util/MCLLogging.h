#pragma once

#include "Logging/LogMacros.h"

#include <sstream>

#define FUNCTIONSTR2(x) #x
#define FUNCTIONSTR TEXT(FUNCTIONSTR2(__FUNCTION__))

class CommaLog
{
public:
	template <typename T>
	inline CommaLog&
	operator,(const T& value)
	{
		wos << value;

		return *this;
	}

	inline CommaLog&
	operator,(const FString& value)
	{
		wos << *value;

		return *this;
	}

	inline CommaLog&
	operator,(const FText& value)
	{
		wos << *value.ToString();

		return *this;
	}

	std::wostringstream wos;
};

DECLARE_LOG_CATEGORY_EXTERN(LogMarcioCommonLibs, Log, All)

#define MCL_LOG_Verbosity(verbosity, first, ...) \
	{ \
		CommaLog l; \
		l, first, ##__VA_ARGS__; \
		UE_LOG(LogMarcioCommonLibs, verbosity, TEXT("%s"), l.wos.str().c_str()) \
	}

#define MCL_LOG_Log(first, ...) MCL_LOG_Verbosity(Log, first, ##__VA_ARGS__)
#define MCL_LOG_Display(first, ...) MCL_LOG_Verbosity(Display, first, ##__VA_ARGS__)
#define MCL_LOG_Warning(first, ...) MCL_LOG_Verbosity(Warning, first, ##__VA_ARGS__)
#define MCL_LOG_Error(first, ...) MCL_LOG_Verbosity(Error, first, ##__VA_ARGS__)

#define IS_MCL_LOG_LEVEL(level) (UMarcioCommonLibsConfiguration::configuration.logLevel > 0 && UMarcioCommonLibsConfiguration::configuration.logLevel >= static_cast<uint8>(level))

#define MCL_LOG_Log_Condition(first, ...) if(IS_MCL_LOG_LEVEL(ELogVerbosity::Log)) MCL_LOG_Log(first, ##__VA_ARGS__)
#define MCL_LOG_Display_Condition(first, ...) if(IS_MCL_LOG_LEVEL(ELogVerbosity::Display)) MCL_LOG_Display(first, ##__VA_ARGS__)
#define MCL_LOG_Warning_Condition(first, ...) if(IS_MCL_LOG_LEVEL(ELogVerbosity::Warning)) MCL_LOG_Warning(first, ##__VA_ARGS__)
#define MCL_LOG_Error_Condition(first, ...) if(IS_MCL_LOG_LEVEL(ELogVerbosity::Error)) MCL_LOG_Error(first, ##__VA_ARGS__)

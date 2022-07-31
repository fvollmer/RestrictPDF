#pragma once

#include <comdef.h>

// RAII wrapper class for CoInitializeEx and CoUninitialize
class CCoInitializer
{
public:
	CCoInitializer(DWORD dwCoInit);
	~CCoInitializer();

private:
	bool _initialized;

	CCoInitializer(const CCoInitializer &);
	CCoInitializer &operator=(const CCoInitializer &);
};

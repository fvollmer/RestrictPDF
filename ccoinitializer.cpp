#include <ccoinitializer.hpp>

CCoInitializer::CCoInitializer(DWORD dwCoInit)
	: _initialized(false)
{
	HRESULT hr = CoInitializeEx(NULL, dwCoInit);
	if (SUCCEEDED(hr))
	{
		_initialized = true;
	}
}

CCoInitializer::~CCoInitializer()
{
	if (_initialized)
	{
		CoUninitialize();
	}
}
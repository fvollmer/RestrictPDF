#include <comdef.h>

#include "util.hpp"

_COM_SMARTPTR_TYPEDEF(IShellItem, __uuidof(IShellItem));

std::vector<std::filesystem::path> shellItemArrayToPathVector(IShellItemArray *psia)
{
	std::vector<std::filesystem::path> file_list;
	HRESULT hr;
	DWORD cItems;
	hr = psia->GetCount(&cItems);
	if (FAILED(hr) || cItems == 0)
	{
		return file_list; // return empty
	}

	for (DWORD i = 0; i < cItems; ++i)
	{
		IShellItemPtr pShellItem;
		hr = psia->GetItemAt(i, &pShellItem);
		if (FAILED(hr))
		{
			continue;
		}

		PWSTR pszFilePath;
		hr = pShellItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

		// add to list
		if (SUCCEEDED(hr))
		{
			file_list.push_back(std::filesystem::path(pszFilePath));
			CoTaskMemFree(pszFilePath);
		}
	}

	return file_list;
}

std::string getRandomString(size_t len)
{
	static const char chars[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789";
	std::string str(len, ' ');

	for (size_t i = 0; i < len; ++i)
	{
		str[i] = chars[rand() % (ARRAYSIZE(chars) - 1)];
	}

	return str;
}

std::string utf8_encode(const WCHAR *wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	std::string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str.data(), size_needed, nullptr, nullptr);
	return str;
}
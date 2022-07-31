#pragma once

#include <vector>
#include <filesystem>

#include <shobjidl.h>

// create vector of filesystem paths from IShellItemArray
std::vector<std::filesystem::path> shellItemArrayToPathVector(IShellItemArray *psia);

// creat a random string of size len (not cryptographically secure!)
std::string getRandomString(size_t len);

// encoodes a utf16 char array as utf8 string
std::string utf8_encode(const WCHAR *wstr);
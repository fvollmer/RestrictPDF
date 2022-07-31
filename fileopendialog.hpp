#include <vector>
#include <filesystem>

#include "parameters.hpp"

HRESULT openFileChooser(HWND parent_hwnd, std::vector<std::filesystem::path> &file_list, Parameters &parameters);
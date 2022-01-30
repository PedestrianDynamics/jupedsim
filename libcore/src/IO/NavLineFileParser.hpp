#include "NavLineParameters.hpp"

#include <filesystem>
#include <vector>

std::vector<NavLineParameters> parseNavLines(const std::filesystem::path & filename);

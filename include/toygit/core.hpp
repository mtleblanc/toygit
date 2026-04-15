#pragma once

#include <string_view>

namespace toygit {

void storeObject(std::string_view object);
void storeBlob(std::string_view blob);

} // namespace toygit

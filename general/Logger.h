#pragma once

#include <string_view>

namespace Logging {

    void Setup();
    void Teardown();

    void Debug(std::string_view msg);
    void Info(std::string_view msg);
    void Warning(std::string_view msg);
    void Error(std::string_view msg);

    struct Guard {
        Guard();
        ~Guard();
    };
}

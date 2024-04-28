#pragma once

#include <cstdlib>

namespace usos_rpc {

    /// @brief Returns true if the output of this program should be colored.
    /// @return true if NO_COLOR environment variable is not set
    bool should_show_colored_output() {
        auto no_color = std::getenv("NO_COLOR");
        return no_color == nullptr || no_color[0] == '\0';
    }

}

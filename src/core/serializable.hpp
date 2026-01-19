#pragma once

#include "utils.hpp"

namespace snow {
    class Serializable {
        public:
            virtual u8* Serialize() const = 0;
            virtual bool Deserialize(const u8* data) = 0;
    };
}

#ifndef SCOUT_BRIDGE_H_
#define SCOUT_BRIDGE_H_

#include <cbor.h>
#include "cn-cbor/cn-cbor.h"

#include <Mesh.h>

namespace pinoccio {
    class Bridge {
        public:
            Bridge();

            void setup(Mesh mesh);

            static void write(const cn_cbor* data);
            static bool handleReport(uint8_t srcAddress, uint8_t srcEndpoint, const cn_cbor *data);

            bool available;
    };
}

#endif

#include "../include/Buffer.h"
#include "../include/GCLContext.h"

#include <cstdint>

int32_t main(int32_t argc, char* argv[]) {
    gcl::GCLContext context;

    gcl::Buffer buf(context, 1024);

    return 0;
}

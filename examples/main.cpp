//
//   Copyright (c) 2025 Nicholas Marino
//   All rights reserved.
//

#include "../include/Buffer.h"
#include "../include/GCLContext.h"
#include "../include/Kernel.h"

#include <cstdint>

int32_t main(int32_t argc, char* argv[]) {
    gcl::GCLContext context;

    gcl::Buffer alpha(context, 16);
    gcl::Buffer beta(context, 16);
    gcl::Buffer out(context, 16);

    gcl::Kernel ma = gcl::Kernel(context, "kernels/t.spv");

    ma.bind(0, alpha);
    ma.bind(1, beta);
    ma.bind(2, out);

    ma.dispatch(16);

    return 0;
}

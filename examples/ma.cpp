//
// Copyright (c) 2025 Nick Marino
// All rights reserved.
//

#include "../include/Buffer.h"
#include "../include/GCLContext.h"
#include "../include/Kernel.h"

#include <cstdint>
#include <iostream>
#include <vector>

int32_t main(int32_t argc, char** argv) {
     if (argc != 2) {
        std::cout << "usage: ./ma <N>" << std::endl;
        return 1;
    }

    gcl::GCLContext ctx;
    const uint32_t N = std::stoul(argv[1]);

    gcl::Buffer<float> a(ctx, N);
    gcl::Buffer<float> b(ctx, N);
    gcl::Buffer<float> r(ctx, N);

    std::vector<float> va(N), vb(N);
    for (uint32_t i = 0; i < N; ++i) {
        va[i] = float(i);
        vb[i] = float(i) * 2.f;
    }

    a.send(va);
    b.send(vb);

    gcl::Kernel k(ctx, "kernels/ma.spv");
    k.bind(0, a);
    k.bind(1, b);
    k.bind(2, r);

    k.dispatch(N);

    std::vector<float> out = r.fetch();
    for (uint32_t i = 0; i < N; ++i)
        std::cout << "r[" << i << "] = " << out[i] << '\n';

    return 0;
}

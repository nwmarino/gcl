//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/Buffer.h"
#include "../include/GCLContext.h"
#include "../include/Kernel.h"

#include <string>
#include <vector>
#include <iostream>
#include <chrono>

int32_t main(int32_t argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: heavy <N> <reps>" << std::endl;
        return 1;
    }

    auto t_end2end_start = std::chrono::high_resolution_clock::now();

    gcl::GCLContext ctx;
    const uint32_t N = std::stoul(argv[1]);
    const uint32_t reps = std::stoul(argv[2]);

    gcl::Buffer<float> a(ctx, N);
    gcl::Buffer<float> b(ctx, N);
    gcl::Buffer<float> r(ctx, N);

    std::vector<float> va(N), vb(N);
    for (uint32_t i = 0; i < N; ++i) {
        va[i] = (i % 1000) * 0.0007f;
        vb[i] = (i % 2048) * 0.0003f;
    }

    a.send(va);
    b.send(vb);

    gcl::Kernel k(ctx, "kernels/heavy.spv");
    k.bind(0, a);
    k.bind(1, b);
    k.bind(2, r);

    k.dispatch(N);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int32_t rep = 0; rep < reps; ++rep)
        k.dispatch(N);

    auto t1 = std::chrono::high_resolution_clock::now();

    std::vector<float> out = r.fetch();
    auto t_end2end_end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double, std::micro> us = t1 - t0;
    double end2end_us = std::chrono::duration<double,std::micro>(
        t_end2end_end - t_end2end_start).count();

    std::cout << "avg dispatch time (us) = " << (us.count() / reps) << '\n'
              << "end2end (us) = " << end2end_us << '\n';
    
    return 0;
}

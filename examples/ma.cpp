//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/Buffer.h"
#include "../include/GCLContext.h"
#include "../include/Kernel.h"

#include <cstdint>
#include <iostream>
#include <vector>

int32_t main() {
    gcl::GCLContext context;

    constexpr uint32_t N = 16;

    gcl::Buffer<float> alpha(context, N);
    gcl::Buffer<float> beta(context, N);
    gcl::Buffer<float> res(context, N);

    std::vector<float> as(N);
    for (uint32_t i = 0; i < N; ++i)
        as[i] = float(i);

    alpha.send(as);

    std::vector<float> bs(N);
    for (uint32_t i = 0; i < N; ++i)
        bs[i] = float(i) * 2.f;

    beta.send(bs);

    gcl::Kernel ma = gcl::Kernel(context, "kernels/ma.spv");
    ma.bind(0, alpha);
    ma.bind(1, beta);
    ma.bind(2, res);

    ma.dispatch(N);

    std::vector<float> result = res.fetch();

    for (uint32_t i = 0; i < N; ++i)
        std::cout << "r[" << i << "] = " << result[i] << '\n';

    return 0;
}

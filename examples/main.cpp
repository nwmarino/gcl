//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include "../include/Buffer.h"
#include "../include/GCLContext.h"
#include "../include/Kernel.h"

#include <cstdint>
#include <iostream>

int32_t main(int32_t argc, char* argv[]) {
    gcl::GCLContext context;

    constexpr uint32_t N = 16;
    constexpr uint64_t bytes = N * sizeof(float);

    gcl::Buffer alpha(context, bytes);
    gcl::Buffer beta(context, bytes);
    gcl::Buffer res(context, bytes);

    {
        void* p = nullptr;
        alpha.map(&p);

        float* af = static_cast<float*>(p);
        for (uint32_t i = 0; i < N; ++i)
            af[i] = float(i);

        alpha.flush();
        alpha.unmap();
    }

    {
        void* p = nullptr;
        beta.map(&p);

        float* bf = static_cast<float*>(p);
        for (uint32_t i = 0; i < N; ++i)
            bf[i] = float(i) * 2.f;

        beta.flush();
        beta.unmap();
    }

    gcl::Kernel ma = gcl::Kernel(context, "kernels/ma.spv");
    ma.bind(0, alpha);
    ma.bind(1, beta);
    ma.bind(2, res);

    ma.dispatch(N);

    {
        //res.invalidate();

        void* p = nullptr;
        res.map(&p);
        
        float* rf = static_cast<float*>(p);

        for (uint32_t i = 0; i < N; ++i)
            std::cout << "r[" << i << "] = " << rf[i] << '\n';
    
        res.unmap();
    }

    return 0;
}

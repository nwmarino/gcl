//
//   Copyright (c) 2025 Nick Marino
//   All rights reserved.
//

#include <string>
#include <vector>
#include <iostream>
#include <chrono>
#include <cmath>

static void cpu_branch(uint32_t N, uint32_t reps) {
    std::vector<float> a(N), b(N), out(N);
    for (uint32_t i = 0; i < N; ++i) {
        a[i] = float(i) / float(N);
        b[i] = (i % 1024) * 0.001f;
    }

    auto t_end2end_start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < N; ++i) {
        out[i] = (a[i] > 0.5f ? std::sqrt(a[i]) * b[i] + 1.0f
                              : (a[i] > 0.25f ? a[i] * b[i] - 0.5f
                                              : (a[i] + 0.001f) * (b[i] - 0.001f)));
    }

    auto t0 = std::chrono::high_resolution_clock::now();
    for (uint32_t rep = 0; rep < reps; ++rep) {
        for (uint32_t i = 0; i < N; ++i) {
            out[i] = (a[i] > 0.5f ? std::sqrt(a[i]) * b[i] + 1.0f
                                  : (a[i] > 0.25f ? a[i] * b[i] - 0.5f
                                                  : (a[i] + 0.001f) * (b[i] - 0.001f)));
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    auto t_end2end_end = std::chrono::high_resolution_clock::now();

    double loop_us = std::chrono::duration<double,std::micro>(t1 - t0).count();
    double end2end_us = std::chrono::duration<double,std::micro>(
        t_end2end_end - t_end2end_start).count();

    std::cout << "cpu_branch avg dispatch (us) = " << (loop_us / reps)
              << "\ncpu_branch end2end (us) = " << end2end_us << "\n";
}

static void cpu_heavy(uint32_t N, uint32_t reps) {
    std::vector<float> a(N), b(N), out(N);
    for (uint32_t i = 0; i < N; ++i) {
        a[i] = float(i) / float(N);
        b[i] = (i % 1024) * 0.001f;
    }

    auto poly = [](float x, float y) {
        float acc = 0.0f;
        acc = std::fma(x, y, acc);
        acc = std::fma(x * x, 0.25f, acc);
        acc = std::fma(y * y, 0.125f, acc);
        acc = std::fma(x * y, 0.0625f, acc);
        acc = std::fma(x + y, 0.03125f, acc);
        for (int k = 0; k < 8; ++k)
            acc = acc * 0.985123f + 0.314159f;
        return acc;
    };

    auto t_end2end_start = std::chrono::high_resolution_clock::now();

    for (uint32_t i = 0; i < N; ++i)
        out[i] = ((int(i & 7) < 3) ? poly(a[i], b[i])
                                   : poly(a[i] * 0.5f, b[i] * 1.5f));

    auto t0 = std::chrono::high_resolution_clock::now();
    for (uint32_t rep = 0; rep < reps; ++rep) {
        for (uint32_t i = 0; i < N; ++i)
            out[i] = ((int(i & 7) < 3) ? poly(a[i], b[i])
                                       : poly(a[i] * 0.5f, b[i] * 1.5f));
    }

    auto t1 = std::chrono::high_resolution_clock::now();

    auto t_end2end_end = std::chrono::high_resolution_clock::now();

    double loop_us = std::chrono::duration<double,std::micro>(t1 - t0).count();
    double end2end_us = std::chrono::duration<double,std::micro>(
        t_end2end_end - t_end2end_start).count();

    std::cout << "cpu_heavy avg dispatch (us) = " << (loop_us / reps)
              << "\ncpu_heavy end2end (us) = " << end2end_us << "\n";
}

int32_t main(int32_t argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: bm <N> <reps>\n";
        return 1;
    }

    uint32_t N = std::stoul(argv[1]);
    uint32_t reps = std::stoul(argv[2]);

    cpu_branch(N, reps);
    cpu_heavy(N, reps);

    return 0;
}

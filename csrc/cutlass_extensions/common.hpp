// 2025 - Modified by MetaX Integrated Circuits (Shanghai) Co., Ltd. All Rights Reserved. 
#pragma once

#ifndef USE_MACA
#include "cutlass/cutlass.h"
#else
#include "mctlass/mctlass.h"
#endif // USE_MACA

#include <climits>
#include "cuda_runtime.h"
#include <iostream>

/**
 * Helper function for checking CUTLASS errors
 */
#ifndef USE_MACA
#define CUTLASS_CHECK(status)                       \
  {                                                 \
    cutlass::Status error = status;                 \
    TORCH_CHECK(error == cutlass::Status::kSuccess, \
                cutlassGetStatusString(error));     \
  }
#else
#define CUTLASS_CHECK(status)                       \
  {                                                 \
    mctlass::Status error = status;                 \
    TORCH_CHECK(error == mctlass::Status::kSuccess, \
                mctlassGetStatusString(error));     \
  }
#endif // USE_MACA

/**
 * Panic wrapper for unwinding CUDA runtime errors
 */
#define CUDA_CHECK(status)                                        \
  {                                                               \
    cudaError_t error = status;                                   \
    TORCH_CHECK(error == cudaSuccess, cudaGetErrorString(error)); \
  }

inline int get_cuda_max_shared_memory_per_block_opt_in(int const device) {
  int max_shared_mem_per_block_opt_in = 0;
  cudaDeviceGetAttribute(&max_shared_mem_per_block_opt_in,
                         cudaDevAttrMaxSharedMemoryPerBlockOptin, device);
  return max_shared_mem_per_block_opt_in;
}

int32_t get_sm_version_num();

/**
 * A wrapper for a kernel that is used to guard against compilation on
 * architectures that will never use the kernel. The purpose of this is to
 * reduce the size of the compiled binary.
 * __CUDA_ARCH__ is not defined in host code, so this lets us smuggle the ifdef
 * into code that will be executed on the device where it is defined.
 */
template <typename Kernel>
struct enable_sm90_or_later : Kernel {
  template <typename... Args>
#ifndef USE_MACA
  CUTLASS_DEVICE void operator()(Args&&... args) {
#else
  MCTLASS_DEVICE void operator()(Args&&... args) {
#endif // USE_MACA
#if defined __CUDA_ARCH__ && __CUDA_ARCH__ >= 900
    Kernel::operator()(std::forward<Args>(args)...);
#endif
  }
};

template <typename Kernel>
struct enable_sm90_only : Kernel {
  template <typename... Args>
#ifndef USE_MACA
  CUTLASS_DEVICE void operator()(Args&&... args) {
#else
  MCTLASS_DEVICE void operator()(Args&&... args) {
#endif // USE_MACA
#if defined __CUDA_ARCH__ && __CUDA_ARCH__ == 900
    Kernel::operator()(std::forward<Args>(args)...);
#endif
  }
};

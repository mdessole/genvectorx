#include "SYCLMath/GenVector/MathUtil.h"
#include <cstdio>
#include <chrono>
#include <iostream>

#if defined(ROOT_MATH_SYCL)
#include <sycl/sycl.hpp>
using mode = sycl::access::mode;

#elif defined(ROOT_MATH_CUDA)
#include <cuda_runtime.h>

#define ERRCHECK(err) __checkCudaErrors((err), __FILE__, __LINE__)
inline static void __checkCudaErrors(cudaError_t code, const char *file, int line, bool abort = true)
{
  if (code != cudaSuccess)
  {
    fprintf(stderr, "GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
    if (abort)
      exit(code);
  }
}

#endif

// namespace ROOT {
// namespace Experimental {

namespace ROOT
{

  namespace Experimental
  {

#if defined(ROOT_MATH_CUDA)
    template <class Scalar, class LVector>
    __global__ void InvariantMassKernel(LVector *vec, Scalar *m, size_t N)
    {
      int id = blockDim.x * blockIdx.x + threadIdx.x;
      if (id < N)
      {
        LVector w = vec[id];
        m[id] = w.mass();
      }
    }

    template <class Scalar, class LVector>
    __global__ void InvariantMassesKernel(LVector *v1, LVector *v2, Scalar *m, size_t N)
    {
      int id = blockDim.x * blockIdx.x + threadIdx.x;
      if (id < N)
      {
        LVector w = v1[id] + v2[id];
        m[id] = w.mass();
      }
    }

    template <class Scalar, class LVector>
    Scalar *InvariantMasses(LVector *v1, LVector *v2, const size_t N,
                            const size_t local_size)
    {

      Scalar *invMasses = new Scalar[N];

      // Allocate device input vector
      LVector *d_v1 = NULL;
      ERRCHECK(cudaMalloc((void **)&d_v1, N * sizeof(LVector)));

      // Allocate device input vector
      LVector *d_v2 = NULL;
      ERRCHECK(cudaMalloc((void **)&d_v2, N * sizeof(LVector)));

      // Allocate the device output vector
      Scalar *d_invMasses = NULL;
      ERRCHECK(cudaMalloc((void **)&d_invMasses, N * sizeof(Scalar)));

#ifdef ROOT_MEAS_TIMING
      auto start = std::chrono::system_clock::now();
#endif

      {

        cudaMemcpy(d_v1, v1, N * sizeof(LVector), cudaMemcpyHostToDevice);
        cudaMemcpy(d_v2, v2, N * sizeof(LVector), cudaMemcpyHostToDevice);

        InvariantMassesKernel<<<N / local_size + 1, local_size>>>(d_v1, d_v2, d_invMasses, N);

        ERRCHECK(cudaMemcpy(invMasses, d_invMasses, N * sizeof(Scalar), cudaMemcpyDeviceToHost));
      }

#ifdef ROOT_MEAS_TIMING
      auto end = std::chrono::system_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() *
          1e-6;
      std::cout << "cuda time " << duration << " (s)" << std::endl;
#endif
      ERRCHECK(cudaFree(d_v1));
      ERRCHECK(cudaFree(d_v2));
      ERRCHECK(cudaFree(d_invMasses));

      return invMasses;
    }

    template <class Scalar, class LVector>
    Scalar *InvariantMass(LVector *v1, const size_t N, const size_t local_size)
    {

      Scalar *invMasses = new Scalar[N];

#ifdef ROOT_MEAS_TIMING
      auto start = std::chrono::system_clock::now();
#endif
      {
        // Allocate the device input vector
        LVector *d_v1 = NULL;
        ERRCHECK(cudaMalloc((void **)&d_v1, N * sizeof(LVector)));

        // Allocate the device output vector
        Scalar *d_invMasses = NULL;
        ERRCHECK(cudaMalloc((void **)&d_invMasses, N * sizeof(Scalar)));
        ERRCHECK(cudaMemcpy(d_v1, v1, N * sizeof(LVector), cudaMemcpyHostToDevice));

        cudaDeviceSynchronize();
        InvariantMassKernel<<<N / local_size + 1, local_size>>>(d_v1, d_invMasses, N);

        ERRCHECK(cudaMemcpy(invMasses, d_invMasses, N * sizeof(Scalar), cudaMemcpyDeviceToHost));

        ERRCHECK(cudaFree(d_v1));
        ERRCHECK(cudaFree(d_invMasses));
      }

#ifdef ROOT_MEAS_TIMING
      auto end = std::chrono::system_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() *
          1e-6;
      std::cout << "cuda time " << duration << " (s)" << std::endl;
#endif

      return invMasses;
    }
#elif defined(ROOT_MATH_SYCL)

    template <class AccScalar, class AccLVector>
    class InvariantMassKernel
    {
    public:
      InvariantMassKernel(AccLVector acc_vec, AccScalar acc_m, size_t n)
          : vec_acc(acc_vec), m_acc(acc_m), N(n) {}

      void operator()(sycl::nd_item<1> item)
      {
        size_t id = item.get_global_id().get(0);
        if (id < N)
        {
          m_acc[id] = vec_acc[id].mass();
        }
      }

    private:
      AccLVector vec_acc;
      AccScalar m_acc;
      size_t N;
    };

    template <class AccScalar, class AccLVector>
    class InvariantMassesKernel
    {
    public:
      InvariantMassesKernel(AccLVector acc_v1, AccLVector acc_v2, AccScalar acc_m, size_t n)
          : v1_acc(acc_v1), v2_acc(acc_v2), m_acc(acc_m), N(n) {}

      void operator()(sycl::nd_item<1> item) const
      {
        size_t id = item.get_global_id().get(0);
        if (id < N)
        {
          auto w = v1_acc[id] + v2_acc[id];
          m_acc[id] = w.mass();
        }
      }

    private:
      AccLVector v1_acc;
      AccLVector v2_acc;
      AccScalar m_acc;
      size_t N;
    };

    template <class Scalar, class LVector>
    Scalar *InvariantMasses(LVector *v1, LVector *v2, const size_t N,
                            const size_t local_size,
                            sycl::queue queue)
    {

      Scalar *invMasses = new Scalar[N];
      std::cout << "sycl::queue check - selected device:\n"
                << queue.get_device().get_info<sycl::info::device::name>()
                << std::endl;

#ifdef ROOT_MEAS_TIMING
      auto start = std::chrono::system_clock::now();
#endif

      { // Start of scope, ensures data copied back to host
        // Create device buffers. The memory is managed by SYCL so we should NOT
        // access these buffers directly.
        auto execution_range = sycl::nd_range<1>{
            sycl::range<1>{((N + local_size - 1) / local_size) * local_size},
            sycl::range<1>{local_size}};

        sycl::buffer<LVector, 1> v1_sycl(v1, sycl::range<1>(N));
        sycl::buffer<LVector, 1> v2_sycl(v2, sycl::range<1>(N));
        sycl::buffer<Scalar, 1> m_sycl(invMasses, sycl::range<1>(N));

        queue.submit([&](sycl::handler &cgh)
                     {
                       // Get handles to SYCL buffers.
                       sycl::accessor v1_acc{v1_sycl, cgh, sycl::range<1>(N), sycl::read_only};
                       sycl::accessor v2_acc{v2_sycl, cgh, sycl::range<1>(N), sycl::read_only};
                       sycl::accessor m_acc{m_sycl, cgh, sycl::range<1>(N), sycl::write_only};
                       //auto v1_acc = v1_sycl.get_access<mode::read>(cgh);
                       //auto v2_acc = v2_sycl.get_access<mode::read>(cgh);
                       //auto m_acc = m_sycl.get_access<mode::write>(cgh);

                       cgh.parallel_for(execution_range, InvariantMassesKernel(v1_acc, v2_acc, m_acc, N)); });
      } // end of scope, ensures data copied back to host
      queue.wait();

#ifdef ROOT_MEAS_TIMING
      auto end = std::chrono::system_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start)
              .count() *
          1e-6;
      std::cout << "sycl time " << duration << " (s)" << std::endl;
#endif
      return invMasses;
    }

#endif

  } // namespace Experimental
} // namespace ROOT

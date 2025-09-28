#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <iostream>
#include <iomanip>

typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t  s64;

const u32 COUNT = 1;

class RV_t
{
    public:
	virtual void vfmmacc(u32 vd, u32 vs1, u32 vs2) = 0;
	virtual void vxor(u32 vd, u32 vs1, u32 vs2) = 0;
	virtual void vle64(u32 vd, double *A) = 0;
	virtual void vse64(u32 vs, double *A) = 0;
	virtual u32& SEW() = 0;
	virtual s64& X(u32 rs) = 0;
	virtual u32 VLENE() const = 0;
	virtual u32 mu() const = 0;
	virtual u32 nu() const = 0;
};

template<u32 VLEN, u32 lambda>
class RVIME_t : public RV_t
{
    private:
	u32	SEW_;
	bool	altfmt_;
	union
	{
	    double f64[VLEN/64];
	    float  f32[VLEN/32];
	    u64    i64[VLEN/64];
	} VR[32];
	s64 X_[32];

	void vfmmacc_fp64(u32 vd, u32 vs1, u32 vs2)
	{
	    u32 L = VLEN/SEW_;
	    u32 m = L/lambda;
	    u32 n = m/lambda;
	    std::cout << "Executing double-precision vfmmacc : " << "vd = " << vd << ", vs1 = " << vs1 << ", vs2 = " << vs2 << ", L = " << L << ", m = " << m << ", n = " << n << std::endl;

	    for (u32 k=0; k<lambda; k++)
	    {
		for (u32 i=0; i<m; i++)
		    for (u32 j=0; j<m; j++)
		    {
			std::cout << "Computing VR[" << vd + j/lambda << "].f64[" << i + m*(j%lambda) << "] += VR[" << vs1 << "].f64[" << i + m*k << "] * VR[" << vs2 << "].f64[" << j + m*k << "]" << std::endl;
			VR[vd + j/lambda].f64[i + m*(j%lambda)] += VR[vs1].f64[i + m*k] * VR[vs2].f64[j + m*k]; 
			std::cout << "VR[" << vs1 << "].f64[" << i + m*k << "] = " << VR[vs1].f64[i + m*k] << std::endl;
			std::cout << "VR[" << vs2 << "].f64[" << j + m*k << "] = " << VR[vs2].f64[j + m*k] << std::endl;
			std::cout << "VR[" << vd + j/lambda << "].f64[" << i + m*(j%lambda) << "] = " << VR[vd + j/lambda].f64[i + m*(j%lambda)] << std::endl;
		    }
	    }
	}

	void vxor_i64(u32 vd, u32 vs1, u32 vs2)
	{
	    u32 L = VLEN/SEW_;
	    for (u32 i=0; i<L; i++) VR[vd].i64[i] = VR[vs1].i64[i] ^ VR[vs2].i64[i];
	}

    public:
	u32& SEW()
	{
	    return SEW_;
	}

	u32 mu() const
	{
	    u32 L = VLEN/SEW_;
	    return L/lambda;
	}

	u32 nu() const
	{
	    u32 L = VLEN/SEW_;
	    return L/lambda;
	}

	s64& X(u32 rs)
	{
	    assert(rs < 32);
	    return X_[rs];
	}

	void vfmmacc(u32 vd, u32 vs1, u32 vs2)
	{
	    switch(SEW_)
	    {
		case 64:
		    vfmmacc_fp64(vd, vs1, vs2);
		    break;
		default:
		    assert(false);
	    }
	}

	void vxor(u32 vd, u32 vs1, u32 vs2)
	{
	    switch(SEW_)
	    {
		case 64:
		    vxor_i64(vd, vs1, vs2);
		    break;
		default:
		    assert(false);
	    }
	}

	void vle64(u32 vd, double *A)
	{
	    std::cout << "Loading VR[" << vd << "]" << std::endl;
	    u32 L = VLEN/SEW_;
	    for (u32 i=0; i<L; i++) VR[vd].f64[i] = A[i];
	}

	void vse64(u32 vs, double *A)
	{
	    std::cout << "Storing VR[" << vs << "]" << std::endl;
	    u32 L = VLEN/SEW_;
	    for (u32 i=0; i<L; i++) A[i] = VR[vs].f64[i];
	}

	u32 VLENE() const
	{
	    return VLEN/SEW_;
	}
};

RV_t *RV = nullptr;
class vsetvli_t
{
    public:
	void operator()(u32 rd, u32 rs1, u32 sew, u32 lmul, u32 ta, u32 ma)
	{
	    RV->SEW() = sew;
	    RV->X(rd) = RV->VLENE();
	}
};

class vfmmacc_t
{
    public:
	void vv(u32 vd, u32 vs1, u32 vs2)
	{
	    return RV->vfmmacc(vd, vs1, vs2);
	}
};

class vxor_t
{
    public:
	void vv(u32 vd, u32 vs1, u32 vs2)
	{
	    return RV->vxor(vd, vs1, vs2);
	}
};

class vle64_t
{
    public:
	void v(u32 vd, double *A)
	{
	    return RV->vle64(vd, A);
	}
};

class vse64_t
{
    public:
	void v(u32 vs, double *A)
	{
	    return RV->vse64(vs, A);
	}
};

vfmmacc_t vfmmacc;
vsetvli_t vsetvli;
vxor_t    vxor;
vle64_t	  vle64;
vse64_t   vse64;

void microgemm
(
    u32     K,
    double  alpha,
    double *A,
    double *B,
    double *C,
    u32     ldc
)
{
    vsetvli(5, 0, 64, 1, true, true);
    for (u32 r=16; r<32; r++) vxor.vv(r, r, r);
    for (u32 k=0; k<K; k++)
    {
	vle64.v(0, A);
	vle64.v(8, B);
	vfmmacc.vv(16, 0, 8);
	A += RV->X(5);
	B += RV->X(5);
    }
    vse64.v(16, C);
}

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

template<u32 VLEN, u32 lambda>
bool run_microgemm
(
    u32 K
)
{
    RVIME_t<VLEN, lambda> RVIME; RV = &RVIME;

    // Find the geometry of the microgemm
    vsetvli(5, 0, 64, 1, true, true);
    u32 M = RV->mu();
    u32 N = RV->nu();

    std::cout << "Microgemm geometry : " << M << " x " << N << std::endl;

    // Allocate A, B, and C panels
    double *A = new double[M*K]; for (u32 i=0; i<M*K; i++) A[i] = drand48() - 0.5;
    double *B = new double[K*N]; for (u32 i=0; i<K*N; i++) B[i] = drand48() - 0.5;
    double *C = new double[M*N]; for (u32 i=0; i<M*N; i++) C[i] = drand48() - 0.5;

    // Invoke the microgemm kernel
    microgemm(K, 1.0, A, B, C, M);

    // Check the result
    for (u32 j=0; j<N; j++)
	for (u32 i=0; i<M; i++)
	{
	    double S = 0;
	    for (u32 k=0; k<K; k++)
		S += A[i+k*M]*B[j+k*N];
	    if (S != C[i+j*M])
	    {
		std::cerr << "Error for C[" << i << "," << j << "] = " << S << std::endl;
		exit(-1);
	    }
	}

    // Free the panels
    delete [] C;
    delete [] B;
    delete [] A;

    // Successful execution
    return true;
}

double run_kernel
(
    void (kernel)(double*, double*, double*, double*),
    uint32_t count
)
{
    const uint32_t N = 1024*1024;
    double *A = (double*)aligned_alloc(4096, sizeof(double) * N); for (uint32_t i=0; i<N; i++) A[i] = drand48() - 0.5;
    double *B = (double*)aligned_alloc(4096, sizeof(double) * N); for (uint32_t i=0; i<N; i++) B[i] = drand48() - 0.5;
    double *C = (double*)aligned_alloc(4096, sizeof(double) * N); for (uint32_t i=0; i<N; i++) C[i] = drand48() - 0.5;
    double *D = (double*)aligned_alloc(4096, sizeof(double) * N); for (uint32_t i=0; i<N; i++) D[i] = drand48() - 0.5;

    volatile double start, finish;

    start = now();
    for(; count; count -= COUNT)
    {
	kernel(A,B,C,D);
    }
    finish = now();

    free(D);
    free(C);
    free(B);
    free(A);

    return (finish - start);
}

void run_kernel_and_report
(
    void (kernel)(double*, double*, double*, double*),
    uint32_t count,
    const char* name,
    uint32_t M,
    uint32_t N,
    uint32_t K
)
{
    volatile double elapsed;
    volatile double flops = 2.0*count*M*N*K;
    elapsed = run_kernel(kernel, count);
    std::cout << std::setprecision(6);
    std::cout << "Time to run " << std::setw(51) << name << " " << count << " times = " << std::setw(10) << std::fixed << elapsed << " seconds (" << std::setw(10) << std::scientific << flops/elapsed << " flops)" << std::endl;
}

#define RUN_KERNEL(kernel, count, M, N, K) run_kernel_and_report(kernel, count, #kernel, M, N, K)

int main
(
    int		argc,
    char      **argv
)
{
    std::cout << "=========================================================================================================================" << std::endl;
    run_microgemm< 64, 1>(2);
    run_microgemm<128, 1>(1);
    run_microgemm<256, 1>(1);

    return 0;
}

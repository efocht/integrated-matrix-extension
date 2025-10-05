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

const u32 debug = 0;

class RV_t
{
    public:
	virtual void vfmmacc(u32 vd, u32 vs1, u32 vs2) = 0;
	virtual void vxor(u32 vd, u32 vs1, u32 vs2) = 0;
	virtual void vle64(u32 vd, double *A) = 0;
	virtual void vse64(u32 vs, double *A) = 0;
	virtual u32& SEW() = 0;
	virtual u32& LMUL() = 0;
	virtual u32& RMUL() = 0;
	virtual u32& CMUL() = 0;
	virtual s64& X(u32 rs) = 0;
	virtual u32  VLEN() const = 0;
	virtual u32& VL() = 0;
	virtual u32  lambda() const = 0;
	virtual u32  VLENE() const = 0;
	virtual u32  sigma() const = 0;
};

template<u32 VLEN_, u32 lambda_>
class RVIME_t : public RV_t
{
    private:
	u32	SEW_;
	u32     LMUL_;
	u32	RMUL_;
	u32	CMUL_;
	u32	VL_;
	bool	altfmt_;
	union
	{
	    double f64[VLEN_/64];
	    float  f32[VLEN_/32];
	    u64    i64[VLEN_/64];
	} VR[32];
	s64 X_[32];

	void vfmmacc_fp64(u32 vd, u32 vs1, u32 vs2)
	{
	    u32 L = VLEN_/SEW_;
	    u32 m = L/lambda_;
	    u32 n = m/lambda_;
	    if (debug > 0)
	    {
		std::cout << "Executing double-precision vfmmacc : " << "vd = " << vd << ", vs1 = " << vs1 << ", vs2 = " << vs2 << ", L = " << L << ", m = " << m << ", n = " << n << std::endl;
	    }

	    for (u32 k=0; k<lambda_; k++)
	    {
		for (u32 i=0; i<m; i++)
		    for (u32 j=0; j<m; j++)
		    {
			if (debug > 0)
			{
			    std::cout << "Computing VR[" << vd + j/lambda_ << "].f64[" << i + m*(j%lambda_) << "] += VR[" << vs1 << "].f64[" << i + m*k << "] * VR[" << vs2 << "].f64[" << j + m*k << "]" << std::endl;
			}
			VR[vd + j/lambda_].f64[i + m*(j%lambda_)] += VR[vs1].f64[i + m*k] * VR[vs2].f64[j + m*k]; 
			if (debug > 0)
			{
			    std::cout << "VR[" << vs1 << "].f64[" << i + m*k << "] = " << VR[vs1].f64[i + m*k] << std::endl;
			    std::cout << "VR[" << vs2 << "].f64[" << j + m*k << "] = " << VR[vs2].f64[j + m*k] << std::endl;
			    std::cout << "VR[" << vd + j/lambda_ << "].f64[" << i + m*(j%lambda_) << "] = " << VR[vd + j/lambda_].f64[i + m*(j%lambda_)] << std::endl;
			}
		    }
	    }
	}

	void vxor_i64(u32 vd, u32 vs1, u32 vs2)
	{
	    u32 L = VLEN_/SEW_;
	    for (u32 i=0; i<L; i++) VR[vd].i64[i] = VR[vs1].i64[i] ^ VR[vs2].i64[i];
	}

    public:
	u32& SEW()
	{
	    return SEW_;
	}

	u32& LMUL()
	{
	    return LMUL_;
	}

	u32& RMUL()
	{
	    return RMUL_;
	}

	u32& CMUL()
	{
	    return CMUL_;
	}

	u32 VLEN() const
	{
	    return VLEN_;
	}

	u32& VL()
	{
	    return VL_;
	}

	u32 lambda() const
	{
	    return lambda_;
	}

	u32 sigma() const
	{
	    u32 L = VLEN_/SEW_;
	    return L/lambda_;
	}

	s64& X(u32 rs)
	{
	    assert(rs < 32);
	    return X_[rs];
	}

	void vfmmacc(u32 vd, u32 vs1, u32 vs2)
	{
	    u32 B = VLENE()/(lambda() * lambda());
	    switch(SEW_)
	    {
		case 64:
		    if (debug > 0)
		    {
			std::cout << "Each basic vfmmacc produces " << B << " vector registers of output" << std::endl;
		    }
		    for (u32 i=0; i<RMUL(); i++) for (u32 j=0; j<CMUL(); j++)
			vfmmacc_fp64(vd + B*i + B*j*RMUL(), vs1 + i, vs2 + j);
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
	    if (debug > 0) { std::cout << "Loading VR[" << vd << "]" << std::endl; }
	    u32 L = VLEN_/SEW_;
	    for (u32 i=0; i<L; i++) VR[vd].f64[i] = A[i];
	}

	void vse64(u32 vs, double *A)
	{
	    if (debug > 0) { std::cout << "Storing VR[" << vs << "]" << std::endl; }
	    u32 L = VLEN_/SEW_;
	    for (u32 i=0; i<L; i++) A[i] = VR[vs].f64[i];
	}

	u32 VLENE() const
	{
	    return VLEN_/SEW_;
	}
};

RV_t *RV = nullptr;

class vsetvli_t
{
    public:
	void operator()(u32 rd, u32 rs1, u32 sew, u32 lmul, u32 ta, u32 ma)
	{
	    assert(0 == rs1);
	    RV->SEW() = sew;
	    RV->LMUL() = lmul;
	    RV->VL() = lmul*RV->VLENE();
	    RV->X(rd) = RV->VL();
	}
};

class vsetvl_t
{
    public:
	void operator()(u32 rd, u32 rs1, u32 sew, u32 lmul, u32 ta, u32 ma)
	{
	    assert(0 == rs1);
	    RV->SEW() = sew;
	    RV->LMUL() = lmul;
	    RV->VL() = lmul*RV->VLENE();
	    RV->X(rd) = RV->VL();
	}
};

class vsetmul_t
{
    public:
	void operator()(u32 rmul, u32 cmul)
	{
	    RV->CMUL() = cmul;
	    RV->RMUL() = rmul;
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
	    assert(RV->VL() == RV->VLENE() * RV->LMUL());
	    for (u32 i=0; i<RV->LMUL(); i++) RV->vle64(vd + i, A + i*RV->VLENE());
	    return;
	}
};

class vse64_t
{
    public:
	void v(u32 vs, double *A)
	{
	    assert(RV->VL() == RV->VLENE() * RV->LMUL());
	    for (u32 i=0; i<RV->LMUL(); i++) RV->vse64(vs + i, A + i*RV->VLENE());
	    return;
	}
};

vfmmacc_t vfmmacc;
vsetvli_t vsetvli;
vsetvl_t  vsetvl;
vsetmul_t vsetmul;
vxor_t    vxor;
vle64_t	  vle64;
vse64_t   vse64;

void microgemm
(
    u32     K,
    double *A,
    double *B,
    double *C,
    u32     M,
    u32     N,
    u32     rmul,
    u32     cmul
)
{
    assert(0 == K % RV->lambda());

    vsetvli(5, 0, 64, 1, true, true);
    vsetmul(rmul,cmul);
    for (u32 r=16; r<32; r++) vxor.vv(r, r, r);
    for (u32 k=0; k<K; k=k+RV->lambda())
    {
	vsetvli(5, 0, 64, RV->RMUL(), true, true);
	vle64.v(0, A);
	vsetvli(5, 0, 64, RV->CMUL(), true, true);
	vle64.v(8, B);
	vfmmacc.vv(16, 0, 8);
	A += M * RV->lambda();
	B += N * RV->lambda();
    }
    vsetvli(5, 0, 64, 1, true, true);
    u32 D = RV->VLENE()/(RV->lambda() * RV->lambda());
    for (u32 j=0; j<cmul; j++)
	for (u32 k=0; k<D; k++)
	    for (u32 i=0; i<rmul; i++)
	    {
		vse64.v(16 + D*i + D*j*rmul + k, C);
		C += RV->VLENE();
	    }
}

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

void packfp64
(
    double *P,
    double *A
)
{
}

template<u32 VLEN, u32 lambda>
bool run_microgemm
(
    u32 K
)
{
    assert(lambda == 1);
    RVIME_t<VLEN, lambda> RVIME; RV = &RVIME;

    // Find the geometry of the microgemm
    vsetvli(5, 0, 64, 1, true, true);
    u32 L = RV->VLENE();
    assert(L >= lambda);
    assert(L/(lambda*lambda) >=  1);
    assert(L/(lambda*lambda) <= 16);

    u32 rmul = 1;
    u32 cmul = 1;
    while (rmul * cmul * L/(lambda*lambda) < 16)
    {
	if (cmul > rmul) rmul = rmul*2;
	else cmul = cmul * 2;
    }
    std::cout << "L = " << std::setw(2) << L << ", lambda = " << std::setw(2) << RV->lambda() << ", sigma = " << std::setw(2) << RV->sigma() << ", RMUL = " << rmul << ", CMUL = " << cmul;

    u32 mu = rmul*RV->sigma();
    u32 nu = cmul*RV->sigma();

    std::cout << ", microgemm geometry : " << std::setw(2) << mu << " x " << std::setw(2) << nu << std::endl;

    u32 M = mu;
    u32 N = nu;

    // Allocate A, B, and C panels
    double *A = new double[M*K]; for (u32 i=0; i<M*K; i++) A[i] = drand48() - 0.5;
    double *B = new double[K*N]; for (u32 i=0; i<K*N; i++) B[i] = drand48() - 0.5;
    double *C = new double[M*N]; for (u32 i=0; i<M*N; i++) C[i] = drand48() - 0.5;

    // Allocate the packed panels
    double *Ap = new double[M*K];
    double *Bp = new double[K*N];

    // Pack the A and B panels
    packfp64(Ap, A);
    packfp64(Bp, B);

    // Invoke the microgemm kernel
    microgemm(K, A, B, C, M, N, rmul, cmul);

    // Check the result
    for (u32 j=0; j<N; j++)
	for (u32 i=0; i<M; i++)
	{
	    double S = 0;
	    for (u32 k=0; k<K; k++)
	    {
		if (debug > 1)
		{
		    if ((2 == i) && (0 == j))
			std::cout << "A[" << i << ", " << k << "] = " << A[i+k*M] << std::endl;
		}
		S += A[i+k*M]*B[j+k*N];
	    }
	    if (S != C[i+j*M])
	    {
		std::cout << "Error for C[" << i << "," << j << "] = " << C[i+j*M] << " != " << S << std::endl;
		exit(-1);
	    }
	}

    // Free the panels
    delete [] Bp;
    delete [] Ap;
    delete [] C;
    delete [] B;
    delete [] A;

    // Successful execution
    return true;
}

int main
(
    int		argc,
    char      **argv
)
{
    std::cout << "=========================================================================================================================" << std::endl;
    run_microgemm<  64, 1>(1);
    run_microgemm<  64, 1>(2);
    run_microgemm<  64, 1>(4);
    run_microgemm<  64, 1>(8);
    run_microgemm< 128, 1>(1);
    run_microgemm< 128, 1>(2);
    run_microgemm< 128, 1>(4);
    run_microgemm< 128, 1>(8);
    run_microgemm< 256, 1>(1);
    run_microgemm< 256, 1>(2);
    run_microgemm< 256, 1>(4);
    run_microgemm< 256, 1>(8);
    run_microgemm< 512, 1>(1);
    run_microgemm< 512, 1>(2);
    run_microgemm< 512, 1>(4);
    run_microgemm< 512, 1>(8);
    run_microgemm<1024, 1>(1);
    run_microgemm<1024, 1>(2);
    run_microgemm<1024, 1>(4);
    run_microgemm<1024, 1>(8);

    return 0;
}

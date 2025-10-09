#include <stdint.h>
#include <time.h>
#include <iostream>
#include <iomanip>
#include <assert.h>

// helper macros
#define EXPAND(x)  _EXPAND(x)
#define _EXPAND(x) #x

// assembly instructions
#define li(RT, SI)			asm("li " #RT "," EXPAND(SI))
#define lxvp(XT, RA, D)			asm("lxvp " #XT "," #D "(" #RA ")")
#define stxvp(XS, RA, D)		asm("stxvp " #XS "," #D "(" #RA ")")
#define xvf64ger(AT, XA, XB)		asm("xvf64ger " #AT "," #XA "," #XB)
#define xvf64gerpp(AT, XA, XB)		asm("xvf64gerpp " #AT "," #XA "," #XB)
#define xxpermdi(XT, XA, XB, IM)	asm("xxpermdi " #XT "," #XA "," #XB "," #IM)
#define xxmfacc(XT)			asm("xxmfacc " #XT)
#define xvadddp(XT, XA, XB)		asm("xvadddp " #XT "," #XA "," #XB)
#define addi(RT, RA, SI)		asm("addi " #RT "," #RA "," #SI)

// innermost loop count
#define COUNT 1

#define load_B_packed_4x8(D)   \
    lxvp(32, 4,   0+D);        \
    lxvp(34, 4,  32+D);        \
    lxvp(36, 4,  64+D);        \
    lxvp(38, 4,  96+D);        \
    lxvp(40, 4, 128+D);        \
    lxvp(42, 4, 160+D);        \
    lxvp(44, 4, 192+D);        \
    lxvp(46, 4, 224+D);

#define load_A_packed_8x4(D)                    \
    lxvp(32, 3,   0+D); lxvp(34, 3,  32+D); \
    lxvp(36, 3,  64+D); lxvp(38, 3,  96+D); \
    lxvp(40, 3, 128+D); lxvp(42, 3, 160+D); \
    lxvp(44, 3, 192+D); lxvp(46, 3, 224+D);

#define rank_4_update_8x8_first() \
    xvf64ger(0, 32, 48);          \
    xvf64ger(1, 32, 49);          \
    xvf64ger(2, 32, 50);          \
    xvf64ger(3, 32, 51);          \
    xvf64ger(4, 34, 48);          \
    xvf64ger(5, 34, 49);          \
    xvf64ger(6, 34, 50);          \
    xvf64ger(7, 34, 51);          \
    xvf64gerpp(0, 36, 52);        \
    xvf64gerpp(1, 36, 53);        \
    xvf64gerpp(2, 36, 54);        \
    xvf64gerpp(3, 36, 55);        \
    xvf64gerpp(4, 38, 52);        \
    xvf64gerpp(5, 38, 53);        \
    xvf64gerpp(6, 38, 54);        \
    xvf64gerpp(7, 38, 55);        \
    xvf64gerpp(0, 40, 56);        \
    xvf64gerpp(1, 40, 57);        \
    xvf64gerpp(2, 40, 58);        \
    xvf64gerpp(3, 40, 59);        \
    xvf64gerpp(4, 42, 56);        \
    xvf64gerpp(5, 42, 57);        \
    xvf64gerpp(6, 42, 58);        \
    xvf64gerpp(7, 42, 59);        \
    xvf64gerpp(0, 44, 60);        \
    xvf64gerpp(1, 44, 61);        \
    xvf64gerpp(2, 44, 62);        \
    xvf64gerpp(3, 44, 63);        \
    xvf64gerpp(4, 46, 60);        \
    xvf64gerpp(5, 46, 61);        \
    xvf64gerpp(6, 46, 62);        \
    xvf64gerpp(7, 46, 63);    

#define rank_4_update_8x8() \
    xvf64gerpp(0, 32, 48); \
    xvf64gerpp(1, 32, 49); \
    xvf64gerpp(2, 32, 50); \
    xvf64gerpp(3, 32, 51); \
    xvf64gerpp(4, 34, 48); \
    xvf64gerpp(5, 34, 49); \
    xvf64gerpp(6, 34, 50); \
    xvf64gerpp(7, 34, 51); \
    xvf64gerpp(0, 36, 52); \
    xvf64gerpp(1, 36, 53); \
    xvf64gerpp(2, 36, 54); \
    xvf64gerpp(3, 36, 55); \
    xvf64gerpp(4, 38, 52); \
    xvf64gerpp(5, 38, 53); \
    xvf64gerpp(6, 38, 54); \
    xvf64gerpp(7, 38, 55); \
    xvf64gerpp(0, 40, 56); \
    xvf64gerpp(1, 40, 57); \
    xvf64gerpp(2, 40, 58); \
    xvf64gerpp(3, 40, 59); \
    xvf64gerpp(4, 42, 56); \
    xvf64gerpp(5, 42, 57); \
    xvf64gerpp(6, 42, 58); \
    xvf64gerpp(7, 42, 59); \
    xvf64gerpp(0, 44, 60); \
    xvf64gerpp(1, 44, 61); \
    xvf64gerpp(2, 44, 62); \
    xvf64gerpp(3, 44, 63); \
    xvf64gerpp(4, 46, 60); \
    xvf64gerpp(5, 46, 61); \
    xvf64gerpp(6, 46, 62); \
    xvf64gerpp(7, 46, 63);

#define add_and_store_8x8()    \
    xxmfacc(0);                \
    xxmfacc(1);                \
    xxmfacc(2);                \
    xxmfacc(3);                \
    xxmfacc(4);                \
    xxmfacc(5);                \
    xxmfacc(6);                \
    xxmfacc(7);                \
    lxvp(32+ 0, 5,   0);       \
    lxvp(32+ 2, 5,  32);       \
    lxvp(32+ 4, 5,  64);       \
    lxvp(32+ 6, 5,  96);       \
    lxvp(32+ 8, 5, 128);       \
    lxvp(32+10, 5, 160);       \
    lxvp(32+12, 5, 192);       \
    lxvp(32+14, 5, 224);       \
    lxvp(32+16, 5, 256);       \
    lxvp(32+18, 5, 288);       \
    lxvp(32+20, 5, 320);       \
    lxvp(32+22, 5, 352);       \
    lxvp(32+24, 5, 384);       \
    lxvp(32+26, 5, 416);       \
    lxvp(32+28, 5, 448);       \
    lxvp(32+30, 5, 480);       \
    xvadddp(32+ 0,  0, 32+ 0); \
    xvadddp(32+ 1,  1, 32+ 1); \
    xvadddp(32+ 2,  2, 32+ 2); \
    xvadddp(32+ 3,  3, 32+ 3); \
    xvadddp(32+ 4,  4, 32+ 4); \
    xvadddp(32+ 5,  5, 32+ 5); \
    xvadddp(32+ 6,  6, 32+ 6); \
    xvadddp(32+ 7,  7, 32+ 7); \
    xvadddp(32+ 8,  8, 32+ 8); \
    xvadddp(32+ 9,  9, 32+ 9); \
    xvadddp(32+10, 10, 32+10); \
    xvadddp(32+11, 11, 32+11); \
    xvadddp(32+12, 12, 32+12); \
    xvadddp(32+13, 13, 32+13); \
    xvadddp(32+14, 14, 32+14); \
    xvadddp(32+15, 15, 32+15); \
    xvadddp(32+16, 16, 32+16); \
    xvadddp(32+17, 17, 32+17); \
    xvadddp(32+18, 18, 32+18); \
    xvadddp(32+19, 19, 32+19); \
    xvadddp(32+20, 20, 32+20); \
    xvadddp(32+21, 21, 32+21); \
    xvadddp(32+22, 22, 32+22); \
    xvadddp(32+23, 23, 32+23); \
    xvadddp(32+24, 24, 32+24); \
    xvadddp(32+25, 25, 32+25); \
    xvadddp(32+26, 26, 32+26); \
    xvadddp(32+27, 27, 32+27); \
    xvadddp(32+28, 28, 32+28); \
    xvadddp(32+29, 29, 32+29); \
    xvadddp(32+30, 30, 32+30); \
    xvadddp(32+31, 31, 32+31); \
    stxvp(32+ 0, 5,   0);      \
    stxvp(32+ 2, 5,  32);      \
    stxvp(32+ 4, 5,  64);      \
    stxvp(32+ 6, 5,  96);      \
    stxvp(32+ 8, 5, 128);      \
    stxvp(32+10, 5, 160);      \
    stxvp(32+12, 5, 192);      \
    stxvp(32+14, 5, 224);      \
    stxvp(32+16, 5, 256);      \
    stxvp(32+18, 5, 288);      \
    stxvp(32+20, 5, 320);      \
    stxvp(32+22, 5, 352);      \
    stxvp(32+24, 5, 384);      \
    stxvp(32+26, 5, 416);      \
    stxvp(32+28, 5, 448);      \
    stxvp(32+30, 5, 480);

#define rank_8_update_8x8_first(D) \
    load_B_packed_4x8((D+0));      \
    load_A_packed_8x4((D+0));      \
    rank_4_update_8x8_first();     \
    load_B_packed_4x8((D+256));    \
    load_A_packed_8x4((D+256));    \
    rank_4_update_8x8();

#define rank_8_update_8x8(D)    \
    load_B_packed_4x8((D+0));   \
    load_A_packed_8x4((D+0));   \
    rank_4_update_8x8();        \
    load_B_packed_4x8((D+256)); \
    load_A_packed_8x4((D+256)); \
    rank_4_update_8x8();

#define packc_8x8()	    \
    lxvp( 0, 6,   0);       \
    lxvp( 2, 6,  32);       \
    lxvp( 4, 6,  64);       \
    lxvp( 6, 6,  96);       \
    lxvp( 8, 6, 128);       \
    lxvp(10, 6, 160);       \
    lxvp(12, 6, 192);       \
    lxvp(14, 6, 224);       \
    lxvp(16, 6, 256);       \
    lxvp(18, 6, 288);       \
    lxvp(20, 6, 320);       \
    lxvp(22, 6, 352);       \
    lxvp(24, 6, 384);       \
    lxvp(26, 6, 416);       \
    lxvp(28, 6, 448);       \
    lxvp(30, 6, 480);       \
    stxvp( 0, 5,   0);      \
    stxvp( 2, 5,  32);      \
    stxvp( 4, 5,  64);      \
    stxvp( 6, 5,  96);      \
    stxvp( 8, 5, 128);      \
    stxvp(10, 5, 160);      \
    stxvp(12, 5, 192);      \
    stxvp(14, 5, 224);      \
    stxvp(16, 5, 256);      \
    stxvp(18, 5, 288);      \
    stxvp(20, 5, 320);      \
    stxvp(22, 5, 352);      \
    stxvp(24, 5, 384);      \
    stxvp(26, 5, 416);      \
    stxvp(28, 5, 448);      \
    stxvp(30, 5, 480);

#define unpackc_8x8()	    \
    lxvp( 0, 5,   0);       \
    lxvp( 2, 5,  32);       \
    lxvp( 4, 5,  64);       \
    lxvp( 6, 5,  96);       \
    lxvp( 8, 5, 128);       \
    lxvp(10, 5, 160);       \
    lxvp(12, 5, 192);       \
    lxvp(14, 5, 224);       \
    lxvp(16, 5, 256);       \
    lxvp(18, 5, 288);       \
    lxvp(20, 5, 320);       \
    lxvp(22, 5, 352);       \
    lxvp(24, 5, 384);       \
    lxvp(26, 5, 416);       \
    lxvp(28, 5, 448);       \
    lxvp(30, 5, 480);       \
    stxvp( 0, 6,   0);      \
    stxvp( 2, 6,  32);      \
    stxvp( 4, 6,  64);      \
    stxvp( 6, 6,  96);      \
    stxvp( 8, 6, 128);      \
    stxvp(10, 6, 160);      \
    stxvp(12, 6, 192);      \
    stxvp(14, 6, 224);      \
    stxvp(16, 6, 256);      \
    stxvp(18, 6, 288);      \
    stxvp(20, 6, 320);      \
    stxvp(22, 6, 352);      \
    stxvp(24, 6, 384);      \
    stxvp(26, 6, 416);      \
    stxvp(28, 6, 448);      \
    stxvp(30, 6, 480);

void matmul_8x8x64_col_row_with_loads_and_stores
(
    double *A,
    double *B,
    double *C,
    double *D
)
{
    li(7, COUNT);
    asm("mtctr 7");
    asm("LOOP08:");

    rank_8_update_8x8_first(0);
    rank_8_update_8x8( 512);
    rank_8_update_8x8(1024);
    rank_8_update_8x8(1536);
    rank_8_update_8x8(2048);
    rank_8_update_8x8(2560);
    rank_8_update_8x8(3072);
    rank_8_update_8x8(3584);

    add_and_store_8x8();

    asm("bdnz LOOP08");
}

void matmul_8x8xK_col_row
(
    double *A,
    double *B,
    double *C,
    double *D,
    int     K
)
{
    asm("mtctr 7");
    asm("LOOP01:");

    rank_8_update_8x8_first(0);
    rank_8_update_8x8( 512);
    rank_8_update_8x8(1024);
    rank_8_update_8x8(1536);
    rank_8_update_8x8(2048);
    rank_8_update_8x8(2560);
    rank_8_update_8x8(3072);
    rank_8_update_8x8(3584);

    addi(3, 3, 4096);
    addi(4, 4, 4096);
    asm("bdnz LOOP01");

    add_and_store_8x8();
}

void matmul_8x8xK_col_row_packc
(
    double *A,
    double *B,
    double *C,
    double *D,
    int     K
)
{
    packc_8x8();

    asm("mtctr 7");
    asm("LOOP02:");

    rank_8_update_8x8_first(0);
    rank_8_update_8x8( 512);
    rank_8_update_8x8(1024);
    rank_8_update_8x8(1536);
    rank_8_update_8x8(2048);
    rank_8_update_8x8(2560);
    rank_8_update_8x8(3072);
    rank_8_update_8x8(3584);

    addi(3, 3, 4096);
    addi(4, 4, 4096);
    asm("bdnz LOOP02");

    add_and_store_8x8();

    unpackc_8x8();
}

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

double *A = 0;
double *B = 0;
double *C = 0;

double run_kernel
(
    void (kernel)(double*, double*, double*, double*, int),
    int k,
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
	kernel(A,B,C,D,k);
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
    void (kernel)(double*, double*, double*, double*, int),
    uint32_t count,
    const char* name,
    uint32_t M,
    uint32_t N,
    uint32_t K,
    uint32_t k
)
{
    assert(0 == K%k);

    volatile double elapsed;
    volatile double flops = 2.0*count*M*N*K;
    elapsed = run_kernel(kernel, K/k, count);
    std::cout << std::setprecision(6);
    std::cout << "Time to run " << std::setw(30) << name << " ( K=" << std::setw(4) << K << " ) " << count << " times = " << std::setw(10) << std::fixed << elapsed << " seconds ( " << std::setw(10) << std::scientific << flops/elapsed << " flops )" << std::endl;
}

#define RUN_KERNEL(kernel, count, M, N, K, k) run_kernel_and_report(kernel, count, #kernel, M, N, K, k)

int main
(
    int		argc,
    char      **argv
)
{
    const uint32_t matmul_8x8xK_col_row_count  =  10000000U;

    std::cout << "=================================================================================================================" << std::endl;

    for (int K=64; K<=512; K+=64)
	RUN_KERNEL(matmul_8x8xK_col_row      , matmul_8x8xK_col_row_count ,  8, 8, K, 64);
    for (int K=64; K<=512; K+=64)
	RUN_KERNEL(matmul_8x8xK_col_row_packc, matmul_8x8xK_col_row_count ,  8, 8, K, 64);

    return 0;
}

#include <stdint.h>
#include <time.h>
#include <iostream>

#define lxvp(XT, RA, D)			asm("lxvp " #XT "," #D "(" #RA ")")
#define xvf64gerpp(AT, XA, XB)		asm("xvf64gerpp " #AT "," #XA "," #XB)
#define xxpermdi(XT, XA, XB, IM)	asm("xxpermdi " #XT "," #XA "," #XB "," #IM)

void matmul_8x8x4_col_col
(
    double *A,
    double *B,
    double *C
)
{
    xxpermdi(48, 32, 34, 0b00);
    xxpermdi(52, 32, 34, 0b11);
    xxpermdi(49, 36, 38, 0b00);
    xxpermdi(53, 36, 38, 0b11);

    xxpermdi(50, 40, 42, 0b00);
    xxpermdi(54, 40, 42, 0b11);
    xxpermdi(51, 44, 46, 0b00);
    xxpermdi(55, 44, 46, 0b11);

    xxpermdi(56, 33, 35, 0b00);
    xxpermdi(60, 33, 35, 0b11);
    xxpermdi(57, 37, 39, 0b00);
    xxpermdi(61, 37, 39, 0b11);

    xxpermdi(58, 41, 43, 0b00);
    xxpermdi(62, 41, 43, 0b11);
    xxpermdi(59, 45, 47, 0b00);
    xxpermdi(63, 45, 47, 0b11);

    xvf64gerpp(0, 32, 48);
    xvf64gerpp(1, 32, 49);
    xvf64gerpp(2, 32, 50);
    xvf64gerpp(3, 32, 51);
    xvf64gerpp(4, 34, 48);
    xvf64gerpp(5, 34, 49);
    xvf64gerpp(6, 34, 50);
    xvf64gerpp(7, 34, 51);

    xvf64gerpp(0, 36, 52);
    xvf64gerpp(1, 36, 53);
    xvf64gerpp(2, 36, 54);
    xvf64gerpp(3, 36, 55);
    xvf64gerpp(4, 38, 52);
    xvf64gerpp(5, 38, 53);
    xvf64gerpp(6, 38, 54);
    xvf64gerpp(7, 38, 55);

    xvf64gerpp(0, 40, 56);
    xvf64gerpp(1, 40, 57);
    xvf64gerpp(2, 40, 58);
    xvf64gerpp(3, 40, 59);
    xvf64gerpp(4, 42, 56);
    xvf64gerpp(5, 42, 57);
    xvf64gerpp(6, 42, 58);
    xvf64gerpp(7, 42, 59);

    xvf64gerpp(0, 44, 60);
    xvf64gerpp(1, 44, 61);
    xvf64gerpp(2, 44, 62);
    xvf64gerpp(3, 44, 63);
    xvf64gerpp(4, 46, 60);
    xvf64gerpp(5, 46, 61);
    xvf64gerpp(6, 46, 62);
    xvf64gerpp(7, 46, 63);
}

void matmul_8x8x4_col_row
(
    double *A,
    double *B,
    double *C
)
{
    xvf64gerpp(0, 32, 48);
    xvf64gerpp(1, 32, 49);
    xvf64gerpp(2, 32, 50);
    xvf64gerpp(3, 32, 51);
    xvf64gerpp(4, 34, 48);
    xvf64gerpp(5, 34, 49);
    xvf64gerpp(6, 34, 50);
    xvf64gerpp(7, 34, 51);

    xvf64gerpp(0, 36, 52);
    xvf64gerpp(1, 36, 53);
    xvf64gerpp(2, 36, 54);
    xvf64gerpp(3, 36, 55);
    xvf64gerpp(4, 38, 52);
    xvf64gerpp(5, 38, 53);
    xvf64gerpp(6, 38, 54);
    xvf64gerpp(7, 38, 55);

    xvf64gerpp(0, 40, 56);
    xvf64gerpp(1, 40, 57);
    xvf64gerpp(2, 40, 58);
    xvf64gerpp(3, 40, 59);
    xvf64gerpp(4, 42, 56);
    xvf64gerpp(5, 42, 57);
    xvf64gerpp(6, 42, 58);
    xvf64gerpp(7, 42, 59);

    xvf64gerpp(0, 44, 60);
    xvf64gerpp(1, 44, 61);
    xvf64gerpp(2, 44, 62);
    xvf64gerpp(3, 44, 63);
    xvf64gerpp(4, 46, 60);
    xvf64gerpp(5, 46, 61);
    xvf64gerpp(6, 46, 62);
    xvf64gerpp(7, 46, 63);
}

void matmul_8x8x4_col_row_with_loads
(
    double *A,
    double *B,
    double *C
)
{
    lxvp(32, 3, 0); lxvp(34, 3, 32);
    lxvp(48, 4, 0); lxvp(50, 4, 32);
    xvf64gerpp(0, 32, 48);
    xvf64gerpp(1, 32, 49);
    xvf64gerpp(2, 32, 50);
    xvf64gerpp(3, 32, 51);
    xvf64gerpp(4, 34, 48);
    xvf64gerpp(5, 34, 49);
    xvf64gerpp(6, 34, 50);
    xvf64gerpp(7, 34, 51);

    lxvp(36, 3, 64); lxvp(38, 3, 96);
    lxvp(52, 4, 64); lxvp(54, 4, 96);
    xvf64gerpp(0, 36, 52);
    xvf64gerpp(1, 36, 53);
    xvf64gerpp(2, 36, 54);
    xvf64gerpp(3, 36, 55);
    xvf64gerpp(4, 38, 52);
    xvf64gerpp(5, 38, 53);
    xvf64gerpp(6, 38, 54);
    xvf64gerpp(7, 38, 55);

    lxvp(40, 3, 128); lxvp(56, 3, 160);
    lxvp(42, 4, 128); lxvp(58, 4, 160);
    xvf64gerpp(0, 40, 56);
    xvf64gerpp(1, 40, 57);
    xvf64gerpp(2, 40, 58);
    xvf64gerpp(3, 40, 59);
    xvf64gerpp(4, 42, 56);
    xvf64gerpp(5, 42, 57);
    xvf64gerpp(6, 42, 58);
    xvf64gerpp(7, 42, 59);

    lxvp(44, 3, 192); lxvp(60, 3, 224);
    lxvp(46, 4, 192); lxvp(62, 4, 224);
    xvf64gerpp(0, 44, 60);
    xvf64gerpp(1, 44, 61);
    xvf64gerpp(2, 44, 62);
    xvf64gerpp(3, 44, 63);
    xvf64gerpp(4, 46, 60);
    xvf64gerpp(5, 46, 61);
    xvf64gerpp(6, 46, 62);
    xvf64gerpp(7, 46, 63);
}

double now()
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
}

void run_kernel
(
    void (kernel)(double*, double*, double*),
    uint32_t count
)
{
    double *A = (double*)aligned_alloc(4096, 1024*1024);
    double *B = (double*)aligned_alloc(4096, 1024*1024);
    double *C = (double*)aligned_alloc(4096, 1024*1024);

    for(; count; count--)
    {
	kernel(A,B,C);
    }

    free(C);
    free(B);
    free(A);
}

int main
(
    int		argc,
    char      **argv
)
{
    const uint32_t matmul_8x8x4_col_row_count = 1000000000U;
    const uint32_t matmul_8x8x4_col_col_count = 1000000000U;

    volatile double start, finish;

    start = now();
    run_kernel(matmul_8x8x4_col_row_with_loads, matmul_8x8x4_col_row_count);
    finish = now();
    std::cout << "Time to run matmul_8x8x4_col_row_with_loads " << matmul_8x8x4_col_row_count << " times = " << (finish - start) << " seconds" << std::endl;

    start = now();
    run_kernel(matmul_8x8x4_col_row, matmul_8x8x4_col_row_count);
    finish = now();
    std::cout << "Time to run matmul_8x8x4_col_row " << matmul_8x8x4_col_row_count << " times = " << (finish - start) << " seconds" << std::endl;

    start = now();
    run_kernel(matmul_8x8x4_col_col, matmul_8x8x4_col_col_count);
    finish = now();
    std::cout << "Time to run matmul_8x8x4_col_col " << matmul_8x8x4_col_col_count << " times = " << (finish - start) << " seconds" << std::endl;

    return 0;
}

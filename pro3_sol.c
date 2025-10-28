#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <ctype.h>

// --- Problem 2 기반: 자료형 및 상수 정의 ---
#define SIGN_FIELD_BITS 1
#define EXPONENT_FIELD_BITS 9 
#define MAX_MANTISSA_DIGITS 512
#define FRACTION_FIELD_BITS (MAX_MANTISSA_DIGITS * 8) 
#define MY_TYPE_BITS (SIGN_FIELD_BITS + EXPONENT_FIELD_BITS + FRACTION_FIELD_BITS)
#define BIAS ((int)((1u << (EXPONENT_FIELD_BITS - 1)) - 1u)) // 255

typedef struct
{
    char digits[MAX_MANTISSA_DIGITS]; 
} frac_part;

typedef struct
{
    unsigned int sign : SIGN_FIELD_BITS;
    unsigned int exp : EXPONENT_FIELD_BITS; 
    frac_part frac;
} my_type;


/* ===== Problem 2 유틸리티 함수 (Problem 3 연산을 위해 필요) ===== */

// split: 숫자를 부호, 정수부, 소수부로 나눔
void split(const char *number, char *sign, char *int_part, char *frac_part) {
    if (number[0] == '+') { *sign = '+'; number++; }
    else if (number[0] == '-') { *sign = '-'; number++; }
    else { *sign = '+'; }

    int i = 0;
    while (*number != '\0' && *number != '.') {
        if (i < 255) int_part[i++] = *number;
        number++;
    }
    int_part[i] = '\0';

    int f = 0;
    if (*number == '.') {
        number++;
        while (*number != '\0') {
            if (f < 255) frac_part[f++] = *number;
            number++;
        }
        frac_part[f] = '\0';
    }
}

// init_type: 문자열을 my_type 구조체로 인코딩
void init_type(const char *number, my_type *t) {
    char sign_char;
    char int_part[256];
    char frac_part_str[256];

    split(number, &sign_char, int_part, frac_part_str);
    
    t->sign = (sign_char == '-') ? 1 : 0;
    
    // 지수 E = -(소수부 길이)
    int E = -strlen(frac_part_str);
    t->exp = E + BIAS; 

    // 가수 M = 정수부 + 소수부 문자열
    strcpy(t->frac.digits, int_part);
    strcat(t->frac.digits, frac_part_str);
}

// print_value: my_type 구조체 값을 사람이 읽는 10진 표기로 출력
void print_value(const my_type *t) {
    if (t->sign == 1) { printf("-"); }

    int E = (int)t->exp - BIAS;
    char *mantissa = t->frac.digits;
    int mantissa_len = strlen(mantissa);
    int frac_len = -E; 

    if (mantissa_len <= frac_len) {
        printf("0.");
        for (int i = 0; i < frac_len - mantissa_len; i++) { putchar('0'); }
        printf("%s", mantissa);
    } else {
        int int_len = mantissa_len - frac_len;
        
        for (int i = 0; i < int_len; i++) { putchar(mantissa[i]); }
        
        if (frac_len > 0) {
            putchar('.');
            printf("%s", &mantissa[int_len]);
        }
    }
    printf("\n");
}


// --- [Problem 3: High-Precision String Arithmetic Utilities] ---
// bigcmp, bigadd, bigsub_nonneg, strip_leading_zeros, append_zeros, align_to_common_exponent, normalize_result
// (제공해주신 로직 그대로 사용)

static void strip_leading_zeros(char *s) {
    size_t n = strlen(s);
    size_t k = 0;
    while (k + 1 < n && s[k] == '0') ++k;
    if (k) memmove(s, s + k, n - k + 1);
}

static int bigcmp(const char *a, const char *b) {
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return (la > lb) ? 1 : -1;
    int cmp = strcmp(a, b);
    return (cmp > 0) - (cmp < 0);
}

static void bigadd(const char *a, const char *b, char *out, size_t cap) {
    size_t la = strlen(a), lb = strlen(b);
    size_t i = la, j = lb, k = 0;
    char tmp[1024];
    int carry = 0;
    while (i > 0 || j > 0 || carry) {
        int da = (i > 0) ? (a[--i] - '0') : 0;
        int db = (j > 0) ? (b[--j] - '0') : 0;
        int s = da + db + carry;
        tmp[k++] = (char)('0' + (s % 10));
        carry = s / 10;
        if (k + 1 >= sizeof(tmp)) break; 
    }
    size_t need = k + 1;
    if (need > cap) { 
        size_t start = (need - cap);
        for (size_t t = 0; t < k - start; ++t) out[t] = tmp[k - 1 - (t + start)];
        out[k - start] = '\0';
        return;
    }
    for (size_t t = 0; t < k; ++t) out[t] = tmp[k - 1 - t];
    out[k] = '\0';
}

static void bigsub_nonneg(const char *A, const char *B, char *out) {
    size_t i = strlen(A), j = strlen(B);
    char tmp[1024];
    size_t k = 0;
    int borrow = 0;

    while (i > 0 || j > 0) {
        int da = (i > 0) ? (A[--i] - '0') : 0;
        int db = (j > 0) ? (B[--j] - '0') : 0;
        int s = da - borrow - db;
        if (s < 0) { s += 10; borrow = 1; } else borrow = 0;
        tmp[k++] = (char)('0' + s);
    }
    while (k > 1 && tmp[k - 1] == '0') --k; 
    for (size_t t = 0; t < k; ++t) out[t] = tmp[k - 1 - t];
    out[k] = '\0';
    strip_leading_zeros(out);
}

static void append_zeros(char *s, int cnt, size_t cap) {
    if (cnt <= 0) return;
    size_t len = strlen(s);
    if (len + cnt >= cap) cnt = (int)(cap - 1 - len);
    for (int i = 0; i < cnt; ++i) s[len + i] = '0';
    s[len + cnt] = '\0';
}

static int align_to_common_exponent(const my_type *a, const my_type *b,
                                    char *outA, char *outB, int *E_common,
                                    size_t cap_digits)
{
    int Ea = (int)a->exp - BIAS;
    int Eb = (int)b->exp - BIAS;
    int minE = (Ea < Eb) ? Ea : Eb;

    size_t La = strlen(a->frac.digits);
    size_t Lb = strlen(b->frac.digits);

    int lowerA = Ea - (int)((cap_digits - 1) - La);
    int lowerB = Eb - (int)((cap_digits - 1) - Lb);
    int lower_bound = (lowerA > lowerB) ? lowerA : lowerB;

    int Ec = (minE < lower_bound) ? lower_bound : minE;

    strncpy(outA, a->frac.digits, cap_digits - 1); outA[cap_digits - 1] = '\0';
    strncpy(outB, b->frac.digits, cap_digits - 1); outB[cap_digits - 1] = '\0';

    int za = Ea - Ec;
    int zb = Eb - Ec;
    append_zeros(outA, za, cap_digits);
    append_zeros(outB, zb, cap_digits);

    *E_common = Ec;
    return 0;
}

static void normalize_result(char *M, int *E) {
    size_t L = strlen(M);
    // 뒤쪽 0 제거: M의 길이가 1보다 클 때 (e.g., "0"을 "0"으로 유지)
    while (L > 1 && M[L - 1] == '0' && *E < BIAS) {
        M[L - 1] = '\0';
        --L;
        ++(*E);
    }
    // 가수 M이 "0"이 될 경우
    if (L == 0) {
        strcpy(M, "0");
        *E = 0;
        return;
    }
    strip_leading_zeros(M);
}


/* ===== 핵심: add / sub ===== */

void add(const my_type *a, const my_type *b, my_type *r)
{
    const size_t CAP = sizeof(r->frac.digits);
    char A[512], B[512], SUM[1024];
    int Ec = 0;

    // 1. 공통 지수 정렬
    align_to_common_exponent(a, b, A, B, &Ec, CAP);
    
    // 2. 부호 처리: 같은 부호 → 덧셈, 다른 부호 → 절대값 비교 후 뺄셈
    if (a->sign == b->sign) {
        // 같은 부호: 덧셈
        bigadd(A, B, SUM, sizeof(SUM));
        r->sign = a->sign;
        int E = Ec;
        
        // 3. 정규화 및 저장
        normalize_result(SUM, &E);
        if (E < -BIAS) E = -BIAS;
        if (E > BIAS) E = BIAS;
        r->exp = (unsigned int)(E + BIAS);
        
        strncpy(r->frac.digits, SUM, CAP - 1); r->frac.digits[CAP - 1] = '\0';
    } else {
        // 다른 부호: 뺄셈
        strip_leading_zeros(A);
        strip_leading_zeros(B);
        int cmp = bigcmp(A, B);
        
        if (cmp == 0) {
            // 결과 0
            r->sign = 0;
            r->exp = (unsigned int)BIAS; 
            r->frac.digits[0] = '0'; r->frac.digits[1] = '\0';
            return;
        }

        const char *P = (cmp > 0) ? A : B;
        const char *Q = (cmp > 0) ? B : A;
        char DIFF[1024];
        bigsub_nonneg(P, Q, DIFF);

        // 큰 쪽의 부호
        r->sign = (cmp > 0) ? a->sign : b->sign;
        int E = Ec;
        
        // 3. 정규화 및 저장
        normalize_result(DIFF, &E);
        if (E < -BIAS) E = -BIAS;
        if (E > BIAS) E = BIAS;
        r->exp = (unsigned int)(E + BIAS);
        
        strncpy(r->frac.digits, DIFF, CAP - 1); r->frac.digits[CAP - 1] = '\0';
    }
}

void sub(const my_type *a, const my_type *b, my_type *r)
{
    // a - b = a + (-b)
    my_type nb = *b;
    nb.sign = !b->sign;
    add(a, &nb, r);
}


// --- [Problem 3: Main Function] ---

// C 표준 main 함수 형식으로 변경 (int main)
int main(int argc, char *argv[]) 
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number1> <number2>\n", argv[0]);
        return 1;
    }

    printf("my_type (%d-bit)\n", MY_TYPE_BITS);
    printf(" Layout : [sign(%d) | exponent(%d) | fraction (%d)]\n",
           SIGN_FIELD_BITS, EXPONENT_FIELD_BITS, FRACTION_FIELD_BITS);
    printf(" BIAS = 2^(%d-1) - 1 = %d\n", EXPONENT_FIELD_BITS, BIAS);

    my_type x, y, result;
    
    init_type(argv[1], &x);
    init_type(argv[2], &y);

    printf("\n");
    printf("Computed Outcome:\n");
    
    // 덧셈 출력
    printf("%s + %s = ", argv[1], argv[2]);
    add(&x, &y, &result);
    print_value(&result);
    
    // 뺄셈 출력
    printf("%s - %s = ", argv[1], argv[2]);
    sub(&x, &y, &result);
    print_value(&result);
    
    return 0;
}
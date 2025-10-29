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


static void strip_leading_zeros(char *s) {
    // 기능: 숫자 문자열의 맨 앞에 불필요한 '0'을 제거 (예: "00123" -> "123")
    size_t n = strlen(s);
    size_t k = 0;
    // 문자열 길이가 1보다 크고, 현재 문자가 '0'인 동안 계속 건너뜀
    while (k + 1 < n && s[k] == '0') ++k;
    // 건너뛴 '0'이 있다면, 나머지 문자열을 앞으로 이동
    if (k) memmove(s, s + k, n - k + 1);
}

static int bigcmp(const char *a, const char *b) {
    // 기능: 두 가수 문자열의 절대값 크기를 비교
    // 반환: a > b 이면 1, a == b 이면 0, a < b 이면 -1
    
    // 1. 길이 비교 (길이가 다르면 긴 쪽이 큼)
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return (la > lb) ? 1 : -1;
    
    // 2. 길이가 같으면 문자열 사전순 비교
    int cmp = strcmp(a, b);
    // strcmp 결과(양수, 0, 음수)를 1, 0, -1로 변환
    return (cmp > 0) - (cmp < 0);
}

static void bigadd(const char *a, const char *b, char *out, size_t cap) {
    // 기능: 두 긴 숫자 문자열 a와 b를 덧셈하여 결과를 out에 저장
    
    size_t la = strlen(a), lb = strlen(b);
    size_t i = la, j = lb, k = 0;
    char tmp[1024]; // 덧셈 결과를 역순으로 임시 저장할 버퍼
    int carry = 0; // 자리 올림 수

    // 낮은 자리부터 시작 (문자열 끝에서부터)
    while (i > 0 || j > 0 || carry) {
        // 현재 자리의 숫자를 가져오고, 포인터를 이동 (--i, --j)
        int da = (i > 0) ? (a[--i] - '0') : 0;
        int db = (j > 0) ? (b[--j] - '0') : 0;
        int s = da + db + carry; // 현재 자리 합
        
        tmp[k++] = (char)('0' + (s % 10)); // 10으로 나눈 나머지를 문자('0'~'9')로 저장
        carry = s / 10; // 자리 올림 수 업데이트
        
        // 버퍼 오버플로우 안전장치
        if (k + 1 >= sizeof(tmp)) break; 
    }
    
    // 역순으로 저장된 결과를 out 버퍼로 정순 복사 (자릿수 제한 처리 포함)
    size_t need = k + 1;
    if (need > cap) { 
        // 결과가 cap을 초과하면 상위 자릿수를 잘라냄 (정밀도 손실 발생)
        size_t start = (need - cap);
        for (size_t t = 0; t < k - start; ++t) out[t] = tmp[k - 1 - (t + start)];
        out[k - start] = '\0';
        return;
    }
    // 일반적인 경우: 전체 결과를 정순으로 복사
    for (size_t t = 0; t < k; ++t) out[t] = tmp[k - 1 - t];
    out[k] = '\0';
}

static void bigsub_nonneg(const char *A, const char *B, char *out) {
    // 기능: A - B를 수행하여 결과를 out에 저장합니다. (A >= B 가정, 음수 결과 없음)
    // 주의: 자리 빌림(borrow)을 처리하는 고정밀도 뺄셈
    
    size_t i = strlen(A), j = strlen(B);
    char tmp[1024];
    size_t k = 0;
    int borrow = 0; // 자리 빌림 수

    // 낮은 자리부터 시작
    while (i > 0 || j > 0) {
        int da = (i > 0) ? (A[--i] - '0') : 0;
        int db = (j > 0) ? (B[--j] - '0') : 0;
        int s = da - borrow - db; // 현재 자리 차
        
        if (s < 0) { // 빌려와야 하는 경우
            s += 10; // 10을 더함
            borrow = 1; // 다음 자리에서 1 빌림
        } else borrow = 0;
        
        tmp[k++] = (char)('0' + s);
    }
    // 상위 불필요한 '0' 제거 후 정순 복사
    while (k > 1 && tmp[k - 1] == '0') --k; 
    for (size_t t = 0; t < k; ++t) out[t] = tmp[k - 1 - t];
    out[k] = '\0';
    strip_leading_zeros(out); // 최종 결과에서 선행 0 제거
}

static void append_zeros(char *s, int cnt, size_t cap) {
    // 기능: 숫자 문자열 s의 끝에 '0'을 cnt개 붙입니다. (소수점 이동을 위해 사용)
    if (cnt <= 0) return;
    size_t len = strlen(s);
    // cap을 초과하지 않도록 '0' 개수를 조정
    if (len + cnt >= cap) cnt = (int)(cap - 1 - len);
    for (int i = 0; i < cnt; ++i) s[len + i] = '0';
    s[len + cnt] = '\0';
}

static int align_to_common_exponent(const my_type *a, const my_type *b,
                                    char *outA, char *outB, int *E_common,
                                    size_t cap_digits)
{
    // 기능: 덧셈/뺄셈을 위해 두 피연산자 a, b의 가수(M)를 공통 지수 E_common에 맞춰 정렬
    // (지수 차이만큼 가수 문자열 뒤에 '0'을 추가)
    
    // 1. 실제 지수 E 계산 (E = exp - BIAS)
    int Ea = (int)a->exp - BIAS;
    int Eb = (int)b->exp - BIAS;
    int minE = (Ea < Eb) ? Ea : Eb; // 더 작은 지수 (가장 많은 '0'이 붙어야 함)

    size_t La = strlen(a->frac.digits);
    size_t Lb = strlen(b->frac.digits);

    // 2. 가수 용량(cap)을 초과하지 않도록 공통 지수 하한선 결정
    int lowerA = Ea - (int)((cap_digits - 1) - La);
    int lowerB = Eb - (int)((cap_digits - 1) - Lb);
    int lower_bound = (lowerA > lowerB) ? lowerA : lowerB;

    // 3. 최종 공통 지수 Ec 결정 (메모리 초과가 발생하지 않는 범위 내에서 가장 작은 E)
    int Ec = (minE < lower_bound) ? lower_bound : minE;

    // 4. 가수 문자열 복사 및 '0' 추가
    strncpy(outA, a->frac.digits, cap_digits - 1); outA[cap_digits - 1] = '\0';
    strncpy(outB, b->frac.digits, cap_digits - 1); outB[cap_digits - 1] = '\0';

    int za = Ea - Ec; // A에 붙일 '0' 개수
    int zb = Eb - Ec; // B에 붙일 '0' 개수
    append_zeros(outA, za, cap_digits);
    append_zeros(outB, zb, cap_digits);

    *E_common = Ec; // 공통 지수 반환
    return 0;
}

static void normalize_result(char *M, int *E) {
    // 기능: 연산 결과 가수 M을 정규화합니다.
    
    size_t L = strlen(M);
    // 1. 후행 0 제거 (E를 증가시키면서 M의 길이를 줄여 정규화)
    while (L > 1 && M[L - 1] == '0' && *E < BIAS) {
        M[L - 1] = '\0';
        --L;
        ++(*E);
    }
    
    // 2. 결과가 0일 경우
    if (L == 0) {
        strcpy(M, "0");
        *E = 0;
        return;
    }
    
    // 3. 선행 0 제거
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

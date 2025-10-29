#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h> // exit, malloc 등 추가 라이브러리 사용 시 포함

// [TODO] You need to decide the bit widths of the fields.
#define SIGN_FIELD_BITS 1
#define EXPONENT_FIELD_BITS 9 
// 9비트는 0~511을 표현 가능하며, 지수 E는 -255부터 +255까지
#define MAX_MANTISSA_DIGITS 512
#define FRACTION_FIELD_BITS (MAX_MANTISSA_DIGITS * 8) 
// sizeof는 바이트 단위 -> 8곱해서 비트 단위로
#define MY_TYPE_BITS (FRACTION_FIELD_BITS + EXPONENT_FIELD_BITS + SIGN_FIELD_BITS)

#define BIAS (int)(pow(2, EXPONENT_FIELD_BITS - 1) - 1)
// 9비트 지수의 BIAS 값은 2^(9-1) - 1 = 255 

typedef struct
{
    char digits[MAX_MANTISSA_DIGITS]; // 512 바이트 (최대 511자 + '\0')
} frac_part;

typedef struct
{
    unsigned int sign : SIGN_FIELD_BITS;
    unsigned int exp : EXPONENT_FIELD_BITS; // 9비트를 확보하기 위해 unsigned int 사용
    frac_part frac;
} my_type;

void split(const char *number, char *sign, char *int_part, char *frac_part)
{
    // 원본 코드와 동일 (부호, 정수부, 소수부 분리)
    if (number[0] == '+')
    {
        *sign = '+';
        number++;
    }
    else if (number[0] == '-')
    {
        *sign = '-';
        number++;
    }
    else
    {
        *sign = '+';
    } 

    // integral part
    int i = 0;
    while (*number != '\0' && *number != '.')
    {
        if (i < 255) int_part[i++] = *number; // 버퍼 오버플로우 방지
        number++;
    }
    int_part[i] = '\0';

    // fraction part
    int f = 0;
    if (*number == '.')
    {
        number++;
        while (*number != '\0')
        {
            if (f < 255) frac_part[f++] = *number; // 버퍼 오버플로우 방지
            number++;
        }
        frac_part[f] = '\0';
    }
}

void init_type(const char *number, my_type *t)
{
    char sign_char;
    char int_part[256];
    char frac_part_str[256];

    split(number, &sign_char, int_part, frac_part_str);
    
    // 1. sign field
    t->sign = (sign_char == '-') ? 1 : 0;

    // 2. exponent field
    // E는 소수부 문자열의 길이에 음수를 붙인 값 (10^-E)
    int E = -strlen(frac_part_str);
    t->exp = E + BIAS; // 계산된 E에 BIAS를 더해 저장 (9비트 기준: 0~511)

    // 3. fraction field
    // 정수부와 소수부 문자열을 하나로 합쳐서 저장
    strcpy(t->frac.digits, int_part);
    strcat(t->frac.digits, frac_part_str);
}

void decode_fields(my_type *t)
{
    printf("Decoded fields\n");

    // 1. sign 비트 출력
    printf("  - sign: ");
    putchar(t->sign ? '1' : '0');
    printf("\n");

    // 2. exponent 비트와 E 값 출력
    printf("  - exponent: ");
    unsigned int exp_val = t->exp;

    // EXPONENT_FIELD_BITS (9)에 맞게 비트 출력
    for (int i = EXPONENT_FIELD_BITS - 1; i >= 0; i--)
    {
        putchar(((exp_val >> i) & 1) ? '1' : '0');
    }
    printf(" (%u) => E = %d - %d = %d\n", t->exp, t->exp, BIAS, (int)t->exp - BIAS);

    // 3. fraction 필드(가수) 출력
    printf("  - fraction:\n");

    // 저장된 메모리 형태 (ASCII 이진수) 출력
    printf("    -> ASCII in Binary: ");
    char *frac_ptr = t->frac.digits;
    while (*frac_ptr)
    {
        unsigned char current_char = *frac_ptr;
        // 8비트씩 출력
        for (int i = 7; i >= 0; i--)
        {
            putchar(((current_char >> i) & 1) ? '1' : '0');
        }
        putchar(' ');
        frac_ptr++;
    }
    printf("\n");

    // 실제 값 (정확한 10진수 문자열) 출력
    printf("    -> Exact Decimal Value: %s\n", t->frac.digits);
}

void print_value(my_type *t)
{
    // 부호 출력
    if (t->sign == 1)
    {
        printf("-");
    }

    // 지수, 가수 가져옴
    int E = t->exp - BIAS;
    char *mantissa = t->frac.digits;
    int mantissa_len = strlen(mantissa);

    // 소수점 이동 횟수 (예: E=-2면 frac_len=2)
    int frac_len = -E; 

    // Case 1: 1보다 작은 소수 (예: 0.0123)
    // 가수 자릿수(123)보다 소수부 길이(3+1)가 더 길거나 같을 때
    if (mantissa_len <= frac_len)
    {
        printf("0.");
        // 소수점 아래 0의 개수 (frac_len - mantissa_len)만큼 0 출력
        for (int i = 0; i < frac_len - mantissa_len; i++)
        {
            putchar('0');
        }
        printf("%s", mantissa); // 가수 문자열 출력
    }
    // Case 2: 정수부와 소수부가 모두 있는 경우 (예: 123.45)
    else
    {
        int int_len = mantissa_len - frac_len;
        
        // 정수부 길이만큼 먼저 출력
        for (int i = 0; i < int_len; i++)
        {
            putchar(mantissa[i]);
        }
        
        // 소수부가 있다면 점(.)을 찍고 나머지 부분을 출력
        if (frac_len > 0)
        {
            putchar('.');
            printf("%s", &mantissa[int_len]);
        } 
        // 정수만 있는 경우 (frac_len = 0)
        else if (frac_len == 0)
        {
            // 정수만 출력됨 (점 없음)
        }
    }
    printf("\n");
}

// C 표준 main 함수 형식으로 변경 (int main)
int main(int argc, char *argv[]) 
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <number_to_encode>\n", argv[0]);
        return 1;
    }

    printf("my_type (%d-bit)\n", MY_TYPE_BITS);
    printf("  Layout  : [sign(%d) | exponent(%d) | fraction(%d)]\n",
           SIGN_FIELD_BITS, EXPONENT_FIELD_BITS, FRACTION_FIELD_BITS);

    printf("\n");
    printf("Value definition :\n");
    // 최종 디자인 공식: V = (-1)^sign * M * 10^E
    printf("  V = (-1)^sign * M * 10^E\n"); 
    printf("  E = exponent - BIAS\n");
    printf("  BIAS = 2^(%d-1) - 1 = %d\n", EXPONENT_FIELD_BITS, BIAS);
    printf("  M = (The integer value of all significant digits)\n");

    my_type t;
    // 데이터를 담을 공간 (메모리공간)
    init_type(argv[1], &t);

    printf("\n");
    decode_fields(&t);
    printf("\n");
    printf("Therefore, by my design:\n");
    printf("V = ");
    print_value(&t);
    
    return 0;
}

#include <string.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// [TODO] You need to decide the bit widths of the fields.
#define SIGN_FIELD_BITS 1
#define EXPONENT_FIELD_BITS 8
#define FRACTION_FIELD_BITS (sizeof(frac_part)*8)
// sizeof 는 바이트 단위이므로 비트 단위로 변환하려면 8을 곱해야 합니다.
#define MY_TYPE_BITS (FRACTION_FIELD_BITS+EXPONENT_FIELD_BITS+SIGN_FIELD_BITS)

#define BIAS (int)(pow(2, EXPONENT_FIELD_BITS-1) - 1)
// 127 이다.

typedef struct { 
    char digits[100];
    //문자열로 소수부분을 저장하는데, 최대 100자리까지 저장가능 
    /*
    [TODO] Define this here.
    */
} frac_part;

typedef struct {
    unsigned char sign : SIGN_FIELD_BITS; 
    unsigned char exp : EXPONENT_FIELD_BITS;    
    frac_part frac;
} my_type;



void split(const char *number, char *sign, char *int_part, char *frac_part) {
	if (number[0] == '+') {
		*sign = '+';
		number++;
	} else if (number[0] == '-') {
		*sign = '-';
		number++;
	} else {
		*sign = '+';
	}//일단 sign 의 값을 결정하고,
	
	
	// integral part
	int i = 0;
    while (*number != '\0' && *number != '.') {
		int_part[i++] = *number;
		number++;
	}
	
	int_part[i] = '\0';
	
	// fraction part
	int f = 0;
	if (*number == '.') {
		number++;
		while (*number != '\0') {
			frac_part[f++] = *number;
			number++;
		}
		
		frac_part[f] = '\0';
	}
    //정수부분과 소수부분을 점을 기준으로 문자열로 저장
}


void init_type(const char *number, my_type *t) {
    char sign_char;
    char int_part[256];
    char frac_part_str[256];
    
    split(number, &sign_char, int_part, frac_part_str);
    
    // 1. sign 필드 설정
    if (sign_char == '-') {
        t->sign = 1;
    } else {
        t->sign = 0;
    }
    
    // 2. exponent 필드 설정 (올바른 방식)
    // 실제 지수 E는 소수부 문자열의 길이에 음수를 붙인 값입니다.
    int E = -strlen(frac_part_str);
    t->exp = E + BIAS; // 계산된 E에 BIAS를 더해 저장합니다.
    
    // 3. fraction 필드 설정 (올바른 방식)
    // 정수부와 소수부 문자열을 하나로 합쳐서 저장합니다.
    strcpy(t->frac.digits, int_part);
    strcat(t->frac.digits, frac_part_str);
}


void decode_fields(my_type *t) {
    printf("Decoded fields\n"); 
    
    // [수정] 원시 메모리를 직접 읽는 대신, 구조체 멤버의 값을 기반으로 비트열을 출력합니다.
    // 이렇게 하면 컴파일러의 비트 필드 배치 방식이나 시스템의 엔디안에 영향을 받지 않아 안정적입니다.

    // 1. sign 비트 출력
    printf("  - sign: ");
    // t->sign 멤버의 값을 그대로 출력합니다.
    putchar(t->sign ? '1' : '0');
    printf("\n");

    // 2. exponent 비트 출력
    printf("  - exponent: ");
    unsigned int exp_val = t->exp;
    // t->exp 멤버의 값을 기반으로 8비트 이진수를 만듭니다.
    for (int i = EXPONENT_FIELD_BITS - 1; i >= 0; i--) {
        putchar(((exp_val >> i) & 1) ? '1' : '0');
    }
    printf(" (%u) => E = %d - %d = %d\n", t->exp, t->exp, BIAS, (int)t->exp - BIAS);

    // 3. [수정] fraction 필드(가수)를 아스키 코드의 이진수 형태로 출력합니다.
    printf("  - fraction (ASCII in binary): ");
    char *frac_ptr = t->frac.digits;
    // 문자열의 끝(\0)을 만날 때까지 반복
    while (*frac_ptr) {
        unsigned char current_char = *frac_ptr;
        // 각 문자를 8비트 이진수로 변환하여 출력
        for (int i = 7; i >= 0; i--) {
            putchar(((current_char >> i) & 1) ? '1' : '0');
        }
        putchar(' '); // 각 문자(바이트) 사이에 공백 추가
        frac_ptr++;
    }
    printf("\n");
}


void print_value(my_type *t) {
    // 부호 출력
    if (t->sign == 1) {
        printf("-");
    }

    // 저장된 데이터를 바탕으로 실제 지수(E)와 가수(mantissa)를 가져옵니다.
    int E = t->exp - BIAS;
    char *mantissa = t->frac.digits;
    int mantissa_len = strlen(mantissa);

    // 소수점 이하 자릿수는 -E와 같습니다.
    int frac_len = -E;

    // Case 1: 1보다 작은 소수 (예: 0.0123)
    if (mantissa_len <= frac_len) {
        printf("0.");
        // 소수점 아래에 필요한 0들을 먼저 출력합니다.
        for (int i = 0; i < frac_len - mantissa_len; i++) {
            putchar('0');
        }
        printf("%s", mantissa);
    }
    // Case 2: 정수부와 소수부가 모두 있는 경우 (예: 123.45)
    else {
        int int_len = mantissa_len - frac_len;
        // 정수부 길이만큼 먼저 출력합니다.
        for (int i = 0; i < int_len; i++) {
            putchar(mantissa[i]);
        }
        // 소수부가 있다면 점(.)을 찍고 나머지 부분을 출력합니다.
        if (frac_len > 0) {
            putchar('.');
            printf("%s", &mantissa[int_len]);
        }
    }
    printf("\n"); // 마지막에 줄바꿈 추가
}

void main(int argc, char **argv) {
	printf("my_type (%d-bit)\n", MY_TYPE_BITS);
	printf("  Layout  : [sign(%ld) | exponent(%ld) | fraction(%ld)]\n", 
	    SIGN_FIELD_BITS, EXPONENT_FIELD_BITS, FRACTION_FIELD_BITS);

    printf("\n");
    printf("Value definition :\n");
    printf("  V = (-1)^sign * M * 10^E\n"); // [TODO] Complete this line with your designed formula.
    printf("  E = exponent - BIAS\n");
    printf("  BIAS = 2^(%ld-1) - 1 = %ld\n", EXPONENT_FIELD_BITS, BIAS);
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
}
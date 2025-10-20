#include <stdio.h>
#include <string.h>
#include <math.h>

#define SIGN_FIELD_BITS 1
#define EXPONENT_FIELD_BITS 8
#define FRACTION_FIELD_BITS 23
#define TYPE_BITS (SIGN_FIELD_BITS+EXPONENT_FIELD_BITS+FRACTION_FIELD_BITS)

typedef struct {
   unsigned int frac : FRACTION_FIELD_BITS;
   unsigned int exp : EXPONENT_FIELD_BITS;
   unsigned int sign : SIGN_FIELD_BITS;
} float_like_type;


float to_float(float_like_type *flt) {
	float f;
	memcpy(&f, flt, sizeof(f));
	return f;
}

float get_largest_float() {
	float_like_type flt;
	
	flt.sign = 0; //양수 
	flt.exp = 254; //8비트에서의 최댓값
	flt.frac = 0x7FFFFF; //23비트에서 최대값 모두 1로 설정

	return to_float(&flt);
}

void main()
{
	float largest = get_largest_float();
	
	printf("The largest number: %f", largest);
}
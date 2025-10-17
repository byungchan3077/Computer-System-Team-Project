#include <string.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// [TODO] You need to decide the bit widths of the fields.
#define SIGN_FIELD_BITS ???
#define EXPONENT_FIELD_BITS ??? 
#define FRACTION_FIELD_BITS (sizeof(frac_part)*8)

#define MY_TYPE_BITS (FRACTION_FIELD_BITS+EXPONENT_FIELD_BITS+SIGN_FIELD_BITS)

#define BIAS (int)(pow(2, EXPONENT_FIELD_BITS-1) - 1)


typedef struct { 
    /*
    [TODO] Define this here.
    */
} frac_part;

typedef struct {
    unsigned char exp : EXPONENT_FIELD_BITS;    
    unsigned char sign : SIGN_FIELD_BITS;       
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
	}
	
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
}


void init_type(const char *number, my_type *t) {
    char sign;
    char int_part[256];
    char frac_part[256];
    
    split(number, &sign, int_part, frac_part);
    
    //printf("int_part: %s\n", int_part);
    //printf("frac_part: %s\n", frac_part);
    
    // sign field
    if (sign == '+') {
	    t->sign = 0;
	    number++;
	}
	else if (sign == '-') {
		t->sign = 1;
		number++;
	}
	
	// TODO: Implement the encoding logic for the exponent field and fraction field of my_type.
	
	// exponent field
	
	
	// fraction field
    
}


void decode_fields(my_type *t) {
	printf("Decoded fields\n"); 
	unsigned char *b = (unsigned char*)t;
    for (size_t i = 0; i < MY_TYPE_BITS; ++i) {
        size_t byte = i / 8;
        int    bit  = 7 - (int)(i % 8);          
        
        if (i == 0) {
	        printf("  - sign: ");
	    } else if (i == 1) {
			printf("  - exponent: ");  
	    } else if (i == SIGN_FIELD_BITS + EXPONENT_FIELD_BITS) {
	        printf("  - fraction: ");
        }
        putchar(((b[byte] >> bit) & 1) ? '1' : '0');
        
        if (i == 0) {
	        printf("\n");
        } else if (i == SIGN_FIELD_BITS + EXPONENT_FIELD_BITS -1) {
            printf(" (%d) => E = %d - %d = %d\n", t->exp, t->exp, BIAS, t->exp - BIAS);
        }
    }
    printf("\n");
}


void print_value(my_type *t) {
    // sign output
	if (t->sign == 1) {
		printf("-");
	} else {
		//printf("+");
	}

	// TODO: Print the value (e.g., 1111.2222)
}

void main(int argc, char **argv) {
	printf("my_type (%d-bit)\n", MY_TYPE_BITS);
	printf("  Layout  : [sign(%ld) | exponent(%ld) | fraction(%ld)]\n", 
	    SIGN_FIELD_BITS, EXPONENT_FIELD_BITS, FRACTION_FIELD_BITS);

    printf("\n");
    printf("Value definition :\n");
    printf("  V = (-1)^sign x \n"); // [TODO] Complete this line with your designed formula.
    printf("  E = exponent - BIAS\n");
    printf("  BIAS = 2^(%ld-1) - 1 = %ld\n", EXPONENT_FIELD_BITS, BIAS);
    printf("  M = \n");

	my_type t;
	init_type(argv[1], &t);   
	
	printf("\n");
	decode_fields(&t);
	printf("\n");
	printf("Therefore, by my design:\n");
	printf("V = ");
	print_value(&t);
}
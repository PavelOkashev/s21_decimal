#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define SIGN_MASK 0x80000000
#define TRUE 1
#define FALSE 0


enum returns { OK, INF, N_INF, N_NAN };

typedef struct {
    unsigned int bits[4];
} s21_decimal;

typedef union {
    unsigned int uns_int;
    float flo;
} soviet;


#define SUCCESS 0
#define CONVERTING_ERROR 1
int getbit(s21_decimal num, int bit);
void setbit(s21_decimal * num, int bit, int value);
int last_bit(s21_decimal number);
int offset_left(s21_decimal *varPtr, int value_offset);
void setsign(s21_decimal * num, int sign);
int getsign(s21_decimal num);
int get_scale(const s21_decimal *num);
void set_scale(s21_decimal *num, int scale);
void clearbits(s21_decimal * num);

int bit_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int bit_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal * result);
int bit_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal * result);
void bit_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result, s21_decimal *tmp);

int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal value_1, s21_decimal value_2);
int s21_is_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);
void scale_equlization(s21_decimal *value_1, s21_decimal *value_2);

int less_without_mod(s21_decimal value_1, s21_decimal value_2);
int is_equal_without_sign(s21_decimal value_1, s21_decimal  value_2);
void mul_ten(s21_decimal * num);
void div_ten(s21_decimal *num);
int getFloatExp(float *src);
int getFloatSign(float src);
int result_sign(s21_decimal value_1, s21_decimal value_2);
void bits_copy(s21_decimal src, s21_decimal *dest);


int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

int s21_negate(s21_decimal value, s21_decimal *result);
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);

#endif  // SRC_S21_DECIMAL_H_

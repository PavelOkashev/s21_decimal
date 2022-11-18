#include "s21_decimal.h"

int getbit(s21_decimal num, int bit) {
  int res = 0;
  if ((bit / 32) < 4) {
    unsigned int mask = 1u << (bit % 32);
    res = num.bits[bit / 32] & mask;
  }
  return !!res;
}

void clearbits(s21_decimal *num) {
  if (num != NULL) {
    memset(num->bits, 0, sizeof(num->bits));
  }
}

int last_bit(s21_decimal number) {
  int last_bit = 95;
  for (; last_bit >= 0 && getbit(number, last_bit) == 0; last_bit--) {
  }
  return last_bit;
}

void setbit(s21_decimal *num, int bit, int value) {
  unsigned int mask = 1u << (bit % 32);
  if ((bit / 32) < 4 && value == TRUE) {
    num->bits[bit / 32] |= mask;
  }
  if ((bit / 32) < 4 && value == FALSE) {
    num->bits[bit / 32] &= ~mask;
  }
}

int getsign(s21_decimal num) {
  unsigned int mask = 1u << 31;
  int res = num.bits[3] & mask;
  return !!res;
}

void setsign(s21_decimal *num, int sign) {
  unsigned int mask = 1u << 31;
  if (sign != 0) {
    num->bits[3] |= mask;
  } else {
    num->bits[3] &= ~mask;
  }
}

int get_scale(const s21_decimal *num) {
  char scale = num->bits[3] >> 16;
  return scale;
}

void set_scale(s21_decimal *num, int scale) {
  int clearMask = ~(0xFF << 16);
  num->bits[3] &= clearMask;
  int mask = scale << 16;
  num->bits[3] |= mask;
}

int bit_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  int tmp = 0;
  int i = 0;
  for (; i < 96; i++) {
    int cur_bit_val_1 = getbit(value_1, i);
    int cur_bit_val_2 = getbit(value_2, i);
    if (!cur_bit_val_1 && !cur_bit_val_2) {
      if (tmp == 1) {
        setbit(result, i, TRUE);
        tmp = 0;
      } else {
        setbit(result, i, FALSE);
      }
    } else if (cur_bit_val_1 != cur_bit_val_2) {
      if (tmp == 1) {
        setbit(result, i, FALSE);
        tmp = 1;
      } else {
        setbit(result, i, TRUE);
      }
    } else {
      if (tmp == 1) {
        setbit(result, i, TRUE);
        tmp = 1;
      } else {
        setbit(result, i, FALSE);
        tmp = 1;
      }
    }
    if (i == 95 && tmp == 1) return_value = INF;
  }
  if (return_value == INF) clearbits(result);
  return return_value;
}

int bit_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int tmp = 0;
  int last_b = 1 + last_bit(value_1);
  for (int i = 0; i < last_b; i++) {
    int cur_bit_val_1 = getbit(value_1, i);
    int cur_bit_val_2 = getbit(value_2, i);
    if (!cur_bit_val_1 && !cur_bit_val_2) {
      if (tmp) {
        tmp = 1;
        setbit(result, i, 1);
      } else {
        setbit(result, i, 0);
      }
    } else if (cur_bit_val_1 && !cur_bit_val_2) {
      if (tmp) {
        tmp = 0;
        setbit(result, i, 0);
      } else {
        setbit(result, i, 1);
      }
    } else if (!cur_bit_val_1 && cur_bit_val_2) {
      if (tmp) {
        tmp = 1;
        setbit(result, i, 0);
      } else {
        tmp = 1;
        setbit(result, i, 1);
      }
    } else if (cur_bit_val_1 && cur_bit_val_2) {
      if (tmp) {
        tmp = 1;
        setbit(result, i, 1);
      } else {
        setbit(result, i, 0);
      }
    }
  }
  return 0;
}

int bit_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  s21_decimal tmp;
  int lastBit = last_bit(value_1);
  for (int i = 0; i <= lastBit; i++) {
    clearbits(&tmp);
    int cur_val_1 = getbit(value_1, i);
    if (cur_val_1) {
      tmp = value_2;
      return_value = offset_left(&tmp, i);
      bit_add(*result, tmp, result);
    }
  }
  return return_value;
}

int offset_left(s21_decimal *num, int value_offset) {
  int return_value = OK;
  int lastbit = last_bit(*num);
  if (lastbit + value_offset > 95) {
    return_value = INF;
  } else {
    for (int i = 0; i < value_offset; i++) {
      int value_31bit = getbit(*num, 31);
      int value_63bit = getbit(*num, 63);
      num->bits[0] <<= 1;
      num->bits[1] <<= 1;
      num->bits[2] <<= 1;
      if (value_31bit) setbit(num, 32, 1);
      if (value_63bit) setbit(num, 64, 1);
    }
  }
  return return_value;
}

int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int return_value;
  s21_decimal zero = {{0, 0, 0, 0}};
  int sign_1 = getsign(value_1);
  int sign_2 = getsign(value_2);
  if (get_scale(&value_1) != get_scale(&value_2))
    scale_equlization(&value_1, &value_2);
  if (is_equal_without_sign(value_1, zero) == TRUE &&
      is_equal_without_sign(value_2, zero) == TRUE) {
    return_value = FALSE;
  } else if (sign_1 < sign_2) {
    return_value = FALSE;
  } else if (sign_1 > sign_2) {
    return_value = TRUE;
  } else {
    return_value = less_without_mod(value_1, value_2);
    if (sign_1 && sign_2 && return_value == TRUE) {
      return_value = FALSE;
    } else if (sign_1 && sign_2 && return_value == FALSE) {
      return_value = TRUE;
    }
  }
  return return_value;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  return s21_is_less(value_2, value_1);
}

int s21_is_equal(s21_decimal value_1, s21_decimal value_2) {
  int return_value;
  if (get_scale(&value_1) != get_scale(&value_2))
    scale_equlization(&value_1, &value_2);
  s21_decimal zero = {{0, 0, 0, 0}};
  if (is_equal_without_sign(zero, value_1) == TRUE &&
      is_equal_without_sign(zero, value_2) == TRUE) {
    return_value = TRUE;
  } else if (getsign(value_1) != getsign(value_2)) {
    return_value = FALSE;
  } else if (value_1.bits[2] != value_2.bits[2]) {
    return_value = FALSE;
  } else if (value_1.bits[1] != value_2.bits[1]) {
    return_value = FALSE;
  } else if (value_1.bits[0] != value_2.bits[0]) {
    return_value = FALSE;
  } else {
    return_value = TRUE;
  }
  return return_value;
}

int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  int return_value = FALSE;
  if (s21_is_equal(value_1, value_2) == 0) {
    return_value = TRUE;
  }
  return return_value;
}

int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int return_value = FALSE;
  if (s21_is_less(value_1, value_2) == TRUE ||
      s21_is_equal(value_1, value_2) == TRUE)
    return_value = TRUE;
  return return_value;
}

int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int return_value = FALSE;
  if (s21_is_greater(value_1, value_2) == TRUE ||
      s21_is_equal(value_1, value_2) == TRUE)
    return_value = TRUE;
  return return_value;
}

void scale_equlization(s21_decimal *value_1, s21_decimal *value_2) {
  if (get_scale(value_1) < get_scale(value_2)) {
    int difference = get_scale(value_2) - get_scale(value_1);
    if (getbit(*value_2, 93) == 0 && getbit(*value_2, 94) == 0 &&
        getbit(*value_2, 95) == 0) {
      for (int i = 0; i < difference; i++) {
        mul_ten(value_1);
      }
      set_scale(value_1, get_scale(value_2));
    } else {
      for (int i = 0; i < difference; i++) {
        div_ten(value_2);
      }
      set_scale(value_2, get_scale(value_1));
    }
  } else {
    int difference = get_scale(value_1) - get_scale(value_2);
    if (getbit(*value_1, 93) == 0 && getbit(*value_1, 94) == 0 &&
        getbit(*value_1, 95) == 0) {
      for (int i = 0; i < difference; i++) {
        mul_ten(value_2);
      }
      set_scale(value_2, get_scale(value_1));
    } else {
      for (int i = 0; i < difference; i++) {
        div_ten(value_1);
      }
      set_scale(value_1, get_scale(value_2));
    }
  }
}

void mul_ten(s21_decimal *num) {
  s21_decimal ten = {{10, 0, 0, 0}};
  bit_mul(*num, ten, num);
}

void div_ten(s21_decimal *num) {
  s21_decimal nothing;
  s21_decimal ten = {{10, 0, 0, 0}};
  bit_div(*num, ten, num, &nothing);
}

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  int sign_1 = getsign(value_1);
  int sign_2 = getsign(value_2);
  if (get_scale(&value_1) != get_scale(&value_2)) {
    scale_equlization(&value_1, &value_2);
  }
  if (sign_1 == sign_2) {
    if (sign_1 == 0) {  //                                                a + b
      return_value = bit_add(value_1, value_2, result);
      setsign(result, 0);
    } else if (sign_1 ==
               1) {  //                                         (-a) + (-b)
      if ((return_value = bit_add(value_1, value_2, result)) == INF)
        return_value = N_INF;
      setsign(result, 1);
    }
  } else if (sign_1 <
             sign_2) {  //                                        a + (-b)
    if (less_without_mod(value_1, value_2) == TRUE) {
      return_value = bit_sub(value_2, value_1, result);
      setsign(result, 1);
    } else {
      return_value = bit_sub(value_1, value_2, result);
      setsign(result, 0);
    }
  } else {  //                                         (-a) + b
    if (less_without_mod(value_1, value_2) == TRUE) {
      return_value = bit_sub(value_2, value_1, result);
      setsign(result, 0);
    } else {
      return_value = bit_sub(value_1, value_2, result);
      setsign(result, 1);
    }
  }
  set_scale(result, get_scale(&value_1));
  return return_value;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  int sign_1 = getsign(value_1);
  int sign_2 = getsign(value_2);
  int sign;
  if (get_scale(&value_1) != get_scale(&value_2)) {
    scale_equlization(&value_1, &value_2);
  }
  //    set_scale(result, get_scale(&value_1));
  if (sign_1 != sign_2) {
    sign = sign_1;
    setsign(&value_1, 0);
    setsign(&value_2, 0);
    return_value = s21_add(value_1, value_2, result);
    if (return_value == INF) {
      return_value = N_INF;
    } else {
      setsign(result, sign);
    }
  } else {
    s21_decimal *big_val, *small_val;
    setsign(&value_1, 0);
    setsign(&value_2, 0);
    if (s21_is_less(value_1, value_2)) {
      small_val = &value_1;
      big_val = &value_2;
      sign = !sign_2;
    } else {
      small_val = &value_2;
      big_val = &value_1;
      sign = sign_1;
    }
    bit_sub(*big_val, *small_val, result);
    set_scale(result, get_scale(&value_1));
    setsign(result, sign);
  }

  return return_value;
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  s21_decimal zero = {{0, 0, 0, 0}};
  int sign = result_sign(value_1, value_2);
  if (is_equal_without_sign(zero, value_2) == FALSE) {
    return_value = bit_mul(value_1, value_2, result);
    if (return_value != INF) {
      setsign(result, sign);
      int scale = get_scale(&value_1) + get_scale(&value_2);
      if (scale > 28) {
          return_value = INF;
          if (sign)
              return_value = N_INF;
      } else {
          set_scale(result, scale);
      }
    }
  }
  if (return_value == INF && sign == 1) return_value = N_INF;
  return return_value;
}

int less_without_mod(s21_decimal value_1, s21_decimal value_2) {
  int return_value;
  if (value_1.bits[2] < value_2.bits[2]) {
    return_value = TRUE;
  } else if (value_1.bits[2] > value_2.bits[2]) {
    return_value = FALSE;
  } else {
    if (value_1.bits[1] < value_2.bits[1]) {
      return_value = TRUE;
    } else if (value_1.bits[1] > value_2.bits[1]) {
      return_value = FALSE;
    } else {
      if (value_1.bits[0] < value_2.bits[0]) {
        return_value = TRUE;
      } else if (value_1.bits[0] > value_2.bits[0]) {
        return_value = FALSE;
      } else {
        return_value = FALSE;
      }
    }
  }
  return return_value;
}

int is_equal_without_sign(s21_decimal value_1, s21_decimal value_2) {
  int res;
  if (value_1.bits[2] != value_2.bits[2]) {
    res = FALSE;
  } else if (value_1.bits[1] != value_2.bits[1]) {
    res = FALSE;
  } else if (value_1.bits[0] != value_2.bits[0]) {
    res = FALSE;
  } else {
    res = TRUE;
  }
  return res;
}

void bit_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result,
             s21_decimal *tmp) {
  clearbits(result);
  clearbits(tmp);
  for (int i = last_bit(value_1); i >= 0; i--) {
    if (getbit(value_1, i)) setbit(tmp, 0, 1);
    if (s21_is_greater_or_equal(*tmp, value_2) == TRUE) {
      bit_sub(*tmp, value_2, tmp);
      if (i != 0) offset_left(tmp, 1);
      offset_left(result, 1);
      setbit(result, 0, 1);
    } else {
      offset_left(result, 1);
      if (i != 0) offset_left(tmp, 1);
      if ((i - 1) >= 0 && getbit(value_1, i - 1)) setbit(tmp, 0, 1);
    }
  }
}

int result_sign(s21_decimal value_1, s21_decimal value_2) {
  int sign_1 = getsign(value_1);
  int sign_2 = getsign(value_2);
  int sign;
  if (sign_1 == sign_2) {
    sign = 0;
  } else {
    sign = 1;
  }
  return sign;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  clearbits(result);
  s21_decimal zero = {{0, 0, 0, 0}};
  s21_decimal tmp = {{0, 0, 0, 0}};
  s21_decimal remember = {{0, 0, 0, 0}};
  int sign = result_sign(value_1, value_2);
  int return_value = OK;
  if (s21_is_equal(value_2, zero) == FALSE) {
    int scale = get_scale(&value_1) - get_scale(&value_2);
    set_scale(&value_1, 0);
    set_scale(&value_2, 0);
    setsign(&value_1, 0);
    setsign(&value_2, 0);
    bit_div(value_1, value_2, &tmp, &remember);
    bits_copy(tmp, result);
    s21_decimal max_value = {{4294967295u, 4294967295u, 4294967295u, 0}};
    set_scale(&max_value, 1);
    int iscale = 0;
    while (iscale < 28 && s21_is_equal(remember, zero) != TRUE) {
      if (s21_is_less(*result, max_value) != TRUE) break;
      mul_ten(&remember);
      bit_div(remember, value_2, &tmp, &remember);
      mul_ten(result);
      bit_add(*result, tmp, result);
      iscale++;
    }
    int new_scale = scale + iscale;
    while (new_scale > 28) {
      div_ten(result);
      new_scale--;
    }
    while (new_scale < 0) {
      mul_ten(result);
      new_scale++;
    }
    setsign(result, sign);
    set_scale(result, new_scale);
  } else {
    return_value = N_NAN;
    clearbits(result);
  }
  return return_value;
}

void bits_copy(s21_decimal src, s21_decimal *dest) {
  dest->bits[0] = src.bits[0];
  dest->bits[1] = src.bits[1];
  dest->bits[2] = src.bits[2];
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
    int return_value = OK;
    clearbits(result);
    int sign = getsign(value_1);
    int scale_1 = get_scale(&value_1);
    int scale_2 = get_scale(&value_2);
    int scale;
    if (scale_1 > scale_2) {
        scale = scale_1;
    } else {
        scale = scale_2;
    }
    s21_decimal mod = {{0, 0, 0, 0}};
    s21_decimal zero = {{0, 0, 0, 0}};
    if (s21_is_equal(zero, value_2)) {
        return_value = 3;
    } else if (scale > 28) {
        return_value = INF;
        if (sign)
            return_value = N_INF;
    } else {
            setsign(&value_1, 0);
            setsign(&value_2, 0);
            scale_equlization(&value_1, &value_2);
            set_scale(&value_1, 0);
            set_scale(&value_2, 0);
            bit_div(value_1, value_2, &zero, &mod);
            set_scale(&mod, scale);
            setsign(&mod, sign);
            *result = mod;
    }
    return return_value;
}

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  clearbits(dst);
  int res = OK;
  if (dst == NULL || isinf(src) || isnan(src)) {
    res = 1;
  } else {
    if (src < 0) {
      src *= -1;
      setsign(dst, 1);
    }
    dst->bits[0] = src;
  }
  return res;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int res = OK;
  int sign = getsign(src);
  int s = getbit(src, 31);
  if (src.bits[2] != 0 || src.bits[1] != 0 || s == 1 || dst == NULL) {
    res = 1;
  } else {
    *dst = 0;
    for (int i = get_scale(&src); i > 0; i--) {
      div_ten(&src);
    }
    *dst = src.bits[0];
    if (sign == 1) {
      *dst *= -1;
    }
  }
  return res;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  clearbits(dst);
  int result = 0;
  if (isinf(src) == 1 || isnan(src) == 1 || dst == NULL) {
    result = 1;
  } else {
    int sign = getFloatSign(src);
    int float_exp = getFloatExp(&src);
    float tmp = fabs(src);
    int powten = 0;
    while (powten < 28 && (int)tmp / (int)pow(2, 21) == 0) {
      tmp *= 10;
      powten++;
    }
    tmp = round(tmp);
    if (powten <= 28 && (float_exp > -94 && float_exp < 96)) {
      soviet mantissa;
      while (fmod(tmp, 10) == 0 && powten > 0) {
        powten--;
        tmp /= 10;
      }
      mantissa.flo = tmp;
      float_exp = getFloatExp(&mantissa.flo);
      dst->bits[float_exp / 32] |= 1 << float_exp % 32;
      for (int i = float_exp - 1, j = 22; j >= 0; i--, j--) {
        if ((mantissa.uns_int & (1 << j)) != 0) {
          dst->bits[i / 32] |= 1 << i % 32;
        }
      }
      setsign(dst, sign);
      set_scale(dst, powten);
    }
  }
  return result;
}

int getFloatSign(float src) {
  int res = 0;
  if (src < 0) {
    res = 1;
  }
  return res;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int res = 0;
  int scale = get_scale(&src);
  if (dst == NULL || scale > 28) {
    res = 1;
  } else {
    *dst = 0.0;
    double tmp = *dst;
    for (int i = 0; i < 96; i++) {
      if (getbit(src, i)) {
        tmp += pow(2, i);
      }
    }
    while (scale > 0) {
      tmp /= 10;
      scale--;
    }
    *dst = tmp;
    if (getsign(src)) {
      *dst *= -1;
    }
  }
  return res;
}

int getFloatExp(float *src) {
  int exp = ((*(int *)src & ~SIGN_MASK) >> 23) - 127;
  return exp;
}

int s21_negate(s21_decimal value, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  int sign = getsign(value);
  if (result == NULL) {
    return_value = 1;
  } else {
    if (!sign) {
      sign = 1;
    } else {
      sign = 0;
    }
    *result = value;
    setsign(result, sign);
  }
  return return_value;
}

int s21_round(s21_decimal value, s21_decimal *result) {
  clearbits(result);
  int res = OK;
  int sign = getsign(value);
  setsign(&value, 0);
  s21_decimal val_without_scale = {{0, 0, 0, 0}};
  s21_decimal five = {{5, 0, 0, 0}};
  set_scale(&five, 1);
  s21_decimal tmp = {{0, 0, 0, 0}};
  s21_decimal one = {{1, 0, 0, 0}};
  if (result == NULL) {
    res = 1;
  } else {
    s21_truncate(value, &val_without_scale);
    s21_sub(value, val_without_scale, &tmp);

    if (s21_is_greater_or_equal(tmp, five)) {
      s21_add(val_without_scale, one, result);
      set_scale(result, 0);
      setsign(result, sign);
    } else {
      *result = val_without_scale;
      set_scale(result, 0);
      setsign(result, sign);
    }
  }
  return res;
}

int s21_truncate(s21_decimal value, s21_decimal *result) {
  clearbits(result);
  int return_value = OK;
  int scale = get_scale(&value);
  int sign = getsign(value);
  if (result == NULL) {
    return_value = 1;
  } else {
    for (int i = scale; i > 0; i--) {
      div_ten(&value);
    }
    *result = value;
    set_scale(result, 0);
    setsign(result, sign);
  }
  return return_value;
}

int s21_floor(s21_decimal value, s21_decimal *result) {
  clearbits(result);
  int res = OK;
  int sign = getsign(value);
  s21_decimal tmp = {{1, 0, 0, 0}};
  int scale = get_scale(&value);
  if (result == NULL) {
    res = 1;
  } else {
    if (scale == 0) {
      *result = value;
    } else {
      s21_truncate(value, result);
      if (sign) {
        bit_add(*result, tmp, result);
        setsign(result, sign);
      }
    }
  }
  return res;
}

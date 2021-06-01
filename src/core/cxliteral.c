/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include <math.h>
#include <float.h>
#include "core/cxliteral.h"

#define _cxml_number__eq(a, b)  (fabs((a)-(b)) <= DBL_EPSILON * fmax(fabs((a)), fabs((b))))
#define _cxml_number__gt(a, b)  ((a)-(b) > (fmax(fabs((a)), fabs((b))) * DBL_EPSILON))
#define _cxml_number__lt(a, b)  ((b)-(a) > (fmax(fabs((a)), fabs((b))) * DBL_EPSILON))
#define _cxml_number__ne(a, b)  (!_cxml_number__eq(a, b))

#define __init_number(_num)                \
    (_num)->type = CXML_NUMERIC_NAN_T;     \
    (_num)->dec_val = 0;

// A number in xpath 1.0 represents a floating-point number
void cxml_number_init(cxml_number *literal){
    if (!literal) return;
    __init_number(literal)
}

cxml_number new_cxml_number(){
    cxml_number literal;
    __init_number(&literal)
    return literal;
}

static long _cxml_literal_r2l(const char *str, int base){
    char* endptr;
    return strtol(str, &endptr, base);
}

static double _cxml_literal_r2d(const char* str){
    char* endptr;
    return strtod(str, &endptr);
}

long cxml_literal_to_long(cxml_string *str){
    char* ch_str = cxml_string_as_raw(str);
    return _cxml_literal_r2l(ch_str, 10);
}

double cxml_literal_to_double(cxml_string *str){
    char* ch_str = cxml_string_as_raw(str);
    return _cxml_literal_r2d(ch_str);
}

void
cxml_set_literal(cxml_number* literal, cxml_literal_t literal_type, cxml_string *str){
    // coerce types to double
    switch(literal_type)
    {
        case CXML_XINTEGER_LITERAL:
            literal->type = CXML_NUMERIC_DOUBLE_T;
            literal->dec_val = (double)_cxml_literal_r2l(cxml_string_as_raw(str), 16);
            break;
        case CXML_INTEGER_LITERAL:
        case CXML_DOUBLE_LITERAL:
            literal->type = CXML_NUMERIC_DOUBLE_T;
            literal->dec_val = cxml_literal_to_double(str);
            break;
        default:
            break;
    }
}

static int
check_digits_or_hex(const char *curr, const char *end, int check){
    unsigned int digit_count = 0;
    size_t len = end - curr + 1;
    if (check == 1)  // check digit
    {
        while (isdigit((unsigned char)(*curr)))
        {
            curr++;
            digit_count++;
        }
    }
    else if (check == 2)   // check hex digit
    {
        while (isdigit((unsigned char)(*curr))
               || isxdigit((unsigned char)(*curr)))
        {
            curr++;
            digit_count++;
        }
        return digit_count && (digit_count == len) ? 2 : 0;
    }
    return digit_count && (digit_count == len);
}

void _cxml_clean_string(const char **current, const char **end) {

    while (isspace((unsigned char) (*(*current))) && (*current != *end)) (*current)++;
    if (*current == *end) return;
    while (isspace((unsigned char) (*(*end)))) (*end)--;

    if ((*(*current) == '"' && *(*end) == '"')
        || (*(*current) == '\'' && *(*end) == '\''))
    {
        (*current)++, (*end)--;
        while (isspace((unsigned char) (*(*current))) && (*current != *end)) (*current)++;
        if (*current == *end) return;
        while (isspace((unsigned char) (*(*end)))) (*end)--;
    }
}

/*
 *
===========================================
Number      :=  Sign?  Digit+
                | Sign? HexStart HexDigit+
Sign        :=  '+' | '-'
HexStart    :=  '0x' | '0X'
HexDigit    :=  [A-F] | [a-f] | Digit
Digit       :=  [0-9]
===========================================
 */

int _cxml_is_integer(const char* current, int len){
    const char* end = current + len - 1;
    _cxml_clean_string(&current, &end);
    if (*current == '+' || *current == '-') current++;
    if (*current == '0' && tolower((unsigned char)(*(current+1))) == 'x'){ // HexStart
        current += 2;
        return check_digits_or_hex(current, end, 2);
    }else if (isdigit((unsigned char)(*current))){  // Digit
        return check_digits_or_hex(current, end, 1);
    }
    return 0;
}

/*
==========================================================
Number          :=  Sign? Digit* DecimalPoint Digit+ Exp?
                    | Sign? Digit+ DecimalPoint Digit* Exp?
                    | Sign? Digit+ DecimalPoint? Exp

Number          :=  Sign? Digit* DecimalPoint Digit+ Exp?
Exp             :=  Exponent Sign? Digit+
Exponent        :=  'e' | 'E'
Sign            :=  '+' | '-'
DecimalPoint    :=  '.'
Digit           :=  [0-9]
==========================================================
 */

int _cxml_is_double(const char *current, int _len) {
    const char* end = current + _len - 1;
    _cxml_clean_string(&current, &end);
    unsigned int digit_count = 0, sign_count = 0;
    bool has_exp = 0, has_point = 0;
    size_t len = end - current + 1;
    if (*current == '+' || *current == '-') current++, sign_count++;

    // Sign? Digit+ DecimalPoint Digit* Exp?
    // Sign? Digit+ DecimalPoint? Exp
    if (isdigit((unsigned char)(*current)))
    {
        while (isdigit((unsigned char)(*current))){  // Digit+
            current++;
            digit_count++;
        }
        if (*current == '.')   // DecimalPoint
        {
            point:
            has_point = 1;
            current++;
            while (isdigit((unsigned char)(*current))) // Digit*
            {
                current++;
                digit_count++;
            }
            if (*current == 'e' || *current == 'E')  // Exp?
            {
                check:
                current++;
                has_exp = 1;
                if (*current == '+' || *current == '-') current++, sign_count++;
                if (isdigit((unsigned char)(*current)))
                {
                    while (isdigit((unsigned char)(*current))){ // Digit*
                        current++;
                        digit_count++;
                    }
                    return (digit_count + sign_count + has_exp + has_point) == len;
                }
                return 0;
            }
            return (digit_count + sign_count + has_exp + has_point) == len;
        }
        else if (*current == 'e' || *current == 'E')
        {
            goto check;
        }
        return 0;
    }
    // Sign? Digit* DecimalPoint Digit+ Exp?
    else if (*current == '.')
    {
        goto point;
    }
    return 0;
}

static cxml_number cxml_literal_raw_to_num(const char *str, int len){
    cxml_number num = new_cxml_number();
    const char* start = str;
    int ret;
    if ((ret = _cxml_is_integer(start, len))){
        num.type = CXML_NUMERIC_DOUBLE_T;
        if (ret == 1){
            num.dec_val = _cxml_literal_r2d(start);
        }else{
            num.dec_val = (double)_cxml_literal_r2l(start, 16);
        }
    }else if (_cxml_is_double(start, len)){
        num.type = CXML_NUMERIC_DOUBLE_T;
        num.dec_val = _cxml_literal_r2d(start);
    }
    return num;
}

cxml_number cxml_literal_to_num(cxml_string *str){
    const char* start = cxml_string_as_raw(str);
    if (!start) return new_cxml_number();
    return cxml_literal_raw_to_num(start, (int)cxml_string_len(str));
}

bool cxml_number_is_d_equal(double a, double b){
    return _cxml_number__eq(a, b);
}

bool cxml_number_is_equal(cxml_number *num1, cxml_number *num2){
    if ((num1->type == CXML_NUMERIC_NAN_T
            && num2->type == CXML_NUMERIC_NAN_T)
        || (num1->type != CXML_NUMERIC_NAN_T
            && num2->type == CXML_NUMERIC_NAN_T)
        || (num1->type == CXML_NUMERIC_NAN_T
            && num2->type != CXML_NUMERIC_NAN_T))
    {
        return false;
    }
    return _cxml_number__eq(num1->dec_val, num2->dec_val);
}

bool cxml_number_is_greater(cxml_number *num1, cxml_number *num2){
    if (num1->type == CXML_NUMERIC_NAN_T
        || num2->type == CXML_NUMERIC_NAN_T)
    {
        return false;
    }
    return _cxml_number__gt(num1->dec_val, num2->dec_val);
}

bool cxml_number_is_less(cxml_number *num1, cxml_number *num2){
    if (num1->type == CXML_NUMERIC_NAN_T
        || num2->type == CXML_NUMERIC_NAN_T)
    {
        return false;
    }
    return _cxml_number__lt(num1->dec_val, num2->dec_val);
}

bool cxml_number_is_not_equal(cxml_number *num1, cxml_number *num2){
    if ((num1->type == CXML_NUMERIC_NAN_T
            && num2->type == CXML_NUMERIC_NAN_T)
        || (num1->type != CXML_NUMERIC_NAN_T
            && num2->type == CXML_NUMERIC_NAN_T)
        || (num1->type == CXML_NUMERIC_NAN_T
            && num2->type != CXML_NUMERIC_NAN_T))
    {
        return true;
    }
    return _cxml_number__ne(num1->dec_val, num2->dec_val);
}

/*
 * Copyright © 2021 Jeremiah Ikosin
 * Distributed under the terms of the MIT license.
 */

#include "cxfixture.h"

// no seg-fault tests, because cxml_number is an internal structure with
// very precise use-case, and isn't meant to be used (directly) by external users.
// external use-cases are for value checking. If any of the conversion functions or other functions
// are to be used, care must be taken by the user, as none of the functions exposed adds extra safety.,
// or is written with extreme care to be used by external users.

void empty_literal_asserts(cxml_number *number){
    CHECK_EQ(number->type, CXML_NUMERIC_NAN_T);
    CHECK_EQ(number->dec_val, 0);
}

TEST(cxliteral, cxml_number_init){
    cxml_number number;
    cxml_number_init(&number);
    empty_literal_asserts(&number);
}

TEST(cxliteral, new_cxml_number){
    cxml_number number = new_cxml_number();
    empty_literal_asserts(&number);
}

TEST(cxliteral, cxml_literal_to_long){
    cxml_string str = new_cxml_string_s("123456");
    long number = cxml_literal_to_long(&str);
    CHECK_EQ(number, 123456);
    cxml_string_free(&str);

    str = new_cxml_string_s("1234.56");
    number = cxml_literal_to_long(&str);
    CHECK_EQ(number, 1234);
    cxml_string_free(&str);

    str = new_cxml_string_s("0xdeadbeef");
    number = cxml_literal_to_long(&str);
    CHECK_EQ(number, 0);
    cxml_string_free(&str);
}

TEST(cxliteral, cxml_literal_to_double){
    cxml_string str = new_cxml_string_s("1234.564");
    double number = cxml_literal_to_double(&str);
    CHECK_EQ(number, 1234.564);
    cxml_string_free(&str);

    str = new_cxml_string_s("1234.56b2");
    number = cxml_literal_to_double(&str);
    CHECK_EQ(number, 1234.56);
    cxml_string_free(&str);

    str = new_cxml_string_s("232323");
    number = cxml_literal_to_double(&str);
    CHECK_EQ(number, 232323);
    cxml_string_free(&str);

    str = new_cxml_string_s("see1234.56b2");
    number = cxml_literal_to_double(&str);
    CHECK_EQ(number, 0);
    cxml_string_free(&str);
}

TEST(cxliteral, cxml_set_literal){
    cxml_number number = new_cxml_number();
    CHECK_EQ(empty_literal_asserts(&number), 1);

    cxml_string str = new_cxml_string_s("121");
    cxml_set_literal(&number, 2000, &str);
    CHECK_EQ(empty_literal_asserts(&number), 1);

    cxml_set_literal(&number, CXML_INTEGER_LITERAL, &str);
    CHECK_EQ(number.dec_val, 121);
    cxml_string_free(&str);

    str = new_cxml_string_s("0xdeadbeef");
    cxml_set_literal(&number, CXML_XINTEGER_LITERAL, &str);
    CHECK_EQ(number.dec_val, 3735928559.0);
    cxml_string_free(&str);

    str = new_cxml_string_s("3.142");
    cxml_set_literal(&number, CXML_DOUBLE_LITERAL, &str);
    CHECK_EQ(number.dec_val, 3.142);
    cxml_string_free(&str);

}

TEST(cxliteral, _cxml_is_integer){
    CHECK_TRUE(_cxml_is_integer("234", 3));
    CHECK_TRUE(_cxml_is_integer("  \"234  \"  ", 11));
    CHECK_FALSE(_cxml_is_integer("  \"234", 6));
    CHECK_FALSE(_cxml_is_integer("  234 54  ", 10));
    CHECK_TRUE(_cxml_is_integer("  23454  ", 9));
    CHECK_EQ(_cxml_is_integer("0xdeadbeef", 10), 2);
    CHECK_EQ(_cxml_is_integer("0Xdeadbeef", 10), 2);
    CHECK_EQ(_cxml_is_integer("0xaceface", 9), 2);
    CHECK_FALSE(_cxml_is_integer("0xac eface", 10));
    CHECK_FALSE(_cxml_is_integer("1234.534", 8));
    CHECK_FALSE(_cxml_is_integer("1004e-5", 7));
}

TEST(cxliteral, _cxml_is_double){
    CHECK_FALSE(_cxml_is_double("234", 3));
    CHECK_FALSE(_cxml_is_double("0xdeadbeef", 10));
    CHECK_FALSE(_cxml_is_double("0xaceface", 9));
    CHECK_TRUE(_cxml_is_double("1234.534", 8));
    CHECK_TRUE(_cxml_is_double(" \r.534 \n", 8));
    CHECK_TRUE(_cxml_is_double("  1234.534   ", 13));
    CHECK_TRUE(_cxml_is_double("  \"1234.534\"   ", 15));
    CHECK_TRUE(_cxml_is_double("1004e-5  ", 9));
    CHECK_TRUE(_cxml_is_double("1.004e-5", 8));
    CHECK_FALSE(_cxml_is_double("1.00 4e-5", 9));
    CHECK_FALSE(_cxml_is_double("1 .004e-5", 9));
    CHECK_TRUE(_cxml_is_double("1.004e+5", 8));
    CHECK_FALSE(_cxml_is_double("1.004e-5.4", 10));
    CHECK_FALSE(_cxml_is_double("1.00.54", 7));
}

TEST(cxliteral, cxml_literal_to_num){
    cxml_string str = new_cxml_string_s("123.321");
    cxml_number number = cxml_literal_to_num(&str);
    CHECK_EQ(number.dec_val, 123.321);
    CHECK_EQ(number.type, CXML_NUMERIC_DOUBLE_T);
    cxml_string_free(&str);

    str = new_cxml_string_s("123e2");
    number = cxml_literal_to_num(&str);
    CHECK_EQ(number.dec_val, 12300);
    CHECK_EQ(number.type, CXML_NUMERIC_DOUBLE_T);
    cxml_string_free(&str);

    str = new_cxml_string_s("0xdeadbeef");
    number = cxml_literal_to_num(&str);
    CHECK_EQ(number.dec_val, 0xdeadbeef);
    CHECK_EQ(number.type, CXML_NUMERIC_DOUBLE_T);
    cxml_string_free(&str);

    str = new_cxml_string_s("32042");
    number = cxml_literal_to_num(&str);
    CHECK_EQ(number.dec_val, 32042);
    CHECK_EQ(number.type, CXML_NUMERIC_DOUBLE_T);
    cxml_string_free(&str);

}

TEST(cxliteral, cxml_number_is_d_equal){
    CHECK_TRUE(cxml_number_is_d_equal(1.234, 1.234));
    CHECK_TRUE(cxml_number_is_d_equal(1.234, 1.2340));
    CHECK_FALSE(cxml_number_is_d_equal(1.234, 12340));
    CHECK_FALSE(cxml_number_is_d_equal(0, 12340));
}

TEST(cxliteral, cxml_number_is_equal){
    cxml_number num1 = new_cxml_number(),
                num2 = new_cxml_number();

    num1.dec_val = 500;
    num2.dec_val = 500;
    // NaN test: both types are NaN
    CHECK_FALSE(cxml_number_is_equal(&num1, &num2));
    CHECK_FALSE(cxml_number_is_equal(&num1, &num1));

    // set type
    num1.type = CXML_NUMERIC_DOUBLE_T;
    num2.type = CXML_NUMERIC_DOUBLE_T;

    CHECK_TRUE(cxml_number_is_equal(&num1, &num2));
    CHECK_TRUE(cxml_number_is_equal(&num1, &num1));

    num1.dec_val = 13321e-4;
    num2.dec_val = 232.13;
    CHECK_FALSE(cxml_number_is_equal(&num1, &num2));

    num1.dec_val = 0;
    num2.dec_val = 0;
    CHECK_TRUE(cxml_number_is_equal(&num2, &num1));
    CHECK_TRUE(cxml_number_is_equal(&num2, &num2));
}

TEST(cxliteral, cxml_number_is_greater){
    cxml_number num1 = new_cxml_number(),
            num2 = new_cxml_number();

    num1.dec_val = 500;
    num2.dec_val = 32.86;
    // both types are NaN
    CHECK_FALSE(cxml_number_is_greater(&num1, &num2));
    CHECK_FALSE(cxml_number_is_greater(&num1, &num1));

    // set type
    num1.type = CXML_NUMERIC_DOUBLE_T;
    num2.type = CXML_NUMERIC_DOUBLE_T;

    CHECK_TRUE(cxml_number_is_greater(&num1, &num2));
    CHECK_FALSE(cxml_number_is_greater(&num1, &num1));

    num1.dec_val = 13321e-4;
    num2.dec_val = 232.13;
    CHECK_FALSE(cxml_number_is_greater(&num1, &num2));

    num1.dec_val = 0;
    num2.dec_val = 0;
    CHECK_FALSE(cxml_number_is_greater(&num2, &num1));
}

TEST(cxliteral, cxml_number_is_less){
    cxml_number num1 = new_cxml_number(),
            num2 = new_cxml_number();

    num1.dec_val = 500;
    num2.dec_val = 32.86;

    // both types are NaN
    CHECK_FALSE(cxml_number_is_less(&num1, &num2));
    CHECK_FALSE(cxml_number_is_less(&num1, &num1));

    // set type
    num1.type = CXML_NUMERIC_DOUBLE_T;
    num2.type = CXML_NUMERIC_DOUBLE_T;

    CHECK_FALSE(cxml_number_is_less(&num1, &num2));
    CHECK_TRUE(cxml_number_is_less(&num2, &num1));
    CHECK_FALSE(cxml_number_is_less(&num1, &num1));

    num1.dec_val = 13321e-4;
    num2.dec_val = 232.13;
    CHECK_TRUE(cxml_number_is_less(&num1, &num2));

    num1.dec_val = 0;
    num2.dec_val = 0;
    CHECK_FALSE(cxml_number_is_less(&num2, &num1));
}

TEST(cxliteral, cxml_number_is_not_equal){
    cxml_number num1 = new_cxml_number(),
            num2 = new_cxml_number();

    num1.dec_val = 500;
    num2.dec_val = 32.86;

    // both types are NaN
    CHECK_TRUE(cxml_number_is_not_equal(&num1, &num2));
    CHECK_TRUE(cxml_number_is_not_equal(&num1, &num1));
    CHECK_TRUE(cxml_number_is_not_equal(&num2, &num2));

    // set type
    num1.type = CXML_NUMERIC_DOUBLE_T;
    num2.type = CXML_NUMERIC_DOUBLE_T;

    CHECK_TRUE(cxml_number_is_not_equal(&num1, &num2));
    CHECK_FALSE(cxml_number_is_not_equal(&num1, &num1));
    CHECK_FALSE(cxml_number_is_not_equal(&num2, &num2));

    num1.dec_val = 13321e-4;
    num2.dec_val = 232.13;
    CHECK_TRUE(cxml_number_is_not_equal(&num1, &num2));

    num1.dec_val = 0;
    num2.dec_val = 0;
    CHECK_FALSE(cxml_number_is_not_equal(&num2, &num1));
}
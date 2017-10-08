/*
 * cgi_utilities.c
 *
 *  Created on: Sep 28, 2017
 *      Author: CNTHXIE
 */


// convert int to c type string.
// only fit int within 0~2^16
void int2cstr (int val, char *str_ary)
{
    char digit[5] = {0};
    char dec_order = 5;
    int i = 0;

    digit[0] = val/10000;
    digit[1] = (val-digit[0]*10000)/1000;
    digit[2] = (val-digit[0]*10000 - digit[1]*1000)/100;
    digit[3] = (val-digit[0]*10000-digit[1]*1000-digit[2]*100)/10;
    digit[4] = val-digit[0]*10000-digit[1]*1000-digit[2]*100-digit[3]*10;

    if (digit[0]) dec_order = 5;
    else if(digit[1]) dec_order = 4;
    else if(digit[2]) dec_order = 3;
    else if(digit[3]) dec_order = 2;
    //else if(digit[4]) dec_order = 1;
    else dec_order = 1;

    for (i = 0; i < dec_order; i++) {
        *(str_ary + i)  = (char) (digit[5-dec_order+i] + 0x30);
    }
    *(char *)(str_ary + dec_order)  = (char) 0;

}

// convert double to c type string
// only fit the double within 1.00~65535.99
void double2cstr (double val, char *str_ary)
{
    int d_int_part = 0;
    int d_dec_part2bit = 0;
    int dec_point_pos = 0;
    char dec[3] = {'.', 0, 0};

    d_int_part = floor(val);
    d_dec_part2bit = floor ((double )(val - d_int_part) * 100);
    dec[1] = d_dec_part2bit/10;
    dec[2] = d_dec_part2bit - dec[1]*10;

    dec[1] += 0x30;
    dec[2] += 0x30;

    int2cstr(d_int_part, str_ary);
    dec_point_pos = strlen(str_ary);
    memcpy((char*)(str_ary+dec_point_pos), dec, 3);

}

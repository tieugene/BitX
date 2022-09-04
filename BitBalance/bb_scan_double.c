#include <math.h>
#include <stdio.h>
#include "bb_scan_double.h"

_Bool bb_scan_double(char *new_text, double *pd)
{
    double d1, d2;
    _Bool b1, b2, res;
    char *p;

    for (p = new_text; *p; ++p)
        if ('.' == *p)
            *p = ',';
    b1 = (sscanf(new_text, "%lf", &d1) == 1);

    for (p = new_text; *p; ++p)
        if (',' == *p)
            *p = '.';
    b2 = (sscanf(new_text, "%lf", &d2) == 1);

    /*
        нет b1 и нет b2 -> 0.0      (!(b1 || b2) == !b1 && !b2)
        есть b1 и нет b2 -> d1
        есть b1 и есть b2 и (fabs(d1) > fabs(d2)) -> d1
        есть b1 и есть b2 и (fabs(d1) <= fabs(d2)) -> d2
        нет b1 и есть b2 -> d2
    */

    res = (b1 || b2);
    *pd = !res ? (double) 0 : ((b1 && (!b2 || (fabs(d1) > fabs(d2)))) ? d1 : d2);

    return res;
}
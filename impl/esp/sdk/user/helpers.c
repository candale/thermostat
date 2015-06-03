#include "helpers.h"


/*****************************************************************************/
/******************************** HELPERS ************************************/
/*****************************************************************************/


uint8 nstrcmp(char* first, char* second, unsigned short length) {
    if(os_strlen(first) < length || os_strlen(second) < length) {
        return -2;
    }
    unsigned short index = 0;
    while(index < length) {
        if(first[index] > second[index]) {
            return 1;
        } else if(first[index] < second[index]) {
            return -1;
        }
        index++;
    }

    return 0;
}

// Newton method for computing square root
double n_sqrt(double n)
{
    double x = 0.0;
    double xn = 0.0;
    int iters = 0;
    int i;
    for (i = 0; i <= (int)n; ++i)
    {
        double val = i*i-n;
        if (val == 0.0) {
            return i;
        }
        if (val > 0.0)
        {
            xn = (i + (i - 1)) / 2.0;
            break;
        }
    }
    while (!(iters++ >= 100 || x == xn))
    {
        x = xn;
        xn = x - (x * x - n) / (2 * x);
    }

  return xn;
}

double os_atof(const char* s)
{
    double rez = 0, fact = 1;
    while (*s && (*s == ' ' || *s == '\t' || *s == '\r' || *s == '\n'))
        s++;
    if (*s == 0)
        return 0; // can't read

    if (*s == '-')
    {
        s++;
        fact = -1;
    };
    int point_seen;
    for (point_seen = 0; *s; s++)
    {
        if (*s == '.')
        {
            point_seen = 1;
            continue;
        };
        int d = *s - '0';
        if (d >= 0 && d <= 9)
        {
            if (point_seen)
                fact /= 10.0;
            rez = rez * 10.0f + (double) d;
        };
    };
    return rez * fact;
}

double os_pow(int x, int y) {
    int i;
    for(i = 1; i < y; i++) {
        x = x * x;
    }
    return x;
}

void ftoa(double val, char *buff) {
    char smallBuff[16];
    char smallBuff_tmp[16];
    int val1 = (int) val;
    int val2 = (int) (100.0 * val) % 100;
    unsigned int uval2;
    if (val < 0) {
        uval2 = (int) (-100.0 * val) % 100;
    } else {
        uval2 = (int) (100.0 * val) % 100;
    }
    if (uval2 < 10) {
        os_sprintf(smallBuff, "%i.0%u", val1, uval2);
    } else {
        os_sprintf(smallBuff, "%i.%u", val1, uval2);
    }

    if(val2 < 0 && val1 == 0) {
        os_sprintf(smallBuff_tmp, "-%s", smallBuff);
        strcat(buff, smallBuff_tmp);
    } else {
        strcat(buff, smallBuff);
    }
}

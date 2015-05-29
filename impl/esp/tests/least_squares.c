#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>

//#define float float
#define float double

inline static float sqr(float x) {
    return x*x;
}


int linreg(int n, const float y[], float* m, float* b, float* r)
{
    float sumx = 0.0;                        /* sum of x                      */
    float sumx2 = 0.0;                       /* sum of x**2                   */
    float sumxy = 0.0;                       /* sum of x * y                  */
    float sumy = 0.0;                        /* sum of y                      */
    float sumy2 = 0.0;                       /* sum of y**2                   */

    int i;
    for (i = 0; i < n; i++)
    {
        sumx  += i;
        sumx2 += sqr(i);
        sumxy += i * y[i];
        sumy  += y[i];
        sumy2 += sqr(y[i]);
    }

    float denom = (n * sumx2 - sqr(sumx));
    if (denom == 0) {
           // singular matrix. can't solve the problem.
        *m = 0;
        *b = 0;
        *r = 0;
        return 1;
    }

    *m = (n * sumxy  -  sumx * sumy) / denom;
    *b = (sumy * sumx2  -  sumx * sumxy) / denom;
    if (r!=NULL) {
        *r = (sumxy - sumx * sumy / n) /
            (sqrt((sumx2 - sqr(sumx) / n) * (sumy2 - sqr(sumy) / n)));
    }

    return 0;
}

void get_float(float f, int decimal_places, char* buffer)
{
    char buf[50];
    int no = (int)(f * pow(10, decimal_places));
    sprintf(buf, "%d", no);
    printf("%s\n", buf);
    int dot_place = strlen(buf) - decimal_places;
    int i = 0;
    char c, aux_c;
    int len = strlen(buf) + 1;
    while(i < len) {
        if(i == dot_place) {
            c = buf[i];
            buf[i] = '.';
        }
        if(i > dot_place) {
            aux_c = buf[i];
            buf[i] = c;
            c = aux_c;
        }
        i++;
    }
    strcat(buffer, buf);
}

int main() {
    float x[] = {22.10, 25.88, 23.50, 23.19, 23.19, 23.10, 23.00, 23.00, 23.00, 23.00};
    float m, b, r;
    linreg(10, x, &m, &b, &r);
    printf("%f\n", m);
    return 0;
}

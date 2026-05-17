/**@file ECEF_to_geodetic.c
 *
 * @brief Reconstruction of Karl Osen's Algorithm for ECEF
 *  to geodetic coordinate conversion.
 *
 * This file is a modified version of the software provided in this paper:
 *      Karl Osen. Accurate Conversion of Earth-Fixed Earth-Centered
 *      Coordinates to Geodetic Coordinates. [Research Report]
 *      Norwegian University of Science and Technology. 2017.
 *      hal-01704943v2f
 *      https://hal.science/hal-01704943v2/document
 *
 * Modifications made to the original algorithm in this file include:
 *  consolidating the algorithm into a single .h and .c file,
 *  unpacking the structs used as parameters into raw doubles,
 *  removing the choice to return geodetic coords in radians,
 *  and removing unused WGS84 macros and replacing all
 *      constant integers with their macro counterparts.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 * @date 9/3/2023
 */

#include <math.h>
#include "ECEF_to_geodetic.h"


//WGS84 Constants
#define WGS84_INVAA     +2.45817225764733181057e-0014 /* 1/(a^2) */
#define WGS84_EED2      +3.34718999507065852867e-0003 /* (e^2)/2 */
#define WGS84_P1MEE     +9.93305620009858682943e-0001 /* 1-(e^2) */
#define WGS84_P1MEEDAA  +2.44171631847341700642e-0014 /* (1-(e^2))/(a^2) */
#define WGS84_EEEE      +4.48147234524044602618e-0005 /* e^4 */
#define WGS84_EEEED4    +1.12036808631011150655e-0005 /* (e^4)/4 */
#define WGS84_HMIN      +2.25010182030430273673e-0014 /* (e^12)/4 */
#define WGS84_INVCBRT2  +7.93700525984099737380e-0001 /* 1/(2^(1/3)) */
#define WGS84_INV3      +3.33333333333333333333e-0001 /* 1/3 */
#define WGS84_INV6      +1.66666666666666666667e-0001 /* 1/6 */
#define WGS84_R2D       +5.72957795130823208766e+0001 /* 180/pi */


int wgs84EcefToGeo(
    double x,
    double y,
    double z,
    double *longitude,
    double *latitude,
    double *altitude
) {
    // The variables below correspond to symbols used in the paper
    // "Accurate Conversion of Earth-Centered, Earth-Fixed Coordinates
    // to Geodetic Coordinates"
    double beta;
    double C;
    double dFdt;
    double dt;
    double dw;
    double dz;
    double F;
    double G;
    double H;
    double i;
    double k;
    double m;
    double n;
    double p;
    double P;
    double t;
    double u;
    double v;
    double w;

    // Intermediate variables
    double j;
    double ww; // w^2
    double mpn; // m+n
    double g;
    double tt; // t^2
    double ttt; // t^3
    double tttt; // t^4
    double zu; // z * u
    double wv; // w * v
    double invuv; // 1 / (u * v)
    double da;
    double t1, t2, t3, t4, t5, t6, t7;


    ww = x * x + y * y;
    m = ww * WGS84_INVAA;
    n = z * z * WGS84_P1MEEDAA;
    mpn = m + n;
    p = WGS84_INV6 * (mpn - WGS84_EEEE);
    G = m * n * WGS84_EEEED4;
    H = 2 * p * p * p + G;

    if (H < WGS84_HMIN) return -1;
    
    C = pow(H + G + 2 * sqrt(H * G), WGS84_INV3) * WGS84_INVCBRT2;
    i = -WGS84_EEEED4 - 0.5 * mpn;
    P = p * p;
    beta = WGS84_INV3 * i - C - P / C;
    k = WGS84_EEEED4 * (WGS84_EEEED4 - mpn);

    // Compute left part of t
    t1 = beta * beta - k;
    t2 = sqrt(t1);
    t3 = t2 - 0.5 * (beta + i);
    t4 = sqrt(t3);

    // Compute right part of t
    t5 = 0.5 * (beta - i);

    // t5 may accidentally drop just below zero due to numeric turbulence
    // This only occurs at latitudes close to +- 45.3 degrees
    t5 = fabs(t5);
    t6 = sqrt(t5);
    t7 = (m < n) ? t6 : -t6;

    // Add left and right parts
    t = t4 + t7;

    // Use Newton-Raphson's method to compute t correction
    j = WGS84_EED2 * (m - n);
    g = 2 * j;
    tt = t * t;
    ttt = tt * t;
    tttt = tt * tt;
    F = tttt + 2 * i * tt + g * t + k;
    dFdt = 4 * ttt + 4 * i * t + g;
    dt = -F / dFdt;

    // compute latitude (range -PI/2..PI/2)
    u = t + dt + WGS84_EED2;
    v = t + dt - WGS84_EED2;
    w = sqrt(ww);
    zu = z * u;
    wv = w * v;
    double lat = atan2(zu, wv);

    // compute altitude
    invuv = 1 / (u * v);
    dw = w - wv * invuv;
    dz = z - zu * WGS84_P1MEE * invuv;
    da = sqrt(dw * dw + dz * dz);
    double alt = (u < 1) ? -da : da;

    //compute longitude (range -PI..PI)
    double lon = atan2(y, x);

    *longitude = WGS84_R2D * lon;
    *latitude  = WGS84_R2D * lat;
    *altitude  = alt;

    return 0;
}






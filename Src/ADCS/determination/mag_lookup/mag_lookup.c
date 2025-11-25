#include "determination/mag_lookup/mag_lookup.h"
#include "adcs_math/vector.h"


// Gauss coefficients
float g[IGRF_GEN][IGRF_GEN + 1];
float h[IGRF_GEN][IGRF_GEN + 1];

float decimal_years = 0;    // Decimal days since January 1, IGRF_START_YEAR

float igrf_B_ned[3] = {0.0, 0.0, 0.0};
float igrf_B_sph[3] = {0.0, 0.0, 0.0};

// Schmidt quasi-normalized coefficients
float g_val[104] = {
    -29405, -1450.9, -3749.4, 5165, 1452.3, 3408, -7290.9, 2393.9,
    415.6, 3950.6, 4479.8, 337.7, -647.16, 35.496, -1845.1, 3692.5,
    1443.3, -662.16, -335.44, 9.4711, 952.88, 1238.2, 1089.4, -1210.5,
    -197.54, 31.412, -43.459, 2161.1, -2720.5, -237.48, 1157, 195.11,
    39.517, -17.437, 6.3431, 1191.5, 650.2, -987.05, -20.71, -564.13,
    226.91, 94.054, -41.363, -0.18801, 474.8, 1070.2, 315.09, -124.47,
    -62.013, -444.72, 19.138, 66.295, -24.031, -7.2477, -342.81, -1508.4,
    -21.069, 280.98, -105.18, 51.741, -37.188, 38.082, 11.456, -6.3715,
    -2.2558, 1033.3, -652.94, -1022.6, 754.33, -215.56, 47.527, -65.882,
    -4.9604, 31.864, -5.289, 0.54407, 1.7979, -1320.4, -89.703, 397.56,
    843.99, -584.3, 233.81, 62.49, 58.527, -17.558, -12.772, 0.94325,
    -3.0596, -0.17033, 126.96, -1557.2, 773.76, 898.19, -295.24, 556.7,
    0, 213.71, 0, 27.229, 2.8388, 5.0184, -1.4194, -0.2227};

float g_svi[104] = {
    5.7, 7.4, -16.5, -12.124, -1.8187, 5.5, -18.065, 6.0031,
    -9.4868, -5.25, -8.8544, -23.087, 10.877, -3.7715, -2.3625, 5.0833,
    -4.6111, 0.94124, 2.8841, 0.6314, -7.2188, -5.6709, 5.9777, 12.952,
    -7.6396, 0, 0.60452, -2.6813, -7.0939, 0, 14.335, 1.2349,
    -3.0872, -1.9375, 0.51781, 0, 6.7031, -5.6082, 16.568, -2.6736,
    5.9322, 2.0596, -0.25068, 0.25068, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

float h_val[104] = {
    0, 4652.5, 0, -5181.6, -636.18, 0, -251.38, 468.44,
    -429.6, 0, 1560, -619.84, 417.7, -258.61, 0, 484.95,
    1600.8, -570.39, 71.659, 69.384, 0, -361.05, 375.1, 526.04,
    -351.97, 20.709, 45.742, 0, -1826.7, -489.44, 45.052, 290.2,
    -13.584, -65.874, -1.1651, 0, 563.06, -858.06, 530.17, -312.81,
    220.98, 24.715, -17.297, 1.7548, 0, -2981.2, 1195.2, 813.23,
    -287.52, -212.25, 135.7, 3.0134, -3.6176, 5.8469, 0, 827.17,
    -42.138, 595.01, 560.98, -635.67, -4.132, -86.186, -27.821, -0.26548,
    -5.2239, 0, 0, 1022.6, -196.78, -95.806, 95.054, -18.824,
    -84.327, -36.416, -26.445, -5.4407, -1.5079, 0, -1076.4, 397.56,
    908.91, -876.45, 33.402, 166.64, -23.411, 35.116, 5.1087, -8.4892,
    0, 0.28388, 0, -1557.2, 928.51, 1796.4, -393.65, -904.64,
    -45.155, 80.141, -14.279, 34.036, 14.194, -4.0147, -1.1355, -0.33405};

float h_svi[104] = {
    0, -25.9, 0, -52.308, -19.399, 0, 18.371, -2.1301,
    0.39528, 0, -0.5534, 25.435, 7.5299, -3.6975, 0, 0,
    19.213, -2.8237, 6.6556, 0.21047, 0, 0, -23.911, -12.952,
    4.3655, 0, 0.67169, 0, 21.282, 17.376, -16.383, -2.4698,
    -6.7919, 0.24218, 0.19418, 0, -13.406, 33.649, -8.2839, 13.368,
    -4.4492, -2.7461, 1.2534, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};


//-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-//
//+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+//

void compute_gauss_coeffs(float decimal_years_) {
    /*
     * this part will require some work, to embed functions that 
     * extract values from an Intellisat defined buffer.
     * 
     * this applies to g_val , g_svi , h_val , h_svi
    */

    uint8_t k = 0;
    for (int i = 1; i <= IGRF_GEN; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            //  std::cout << i << j << std::endl;
            g[i - 1][j] = g_val[k] + g_svi[k] * decimal_years_;
            h[i - 1][j] = h_val[k] + h_svi[k] * decimal_years_;

            k++;
        }
    }
}

void igrf_update(float latitude, float longitude, float radius, int interpolate_flag, vec3 *B_ned)
{

    float a = 6371.2; // Radius of Earth, Km
    float a_r = a / radius;

    // Check pole to avoid singularities (Re-check)
    if (latitude > -1.7453293e-10 && latitude < -1.7453293e-10)
        latitude = -1.7453293e-10;
    else if (latitude < 3.14159265376 && latitude > 3.14159265342)
        latitude = 3.14159265342;

    float theta = PI_2 - latitude; // Co-latitude
    float phi = longitude;

    if (interpolate_flag)
    {
        compute_gauss_coeffs(decimal_years);
    }

    //  1. Calculate Legendre polynomials and its derivatives recursively
    //  2. Compute magnetic field strength using terms computed in 1
    float Plnln, dPlnln, Plnm, dPlnm, Pnm, dPnm, Pllnm, dPllnm, Knm;
    float B_radial = 0.0, B_theta = 0.0, B_phi = 0.0;

    // For n = m = 0
    Plnln = 1;
    Plnm = 1;
    dPlnln = 0;
    dPlnm = 0;

    for (int m = 0; m <= IGRF_GEN; m++)
    {
        for (int n = 1; n <= IGRF_GEN; n++)
        {
            if (m <= n)
            {
                if (n == m)
                {
                    Pnm = sin(theta) * Plnln;
                    dPnm = sin(theta) * dPlnln + cos(theta) * Plnln;

                    Plnln = Pnm;
                    Plnm = Plnln;
                    Pllnm = 0;

                    dPlnln = dPnm;
                    dPlnm = dPlnln;
                    dPllnm = 0;
                }

                else if (n == 1)
                {
                    Pnm = cos(theta) * Plnm;
                    dPnm = cos(theta) * dPlnm - sin(theta) * Plnm;

                    Pllnm = Plnm;
                    Plnm = Pnm;

                    dPllnm = dPlnm;
                    dPlnm = dPnm;
                }

                else // ((n != m) && (n > 1))
                {
                    Knm = (float)((n - 1) * (n - 1) - m * m) / ((2.0f * n - 1) * (2.0f * n - 3));
                    Pnm = cos(theta) * Plnm - Knm * Pllnm;
                    dPnm = cos(theta) * dPlnm - sin(theta) * Plnm - Knm * dPllnm;

                    Pllnm = Plnm;
                    Plnm = Pnm;

                    dPllnm = dPlnm;
                    dPlnm = dPnm;
                }

                float factor = pow(a_r, n + 2);
                float g_ = g[n - 1][m];
                float h_ = h[n - 1][m];
                float cos_ = cos(m * phi);
                float sin_ = sin(m * phi);
                float hsin = h_ * sin_;
                float gcos = g_ * cos_;

                B_radial += factor * (n + 1) * ((g_ * cos_ + hsin) * Pnm);
                B_theta += factor * ((g_ * cos_ + hsin) * dPnm);
                B_phi += factor * (m * (-g_ * sin_ + h_ * cos_) * Pnm);
            }
        }
    }

    igrf_B_sph[0] =  B_radial;
    igrf_B_sph[1] = -B_theta;
    igrf_B_sph[2] = -B_phi / sin(theta);
    igrf_B_ned[0] = -igrf_B_sph[1];
    igrf_B_ned[1] =  igrf_B_sph[2];
    igrf_B_ned[2] = -igrf_B_sph[0];

    vec_set((double) igrf_B_ned[0], 
            (double) igrf_B_ned[1],
            (double) igrf_B_ned[2], 
            B_ned);
}

float igrf_get_horizontal_intensity()
{
    float x = igrf_B_ned[0];
    float y = igrf_B_ned[1];

    return sqrt(x * x + y * y);
}

float igrf_get_inclination()
{
    float x = igrf_B_ned[0];
    float y = igrf_B_ned[1];
    float z = igrf_B_ned[2];
    float hypotenuse = sqrt(x * x + y * y);

    return atan(z / hypotenuse);
}

float igrf_get_declination()
{
    float x = igrf_B_ned[0];
    float y = igrf_B_ned[1];

    return atan2(y, x);
}

float igrf_get_norm()
{
    float x = igrf_B_ned[0];
    float y = igrf_B_ned[1];
    float z = igrf_B_ned[2];

    return sqrt(x * x + y * y + z * z);
}


igrf_set_date_status
igrf_set_date_time(uint16_t year, uint8_t month, uint8_t day,
                           uint8_t hour, uint8_t min, uint8_t sec)
{
    decimal_years = 0;

    if ((year < IGRF_START_YEAR) || (year > IGRF_END_YEAR) ||
        (month > 12) || (day > 31) || (hour > 23) || (min > 60) || (sec > 60))
    {
        return IGRF_SET_DATE_OUT_OF_BOUNDS;
    }

    // Count complete years
    int complete_years = 0;
    for (int i = IGRF_START_YEAR; i < year; i++)
    {
        complete_years++;
    }

    // Count decimal years incomplete year
    int leap_flag = (((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0)));
    int total_days_in_year = leap_flag ? 366 : 365;

    int days_arr[] = {0, 31, 59, 90, 120, 151, 182, 212, 243, 273, 304, 334};
    double day_in_year = (days_arr[month - 1] + day + (month > 2 ? leap_flag : 0));
    double hour_in_day = hour + (min / 60) + (sec / 3600);

    decimal_years = complete_years + day_in_year / total_days_in_year +
                    hour_in_day / (24.0f * total_days_in_year);

    return IGRF_SET_DATE_SUCCESS;
}

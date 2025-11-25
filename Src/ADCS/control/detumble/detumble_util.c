#include "control/detumble/detumble_util.h"
#include "control/detumble/detumble.h"
#include "math.h"
#include "control/bdot/bdot_control.h"
#include "virtual_intellisat.h"

// TODO: Implement computeB_coils to convert coils curr to magitude

const double control_constant = 67200.0; // TODO: tune :p
const double coilInductance = 1;         // TODO: Messure (Henrys)
const double coilResistance = 1;         // TODO: Measure (Ohms)
const double B_Earth = 1;                // TODO: I need
const double decayPercent = 0.2;         // TODO: Decide on percentage

vec3 findAngVel(vec3 b0, vec3 b1, uint64_t delta_t) {

    vec3 bdot;   // The velocity vector pointing from b0 to b1
    vec3 angVel; // The angular velocity

    if (delta_t == 0)
        return (vec3){0.0, 0.0, 0.0};

    bdot_control(b1, b0, delta_t, &bdot);
    vec_scalar((1.0 / vec_mag(b0)), bdot, &angVel);

    // Convert into degree per second
    double RPSmultiplier = (180 / M_PI) * 1000;
    vec_scalar(RPSmultiplier, angVel, &angVel);

    return angVel;
}

// TODO: work on this function
double computeB_coils(double current) { return current; }

double computeDecay(double B_initial) {

    double tau = coilInductance / coilResistance;
    double ratio = B_Earth / B_initial;

    return -1 * tau * log(decayPercent * ratio);
}

bool detumbleDelay(vec3 mdm)
{
    double coilsMagnetic;   // |Coil's magnetic field|
    double delayTime;

    coilsMagnetic = computeB_coils(vec_mag(mdm));
    delayTime = computeDecay(coilsMagnetic);
    if (vi_delay_ms(delayTime)) return true;

    return false;
}

vec3 computeMDM(vec3 mag_curr, vec3 mag_prev, uint64_t delta_t, vec3 coils_curr,
                vec3 needle) {

    vec3 mdm;
    bdot_control(mag_curr, mag_prev, delta_t, &coils_curr);
    vec_sub(coils_curr, needle, &coils_curr);
    vec_scalar(-control_constant, coils_curr, &mdm);
    capCurrent(&mdm);

    return mdm;
}

bool aboveThreshold(vec3 input, double threshold) {
    if (input.x > threshold || input.y > threshold || input.z > threshold)
        return true;

    return false;
}

void capCurrent(vec3 *mdm) {
    vec3 coils_output = *mdm;

    // cap output at maximum 0.158 Amps across all coils.
    double temp_mag = vec_mag(coils_output);
    if (temp_mag > 0.158f) {
        // this step is equivalent to 0.158 * normalized temp.
        vec_scalar(0.158f / temp_mag, coils_output, &coils_output);
    }

    (*mdm) = coils_output;
}
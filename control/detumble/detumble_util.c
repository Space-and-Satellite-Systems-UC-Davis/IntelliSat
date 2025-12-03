#include "control/detumble/detumble_util.h"

// TODO: Implement computeB_coils to convert coils curr to magitude


const double coilInductance = 1;         // TODO: Messure (Henrys)
const double coilResistance = 1;         // TODO: Measure (Ohms)
const double B_Earth = 1;                // TODO: I need
const double decayPercent = 0.2;         // TODO: Decide on percentage


// TODO: work on this function
// Look into vi_get_coils_current()
double computeB_coils(double current) { return current; }

// TODO: switch into some kind of constant timeout
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
#include "db_convert.h"

#include <math.h>

#define DB_CONVERT_MIN_VOLTS 1e-6f

static float db_convert_safe_volts(float volts_rms) {
    /* Not fabsf(): this firmware's exported libm subset disables fabsf (and
       nearly everything else - see the log10f note below), so anything used
       here has to either be a plain expression or go through powf. */
    float v = volts_rms < 0.0f ? -volts_rms : volts_rms;
    return v < DB_CONVERT_MIN_VOLTS ? DB_CONVERT_MIN_VOLTS : v;
}

/*
 * log10f is listed in this firmware's API table but disabled (not linked
 * into this build) - confirmed by actually building against the real SDK.
 * powf is the one enabled transcendental function, so log10 is computed by
 * numerically inverting it: find y such that powf(10, y) == x, via
 * bisection. Not hot-path code (called a handful of times per UI refresh,
 * not per audio sample), so ~40 powf calls per conversion is not a concern.
 */
static float db_log10f(float x) {
    float lo = -45.0f, hi = 45.0f; /* comfortably covers float's dynamic range */
    for(int i = 0; i < 40; i++) {
        float mid = (lo + hi) * 0.5f;
        if(powf(10.0f, mid) < x) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return (lo + hi) * 0.5f;
}

float db_convert_volts_to_dbu(float volts_rms) {
    return 20.0f * db_log10f(db_convert_safe_volts(volts_rms) / DB_CONVERT_DBU_REF_VOLTS);
}

float db_convert_volts_to_dbv(float volts_rms) {
    return 20.0f * db_log10f(db_convert_safe_volts(volts_rms) / DB_CONVERT_DBV_REF_VOLTS);
}

float db_convert_dbu_to_volts(float dbu) {
    return DB_CONVERT_DBU_REF_VOLTS * powf(10.0f, dbu / 20.0f);
}

float db_convert_dbv_to_volts(float dbv) {
    return DB_CONVERT_DBV_REF_VOLTS * powf(10.0f, dbv / 20.0f);
}

float db_convert_dbu_to_dbv(float dbu) {
    /* 0 dBV expressed in dBu, derived rather than hardcoded so it can't drift
       out of sync with the reference voltages above. */
    return dbu - db_convert_volts_to_dbu(DB_CONVERT_DBV_REF_VOLTS);
}

float db_convert_dbv_to_dbu(float dbv) {
    return dbv + db_convert_volts_to_dbu(DB_CONVERT_DBV_REF_VOLTS);
}

float db_convert_adc_code_to_input_volts(uint16_t adc_code, const DbConvertCalibration* cal) {
    float adc_volts = ((float)adc_code / (float)cal->adc_max_code) * cal->adc_vref_volts;
    return (adc_volts - cal->bias_volts) / cal->attenuation;
}

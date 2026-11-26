#include "db_convert.h"

#include <math.h>

#define DB_CONVERT_MIN_VOLTS 1e-6f

static float db_convert_safe_volts(float volts_rms) {
    float v = fabsf(volts_rms);
    return v < DB_CONVERT_MIN_VOLTS ? DB_CONVERT_MIN_VOLTS : v;
}

float db_convert_volts_to_dbu(float volts_rms) {
    return 20.0f * log10f(db_convert_safe_volts(volts_rms) / DB_CONVERT_DBU_REF_VOLTS);
}

float db_convert_volts_to_dbv(float volts_rms) {
    return 20.0f * log10f(db_convert_safe_volts(volts_rms) / DB_CONVERT_DBV_REF_VOLTS);
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

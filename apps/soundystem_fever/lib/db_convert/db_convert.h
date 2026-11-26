#pragma once

#include <stdint.h>

/*
 * Pure, SDK-independent dBu/dBV/volts conversions - see ../docs/audio-levels.md
 * for the reference definitions these implement. No Flipper headers, no
 * hardware access: safe to reuse from any page (or a future calibration
 * flow), and testable outside the firmware with a plain C compiler.
 */

#define DB_CONVERT_DBU_REF_VOLTS 0.775f
#define DB_CONVERT_DBV_REF_VOLTS 1.0f

float db_convert_volts_to_dbu(float volts_rms);
float db_convert_volts_to_dbv(float volts_rms);
float db_convert_dbu_to_volts(float dbu);
float db_convert_dbv_to_volts(float dbv);
float db_convert_dbu_to_dbv(float dbu);
float db_convert_dbv_to_dbu(float dbv);

/*
 * Calibration for the RCA front end in ../docs/rca-input-frontend.md: converts
 * a raw ADC code back to the volts implied at the RCA jack. The values below
 * match that doc's nominal design; real numbers should come from the M4
 * in-app calibration flow (../PLAN.md), not be trusted as exact.
 */
typedef struct {
    float adc_vref_volts;  /* ADC reference, e.g. 3.3f */
    uint16_t adc_max_code; /* e.g. 4095 for a 12-bit ADC */
    float bias_volts;      /* Node B quiescent bias, nominally 1.65f */
    float attenuation;     /* Node-B-AC / RCA-input ratio, nominally 0.4f (-8 dB) */
} DbConvertCalibration;

#define DB_CONVERT_NOMINAL_CALIBRATION \
    { .adc_vref_volts = 3.3f, .adc_max_code = 4095, .bias_volts = 1.65f, .attenuation = 0.4f }

/*
 * Instantaneous input volts at the RCA jack implied by one raw ADC code. This
 * is a single-sample value (signed, relative to 0V at the jack) - computing
 * RMS/peak over a sample window belongs to the measurement page / PLAN.md M3,
 * not to this module.
 */
float db_convert_adc_code_to_input_volts(uint16_t adc_code, const DbConvertCalibration* cal);

/* Automatically generated nanopb header */
/* Generated by nanopb-0.2.9-dev at Fri Oct  3 13:46:47 2014. */

#ifndef _PB_READINGS_PB_H_
#define _PB_READINGS_PB_H_
#include <pb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
/* Struct definitions */
typedef struct {
    size_t size;
    uint8_t bytes[92];
} Sample_AVR_t;

typedef struct _Sample {
    uint32_t time;
    bool has_batt;
    float batt;
    bool has_temp;
    float temp;
    bool has_accX;
    int32_t accX;
    bool has_accY;
    int32_t accY;
    bool has_accZ;
    int32_t accZ;
    bool has_ADC1;
    uint32_t ADC1;
    bool has_ADC2;
    uint32_t ADC2;
    bool has_rain;
    uint32_t rain;
    bool has_AVR;
    Sample_AVR_t AVR;
} Sample;

/* Default values for struct fields */

/* Field tags (for use in manual encoding/decoding) */
#define Sample_time_tag                          1
#define Sample_batt_tag                          2
#define Sample_temp_tag                          3
#define Sample_accX_tag                          4
#define Sample_accY_tag                          5
#define Sample_accZ_tag                          6
#define Sample_ADC1_tag                          7
#define Sample_ADC2_tag                          8
#define Sample_rain_tag                          9
#define Sample_AVR_tag                           10

/* Struct field encoding specification for nanopb */
extern const pb_field_t Sample_fields[11];

/* Maximum encoded size of messages (where known) */
#define Sample_size                              145

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
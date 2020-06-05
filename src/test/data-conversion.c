#include <esdm-internal.h>
#include <stdbool.h>

int main() {
  const int64_t ref_int64[] = {0, 1, 0x7f, 0xff, 0x7fff, 0xffff, 0x7fffffff, 0xffffffff, 0x7fffffffffffffff, -0x8000000000000000, -0x80000000, -0x8000, -0x80, -1};
  const int32_t ref_int32[] = {0, 1, 0x7f, 0xff, 0x7fff, 0xffff, 0x7fffffff, -0x80000000, -0x8000, -0x80, -1};
  const int16_t ref_int16[] = {0, 1, 0x7f, 0xff, 0x7fff, -0x8000, -0x80, -1};
  const int8_t ref_int8[] = {0, 1, 0x7f, -0x80, -1};
  const uint64_t ref_uint64[] = {0, 1, 0x7f, 0xff, 0x7fff, 0xffff, 0x7fffffff, 0xffffffff, 0x7fffffffffffffff, 0xffffffffffffffff};
  const uint32_t ref_uint32[] = {0, 1, 0x7f, 0xff, 0x7fff, 0xffff, 0x7fffffff, 0xffffffff};
  const uint16_t ref_uint16[] = {0, 1, 0x7f, 0xff, 0x7fff, 0xffff};
  const uint8_t ref_uint8[] = {0, 1, 0x7f, 0xff};
  const float ref_float[] = {-1.0e30, -0xffffff, -1, -1.0e-30, -0.0, 0.0, 1.0e-30, 1, 0xffffff, 1.0e30};
  const double ref_double[] = {-1.0e300, -0xffffff, -1, -1.0e-300, -0.0, 0.0, 1.0e-300, 1, 0xffffff, 1.0e300};

  bool success = true;

  #define checkConversion(reference, sourceType, sourceFormat, sourcePrintType, destType, destFormat, destPrintType) { \
    size_t elementCount = sizeof(reference)/sizeof*reference; \
    destType converted[elementCount]; \
    ea_datatype_converter converter = ea_converter_for_types(smd_c_to_smd_type((destType){0}), smd_c_to_smd_type((sourceType){0})); \
    if(!converter) fprintf(stderr, "no converter found to convert `"#sourceType"` to `"#destType"`\n"), abort(); \
    void* result = converter(converted, (const sourceType*)reference, sizeof(reference)); /* the cast exists to remove the `volatile` */ \
    eassert(result == converted); \
    \
    for(int i = elementCount; i--; ) { \
      destType expected = (destType)*(volatile sourceType*)&reference[i]; \
      if(converted[i] < expected || converted[i] > expected) { /* need to avoid `!=` here to quench unsafe floating point comparison warnings */ \
        fprintf(stderr, "conversion of "sourceFormat" ("#sourceType") to "#destType" yields wrong value "destFormat" (expected "destFormat")\n", (sourcePrintType)reference[i], (destPrintType)converted[i], (destPrintType)(destType)reference[i]); \
        success = false; \
      } \
    } \
  }

  //This is separated into an integer and a floating point part to avoid testing the realm of UB too much.
  //My `gcc` seems to flipping a coin to decide whether `-1e30` should be converted to the `uint32_t` value `0` or `0x80000000`.
  #define checkConversionsFromSourceToInt(reference, sourceType, sourceFormat, sourcePrintType) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, int8_t, "%"PRId8, int8_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, int16_t, "%"PRId16, int16_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, int32_t, "%"PRId32, int32_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, int64_t, "%"PRId64, int64_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, uint8_t, "%"PRIu8, uint8_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, uint16_t, "%"PRIu16, uint16_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, uint32_t, "%"PRIu32, uint32_t) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, uint64_t, "%"PRIu64, uint64_t)

  #define checkConversionsFromSourceToFloat(reference, sourceType, sourceFormat, sourcePrintType) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, float, "%g", double) \
    checkConversion(reference, sourceType, sourceFormat, sourcePrintType, double, "%g", double)

  checkConversionsFromSourceToInt(ref_int8, int8_t, "%"PRId8, int8_t)
  checkConversionsFromSourceToInt(ref_int16, int16_t, "%"PRId16, int16_t)
  checkConversionsFromSourceToInt(ref_int32, int32_t, "%"PRId32, int32_t)
  checkConversionsFromSourceToInt(ref_int64, int64_t, "%"PRId64, int64_t)
  checkConversionsFromSourceToInt(ref_uint8, uint8_t, "%"PRIu8, uint8_t)
  checkConversionsFromSourceToInt(ref_uint16, uint16_t, "%"PRIu16, uint16_t)
  checkConversionsFromSourceToInt(ref_uint32, uint32_t, "%"PRIu32, uint32_t)
  checkConversionsFromSourceToInt(ref_uint64, uint64_t, "%"PRIu64, uint64_t)

  checkConversionsFromSourceToFloat(ref_float, float, "%g", double)
  checkConversionsFromSourceToFloat(ref_double, double, "%g", double)

  if(success) {
    printf("OK\n");
  } else {
    fprintf(stderr, "FAILED\n");
    abort();
  }
}

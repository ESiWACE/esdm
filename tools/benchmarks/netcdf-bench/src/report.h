/*
 * =====================================================================================
 *
 *       Filename:  report.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  08/18/2016 10:06:54 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (),
 *   Organization:
 *
 * =====================================================================================
 */

#ifndef report_INC
#define report_INC

#include "benchmark.h"
#include "constants.h"
#include "types.h"

#define NROWS 5
#define NCOLS 8

typedef char *(*table_t)[NROWS];

typedef enum report_type_t { REPORT_PARSER,
  REPORT_HUMAN } report_type_t;
typedef double (*get_bm_value_t)(const benchmark_t *bms);

typedef struct mam_t {
  double min;
  double avg;
  double max;
} mam_t;

typedef struct report_t {
  benchmark_t *bm;
} report_t;

/**
 * @brief
 *
 * @param
 */
void report_init(report_t *report);

/**
 * @brief
 *
 * @param report
 * @param bm
 */
void report_setup(report_t *report, benchmark_t *bm);

/**
 * @brief
 *
 * @param report
 */
void report_destroy(report_t *report);

/**
 * @brief
 *
 * @param report
 * @param type
 */

void report_print(const report_t *report, const report_type_t type);

#endif /* ----- #ifndef report_INC  ----- */

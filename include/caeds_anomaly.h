#ifndef CAEDS_ANOMALY_H
#define CAEDS_ANOMALY_H

/**
 * caeds_anomaly.h - CHARM Anomaly Detection Module Header
 * 
 * This header defines the interface for the CAEDS anomaly detection
 * subsystem, which is responsible for identifying potential weaknesses
 * in entropy sources and alerting when entropy quality degrades.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/**
 * Initialize the anomaly detection module
 * 
 * @return CHARM_SUCCESS on success, error code otherwise
 */
int caeds_anomaly_init(void);

/**
 * Register an entropy source for anomaly detection
 * 
 * @param source_id Unique identifier for the entropy source
 * @param name Human-readable name for the source
 * @return CHARM_SUCCESS on success, error code otherwise
 */
int caeds_anomaly_register_source(uint32_t source_id, const char *name);

/**
 * Analyze entropy sample for anomalies
 * 
 * @param source_id Identifier of the entropy source
 * @param data Pointer to entropy data buffer
 * @param length Length of entropy data in bytes
 * @return Anomaly score (0.0 = no anomalies, 1.0 = severe anomalies)
 */
double caeds_anomaly_analyze(uint32_t source_id, const uint8_t *data, size_t length);

/**
 * Get the number of anomalies detected for a source
 * 
 * @param source_id Identifier of the entropy source
 * @return Count of detected anomalies
 */
uint32_t caeds_anomaly_get_count(uint32_t source_id);

/**
 * Reset anomaly counters for a source
 * 
 * @param source_id Identifier of the entropy source
 * @return CHARM_SUCCESS on success, error code otherwise
 */
int caeds_anomaly_reset(uint32_t source_id);

/**
 * Set the anomaly detection threshold
 * 
 * @param threshold New threshold value (0.0-1.0)
 * @return CHARM_SUCCESS on success, error code otherwise
 */
int caeds_anomaly_set_threshold(double threshold);

/**
 * Get the current anomaly detection threshold
 * 
 * @return Current threshold value
 */
double caeds_anomaly_get_threshold(void);

/**
 * Clean up the anomaly detection module
 */
void caeds_anomaly_cleanup(void);

#endif /* CAEDS_ANOMALY_H */

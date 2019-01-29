#ifdef HAS_BME

#include "bme680mems.h"

// Local logging tag
static const char TAG[] = "main";

bmeStatus_t bme_status;
TaskHandle_t BmeTask;

bsec_virtual_sensor_t sensorList[10] = {
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
};

uint8_t bsecstate_buffer[BSEC_MAX_STATE_BLOB_SIZE] = {0};
uint16_t stateUpdateCounter = 0;

Bsec iaqSensor;

// initialize BME680 sensor
int bme_init(void) {

  // return = 0 -> error / return = 1 -> success

  // block i2c bus access
  if (I2C_MUTEX_LOCK()) {

    Wire.begin(HAS_BME);
    iaqSensor.begin(BME_ADDR, Wire);

    ESP_LOGI(TAG, "BSEC v%d.%d.%d.%d", iaqSensor.version.major,
             iaqSensor.version.minor, iaqSensor.version.major_bugfix,
             iaqSensor.version.minor_bugfix);

    iaqSensor.setConfig(bsec_config_iaq);

    if (checkIaqSensorStatus())
      ESP_LOGI(TAG, "BME680 sensor found and initialized");
    else {
      ESP_LOGE(TAG, "BME680 sensor not found");
      goto error;
    }

    loadState();

    iaqSensor.setTemperatureOffset((float)BME_TEMP_OFFSET);
    iaqSensor.updateSubscription(sensorList, 10, BSEC_SAMPLE_RATE_LP);

    if (checkIaqSensorStatus())
      ESP_LOGI(TAG, "BSEC subscription succesful");
    else {
      ESP_LOGE(TAG, "BSEC subscription error");
      goto error;
    }
  } else {
    ESP_LOGE(TAG, "I2c bus busy - BME680 initialization error");
    goto error;
  }

  I2C_MUTEX_UNLOCK(); // release i2c bus access
  return 1;

error:
  I2C_MUTEX_UNLOCK(); // release i2c bus access
  return 0;

} // bme_init()

// Helper function definitions
int checkIaqSensorStatus(void) {
  int rslt = 1; // true = 1 = no error, false = 0 = error

  if (iaqSensor.status != BSEC_OK) {
    rslt = 0;
    if (iaqSensor.status < BSEC_OK)
      ESP_LOGE(TAG, "BSEC error %d", iaqSensor.status);
    else
      ESP_LOGW(TAG, "BSEC warning %d", iaqSensor.status);
  }

  if (iaqSensor.bme680Status != BME680_OK) {
    rslt = 0;
    if (iaqSensor.bme680Status < BME680_OK)
      ESP_LOGE(TAG, "BME680 error %d", iaqSensor.bme680Status);
    else
      ESP_LOGW(TAG, "BME680 warning %d", iaqSensor.bme680Status);
  }

  return rslt;
} // checkIaqSensorStatus()

// loop function which reads and processes data based on sensor settings
void bme_loop(void *pvParameters) {

  configASSERT(((uint32_t)pvParameters) == 1); // FreeRTOS check

#ifdef HAS_BME
  while (1) {
    // block i2c bus access
    if (I2C_MUTEX_LOCK()) {
      if (iaqSensor.run()) { // If new data is available
        bme_status.raw_temperature = iaqSensor.rawTemperature;
        bme_status.raw_humidity = iaqSensor.rawHumidity;
        bme_status.temperature = iaqSensor.temperature;
        bme_status.humidity = iaqSensor.humidity;
        bme_status.pressure =
            (iaqSensor.pressure / 100.0); // conversion Pa -> hPa
        bme_status.iaq = iaqSensor.iaqEstimate;
        bme_status.iaq_accuracy = iaqSensor.iaqAccuracy;
        bme_status.gas = iaqSensor.gasResistance;
        updateState();
      }
      I2C_MUTEX_UNLOCK();
    }
  }
#endif
  ESP_LOGE(TAG, "BME task ended");
  vTaskDelete(BmeTask); // should never be reached

} // bme_loop()

void loadState(void) {
  if (cfg.bsecstate[BSEC_MAX_STATE_BLOB_SIZE] == BSEC_MAX_STATE_BLOB_SIZE) {
    // Existing state in NVS stored
    ESP_LOGI(TAG, "restoring BSEC state from NVRAM");
    memcpy(bsecstate_buffer, cfg.bsecstate, BSEC_MAX_STATE_BLOB_SIZE);
    iaqSensor.setState(bsecstate_buffer);
    checkIaqSensorStatus();
  } else // no state stored
    ESP_LOGI(TAG,
             "no BSEC state stored in NVRAM, starting sensor with defaults");
}

void updateState(void) {
  bool update = false;

  if (stateUpdateCounter == 0) {
    // first state update when IAQ accuracy is >= 1
    if (iaqSensor.iaqAccuracy >= 1) {
      update = true;
      stateUpdateCounter++;
    }
  } else {

    /* Update every STATE_SAVE_PERIOD minutes */
    if ((stateUpdateCounter * STATE_SAVE_PERIOD) < millis()) {
      update = true;
      stateUpdateCounter++;
    }
  }

  if (update) {
    iaqSensor.getState(bsecstate_buffer);
    checkIaqSensorStatus();
    memcpy(cfg.bsecstate, bsecstate_buffer, BSEC_MAX_STATE_BLOB_SIZE);
    cfg.bsecstate[BSEC_MAX_STATE_BLOB_SIZE] = BSEC_MAX_STATE_BLOB_SIZE;
    ESP_LOGI(TAG, "saving BSEC state to NVRAM");
    saveConfig();
  }
}

#endif // HAS_BME
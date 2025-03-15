#include <openiris.hpp>
#include "data/DeviceMode/DeviceMode.hpp"
/**
 * @brief ProjectConfig object
 * @brief This is the main configuration object for the project
 * @param name The name of the project config partition
 * @param mdnsName The mDNS hostname to use
 */
ProjectConfig deviceConfig("openiris", MDNS_HOSTNAME);
CommandManager commandManager(&deviceConfig);
SerialManager serialManager(&commandManager);

#ifdef CONFIG_CAMERA_MODULE_ESP32S3_XIAO_SENSE
LEDManager ledManager(LED_BUILTIN);

#elif CONFIG_CAMERA_MODULE_SWROOM_BABBLE_S3
LEDManager ledManager(38);

#else
LEDManager ledManager(33);
#endif  // ESP32S3_XIAO_SENSE

#ifndef SIM_ENABLED
CameraHandler cameraHandler(deviceConfig);
#endif  // SIM_ENABLED

#ifndef ETVR_EYE_TRACKER_USB_API
WiFiHandler wifiHandler(deviceConfig,
                        WIFI_SSID,
                        WIFI_PASSWORD,
                        WIFI_CHANNEL,
                        ENABLE_ADHOC);
MDNSHandler mdnsHandler(deviceConfig);
#ifdef SIM_ENABLED
APIServer apiServer(deviceConfig, wifiStateManager, "/control");
#else
APIServer apiServer(deviceConfig, cameraHandler, "/control");
StreamServer streamServer;
#endif  // SIM_ENABLED

void etvr_eye_tracker_web_init() {
  // Check if mode has been changed to USB mode before starting network initialization
  DeviceModeManager* deviceModeManager = DeviceModeManager::getInstance();
  if (deviceModeManager && deviceModeManager->getMode() == DeviceMode::USB_MODE) {
    log_i("[SETUP]: Mode changed to USB before network initialization, aborting");
    WiFi.disconnect(true);
    return;
  }
  
  log_d("[SETUP]: Starting Network Handler");
  deviceConfig.attach(mdnsHandler);
  
  // Check mode again before starting WiFi
  if (deviceModeManager && deviceModeManager->getMode() == DeviceMode::USB_MODE) {
    log_i("[SETUP]: Mode changed to USB before WiFi initialization, aborting");
    WiFi.disconnect(true);
    return;
  }
  
  log_d("[SETUP]: Starting WiFi Handler");
  wifiHandler.begin();
  
  // Check mode again before starting MDNS
  if (deviceModeManager && deviceModeManager->getMode() == DeviceMode::USB_MODE) {
    log_i("[SETUP]: Mode changed to USB before MDNS initialization, aborting");
    WiFi.disconnect(true);
    return;
  }
  
  log_d("[SETUP]: Starting MDNS Handler");
  mdnsHandler.startMDNS();

  switch (wifiStateManager.getCurrentState()) {
    case WiFiState_e::WiFiState_Disconnected: {
      //! TODO: Implement
      break;
    }
    case WiFiState_e::WiFiState_ADHOC: {
#ifndef SIM_ENABLED
      log_d("[SETUP]: Starting Stream Server");
      streamServer.startStreamServer();
#endif  // SIM_ENABLED
      log_d("[SETUP]: Starting API Server");
      apiServer.setup();
      break;
    }
    case WiFiState_e::WiFiState_Connected: {
#ifndef SIM_ENABLED
      log_d("[SETUP]: Starting Stream Server");
      streamServer.startStreamServer();
#endif  // SIM_ENABLED
      log_d("[SETUP]: Starting API Server");
      apiServer.setup();
      break;
    }
    case WiFiState_e::WiFiState_Connecting: {
      //! TODO: Implement
      break;
    }
    case WiFiState_e::WiFiState_Error: {
      //! TODO: Implement
      break;
    }
  }
}
#endif  // ETVR_EYE_TRACKER_WEB_API

void setup() {
  setCpuFrequencyMhz(240);
  Serial.begin(115200);
  Logo::printASCII();
  ledManager.begin();
  
  DeviceModeManager::createInstance();
  DeviceModeManager* deviceModeManager = DeviceModeManager::getInstance();

  #ifdef CONFIG_CAMERA_MODULE_SWROOM_BABBLE_S3  // Set IR emitter strength to 100%.  
    const int ledPin = 1;                       // Replace this with a command endpoint eventually.
    const int freq = 5000;
    const int ledChannel = 0;
    const int resolution = 8;
    const int dutyCycle = 255;
    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(1, ledChannel);
    ledcWrite(ledChannel, dutyCycle); 
  #endif

#ifndef SIM_ENABLED
  deviceConfig.attach(cameraHandler);
#endif  // SIM_ENABLED
  deviceConfig.load();

  serialManager.init();

  DeviceMode currentMode = deviceModeManager->getMode();
  
  if (currentMode == DeviceMode::WIFI_MODE) {
    // Initialize WiFi mode
    etvr_eye_tracker_web_init();
    log_i("[SETUP]: Initialized in WiFi mode");
  } else if (currentMode == DeviceMode::AP_MODE) {
    // Initialize AP mode with serial commands enabled
    etvr_eye_tracker_web_init();
    log_i("[SETUP]: Initialized in AP mode with serial commands enabled");
  } else {
    WiFi.disconnect(true);
    log_i("[SETUP]: Initialized in USB mode");
  }
}

void loop() {
  ledManager.handleLED();
  serialManager.run();
}

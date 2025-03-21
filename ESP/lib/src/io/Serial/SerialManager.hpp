#pragma once
#ifndef SERIAL_MANAGER
#define SERIAL_MANAGER

#include <Arduino.h>
#include <ArduinoJson.h>
#include <USBCDC.h>
#include <esp_camera.h>
#include "data/CommandManager/CommandManager.hpp"
#include "data/config/project_config.hpp"

const char* const ETVR_HEADER = "\xff\xa0";
const char* const ETVR_HEADER_FRAME = "\xff\xa1";

enum QueryAction {
  READY_TO_RECEIVE,
  PARSE_COMMANDS,
  CONNECT_TO_WIFI,
};

enum QueryStatus {
  NONE,
  SUCCESS,
  ERROR,
};

const std::unordered_map<QueryAction, std::string> queryActionMap = {
    {QueryAction::READY_TO_RECEIVE, "ready_to_receive"},
    {QueryAction::PARSE_COMMANDS, "parse_commands"},
    {QueryAction::CONNECT_TO_WIFI, "connect_to_wifi"},
};

class SerialManager {
 private:
  esp_err_t err = ESP_OK;
  CommandManager* commandManager;

  int64_t last_frame = 0;
  long last_request_time = 0;

  void send_frame();

 public:
  SerialManager(CommandManager* commandManager);
  void sendQuery(QueryAction action,
                 QueryStatus status,
                 std::string additional_info);
  void init();
  void run();
  void checkUSBMode();
};

#endif
//
// Created by mr on 11/14/2023.
//

#ifndef ARDUINO_ESP_32_CAM_APP_HTTPD_H
#define ARDUINO_ESP_32_CAM_APP_HTTPD_H


#include <esp_err.h>
#include <esp_http_server.h>

class app_httpd {
public:

    static void startCameraServer();
    static esp_err_t index_handler(httpd_req_t *req);

    static esp_err_t win_handler(httpd_req_t *req);
    static esp_err_t pll_handler(httpd_req_t *req);
    static int parse_get_var(char *buf, const char * key, int def);
    static esp_err_t greg_handler(httpd_req_t *req);



};


#endif //ARDUINO_ESP_32_CAM_APP_HTTPD_H

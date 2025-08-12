// ESP32 Server with static HTML page

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define SSID "Dov SMART"
#define PASS "0$43378976"
#define IR_LED_PIN 2

static bool led_state = 0; 
SemaphoreHandle_t binary_sem;

static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        printf("WiFi connecting ... \n");
        break;

    case WIFI_EVENT_STA_CONNECTED:
        printf("WiFi connected ... \n");
        break;

    case WIFI_EVENT_STA_DISCONNECTED:
        printf("WiFi lost connection ... \n");
        break;

	case IP_EVENT_STA_GOT_IP:
		printf("WiFi got IP ... \n\n");
		break;

    default:
        break;
    }
}

void wifi_connection()
{
    // 1 - Wi-Fi/LwIP Init Phase
    esp_netif_init(); // TCP/IP initiation 					s1.1
	esp_event_loop_create_default(); // event loop 			s1.2
	esp_netif_create_default_wifi_sta(); // WiFi station 	s1.3
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); // 					s1.4
	
	// 2 - Wi-Fi Configuration Phase
	esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);

	// 3 - Wi-Fi Start Phase
    esp_wifi_start();

	// 4- Wi-Fi Connect Phase
	esp_wifi_connect();
}

static esp_err_t get_base_html_handler(httpd_req_t *req)
{
    char *response_message = "<!DOCTYPE HTML><html>\
                                <head>\
                                <title>ESP32 Web Server</title>\
                                <style>\
                                    body { font-family: Arial, sans-serif; text-align: center; background-color: #2d2d2d; color: #fff; }\
                                    h1 { color: #0F3376; padding: 2vh; }\
                                    p { font-size: 1.5rem; }\
                                    button {\
                                        padding: 50px 50px;\
                                        font-size: 3rem;\
                                        color: #fff;\
                                        background-color: #4CAF50;\
                                        border: none;\
                                        border-radius: 12px;\
                                        box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);\
                                        cursor: pointer;\
                                        transition: background-color 0.3s ease, box-shadow 0.3s ease;\
                                        }\
                                        button:hover {\
                                        background-color: #45a049;\
                                        box-shadow: 0 6px 8px rgba(0, 0, 0, 0.15);\
                                        }\
                                        button:active {\
                                        background-color: #3e8e41;\
                                        box-shadow: 0 2px 4px rgba(0, 0, 0, 0.1);\
                                        transform: translateY(2px);\
                                    }\
                                </style>\
                                </head>\
                                <body>\
                                <h1>ESP32 Web Server</h1>\
                                <p>Click the button to toggle AC:</p>\
                                <button onclick=\"toggleAC()\">Toggle AC</button>\
                                <p id=\"status\">AC is OFF</p>\
                                <script>\
                                    function toggleAC() {\
                                    fetch(\'/toggle\').then(response => response.text()).then(state => {\
                                        document.getElementById(\'status\').innerText = \'AC is \' + state;\
                                    });\
                                    }\
                                </script>\
                                </body>\
                            </html>";
    httpd_resp_send(req, response_message, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t toggle_ac_handler(httpd_req_t *req)
{
    xSemaphoreGive(binary_sem);
    led_state = !led_state;
    ESP_LOGI("toggle", "gpio is:%d", led_state);
    // gpio_set_level(GPIO_NUM_2, led_state);
    httpd_resp_send(req, led_state ? "ON":"OFF", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void server_initiation()
{
    httpd_config_t server_config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server_handle = NULL;
    httpd_start(&server_handle, &server_config);

    httpd_uri_t uri_get = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = get_base_html_handler,
        .user_ctx = NULL
    };
    
    httpd_uri_t uri_toggle_ac = {
        .uri = "/toggle",
        .method = HTTP_GET,
        .handler = toggle_ac_handler,
        .user_ctx = NULL
    };

    httpd_register_uri_handler(server_handle, &uri_get);
    httpd_register_uri_handler(server_handle, &uri_toggle_ac);

}

void pulseIR(long microsecs) {
    // we'll count down from the number of microseconds we are told to wait
    
    while (microsecs > 0) {
        // 38 kHz is about 13 microseconds high and 13 microseconds low
        gpio_set_level(IR_LED_PIN, 1);  // this takes about 3 microseconds to happen
        esp_rom_delay_us(10);        // hang out for 10 microseconds, you can also change this to 9 if its not working
        gpio_set_level(IR_LED_PIN, 0);   // this also takes about 3 microseconds
        esp_rom_delay_us(10);        // hang out for 10 microseconds, you can also change this to 9 if its not working
        
        // so 26 microseconds altogether
        microsecs -= 26;
    }
    
}
void logicOne(){
    pulseIR(950);  
}

void logicZero(){
    esp_rom_delay_us(950);
}

void ac_power_code(){
    int i = 0;
    for(i=0; i<3 ; i++){
        
        //0b11100001,//msb
        logicOne();
        logicOne();
        logicOne();
        logicZero();
        logicZero();
        logicZero();
        logicZero();
        logicOne();
        
        //0b10100110,//166
        logicOne();
        logicZero();
        logicOne(); 
        logicZero();
        logicZero();
        logicOne(); 
        logicOne(); 
        logicZero(); 
        
        //0b10101010,//170
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        
        //0b10100110,//166
        logicOne();
        logicZero();
        logicOne(); 
        logicZero();
        logicZero();
        logicOne(); 
        logicOne(); 
        logicZero(); 
        
        //0b10101010,//170
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        
        //0b10101010,//170
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        
        //0b10101010,//170
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        
        //0b10101010,//170
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        
        //0b10101001,//169
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicOne();
        logicZero();
        logicZero();
        logicOne();
        
        //0b10000000 //128    
        logicOne();
        logicZero();
        
    }  
    logicOne();
    logicOne();
    logicOne();
    logicOne();
}

// task to toggle AC
void toggle_ac_task(void* parameters){
    while(1){
        xSemaphoreTake(binary_sem, portMAX_DELAY);
        ac_power_code();
    }
}

void app_main(void)
{
    // Set IR LED pin directin
    gpio_set_direction(IR_LED_PIN, GPIO_MODE_OUTPUT);
    
    // Initialize binary semaphore
    binary_sem = xSemaphoreCreateBinary();

    // Connecting to Wifi - Runs on core 0
    nvs_flash_init();   
	wifi_connection();  

    // Initializing web server - Runs on core 0
	server_initiation();

    // IR code sending task - Runs on core 1
    // The reason is that the IR pulses need to be accurate and 
    // should not be preempted by any other task. Running on core 0
    // requires the use of an ISR 
    xTaskCreatePinnedToCore(
        toggle_ac_task,
        "Send AC code task",
        1024,
        NULL,
        0,
        NULL,
        1
    );
}
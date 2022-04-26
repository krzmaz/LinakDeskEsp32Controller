#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#define USE_LITTLEFS true
#define ESP_DRD_USE_LITTLEFS true
#define ESP_DRD_USE_SPIFFS false
#define ESP_DRD_USE_EEPROM false

#define DRD_TIMEOUT 10
#define DRD_ADDRESS 0
#include <ArduinoJson.h>
#include <DeskControllerFactory.h>
#include <ESP_DoubleResetDetector.h> //https://github.com/khoih-prog/ESP_DoubleResetDetector
#include <ESPAsync_WiFiManager.h> //https://github.com/khoih-prog/ESPAsync_WiFiManager
#include <ESPmDNS.h>

#include "html.h"
#include <fauxmoESP.h>
#include <TaskScheduler.h>

#define DESK_NAME_MAX_LEN 32
#define DESK_BT_ADDRESS_LEN 18
#define DeskName_Label "DeskName"
#define DeskBtAddress_Label "DeskBtAddress"
#define UserReset_Label "UserReset"

char deskName[DESK_NAME_MAX_LEN] = "Standing desk";
char deskBtAddress[DESK_BT_ADDRESS_LEN] = "AA:BB:CC:DD:EE:FF";

const char* CONFIG_FILE = "/ConfigSW.json";

DoubleResetDetector* drd;
const int PIN_LED = 2;
bool needsConfig = false;
AsyncWebServer server(80);
LinakDesk::DeskController controller = LinakDesk::DeskControllerFactory::make();
fauxmoESP fauxmo;
Scheduler runner;

void checkConnection(){
    auto isConnected = controller.isConnected();
    Serial.printf("[checkConnection] (millis: %d), state: %d\n", millis(), isConnected);
    if (!isConnected){
        Serial.printf("[checkConnection] Not connected trying to reconnect!\n");
        controller.connect(deskBtAddress);
    }
    if(!controller.isConnected()){
        Serial.printf("[checkConnection] Reconnect unsuccessful, rebooting!\n");
        ESP.restart();
    }
}

Task checkConnectionTask(60000, TASK_FOREVER, &checkConnection);

void moveToHeightHttpHandler(AsyncWebServerRequest* request) {
    if (request->hasParam("destination")) {
        auto message = request->getParam("destination")->value();
        auto destination = message.toInt();
        if (controller.isConnected() && destination >= 0 && destination < 7000) {
            request->send(200, "text/plain", "Moving to: " + message);
            controller.moveToHeightRaw(destination);
            return;
        }
    }
    request->send(400, "text/plain", "Wrong input");
}

void moveToHeightMmHttpHandler(AsyncWebServerRequest* request) {
    if (request->hasParam("destination")) {
        auto message = request->getParam("destination")->value();
        auto destination = message.toInt();
        if (controller.isConnected() && destination >= 0 && destination < 2000) {
            request->send(200, "text/plain", "Moving to: " + message);
            controller.moveToHeightMm(destination);
            return;
        }
    }
    request->send(400, "text/plain", "Wrong input");
}

void saveCurrentHeightAsFavHttpHandler(AsyncWebServerRequest* request) {
    if (request->hasParam("position")) {
        auto message = request->getParam("position")->value();
        auto position = message.toInt();
        if (controller.isConnected() && position >= 0 && position < 4) {
            request->send(200, "text/plain", "Saving current height to position number: " + message);
            controller.setMemoryPositionFromCurrentHeight(position);
            return;
        }
    }
    request->send(400, "text/plain", "Wrong input");
}

void notFound(AsyncWebServerRequest* request) { request->send(404, "text/plain", "Not found"); }

bool writeConfigFile(bool userReset = false) {
    Serial.println("Saving config file");

    DynamicJsonDocument json(1024);

    // JSONify local configuration parameters
    json[DeskName_Label] = deskName;
    json[DeskBtAddress_Label] = deskBtAddress;
    json[UserReset_Label] = userReset;

    // Open file for writing
    File f = FileFS.open(CONFIG_FILE, "w");

    if (!f) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    serializeJsonPretty(json, Serial);
    // Write data to file and close it
    serializeJson(json, f);

    f.close();

    Serial.println("\nConfig file was successfully saved");
    return true;
}

bool readConfigFile() {
    // this opens the config file in read-mode
    File f = FileFS.open(CONFIG_FILE, "r");

    if (!f) {
        Serial.println("Configuration file not found");
        return false;
    } else {
        // we could open the file
        size_t size = f.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size + 1]);

        // Read and store file contents in buf
        f.readBytes(buf.get(), size);
        // Closing file
        f.close();
        // Using dynamic JSON buffer which is not the recommended memory model, but anyway
        // See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model

        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        if (deserializeError) {
            Serial.println("JSON parseObject() failed");
            return false;
        }
        serializeJson(json, Serial);

        // Parse all config file parameters, override
        // local config variables with parsed values
        if (json.containsKey(DeskName_Label)) {
            strcpy(deskName, json[DeskName_Label]);
        }
        if (json.containsKey(DeskBtAddress_Label)) {
            strcpy(deskBtAddress, json[DeskBtAddress_Label]);
        }
        if (json.containsKey(UserReset_Label)) {
            if (json[UserReset_Label]) {
                needsConfig = true;
            }
        }
    }
    Serial.println("\nConfig file was successfully parsed");
    return true;
}

void initFS() {
    // Format FileFS if not yet
    if (!FileFS.begin(true)) {
        Serial.print(FS_Name);
        Serial.println(F(" failed! AutoFormatting."));
    }
}

void wifiManagerSetup() {
    if (!readConfigFile()) {
        Serial.println(F("Failed to read ConfigFile, using default values"));
        needsConfig = true;
    }

    drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    if (drd->detectDoubleReset()) {
        Serial.println(F("Detected Double Reset"));
        needsConfig = true;
    }
    // Resources for ConfigPortal that won't be used for normal operation
    const char customhtml[] PROGMEM = "pattern=\"([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}\"";
    ESPAsync_WMParameter p_deskName(DeskName_Label, "Desk Name", deskName, DESK_NAME_MAX_LEN);
    ESPAsync_WMParameter p_deskBtAddress(DeskBtAddress_Label, "Desk BT address", deskBtAddress, DESK_BT_ADDRESS_LEN,
                                         customhtml);
    DNSServer dnsServer;
    ESPAsync_WiFiManager ESPAsync_wifiManager(&server, &dnsServer, "LinakDeskEsp32Controller");

    ESPAsync_wifiManager.addParameter(&p_deskName);
    ESPAsync_wifiManager.addParameter(&p_deskBtAddress);

    if (ESPAsync_wifiManager.WiFi_SSID() == "") {
        Serial.println(F("No AP credentials"));
        needsConfig = true;
    }
    if (needsConfig) {
        Serial.println(F("Starting Config Portal"));
        digitalWrite(PIN_LED, HIGH);
        if (!ESPAsync_wifiManager.startConfigPortal()) {
            Serial.println(F("Not connected to WiFi"));
        } else {
            Serial.println(F("connected"));
        }

        strcpy(deskName, p_deskName.getValue());
        strcpy(deskBtAddress, p_deskBtAddress.getValue());
        writeConfigFile();
        digitalWrite(PIN_LED, LOW);
    } else {
        WiFi.mode(WIFI_STA);
        WiFi.begin();
    }
}

void setupWebServer() {
    // respond to GET requests on URL /heap
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send_P(200, "text/html", html::index_html, html::processor);
    });
    server.on("/cn", HTTP_GET, [](AsyncWebServerRequest* request) {
        writeConfigFile(true);
        ESP.restart();
    });
    server.on("/heap", HTTP_GET,
              [](AsyncWebServerRequest* request) { request->send(200, "text/plain", String(ESP.getFreeHeap())); });
    server.on("/moveToHeight", HTTP_GET, moveToHeightHttpHandler);
    server.on("/moveToHeightMm", HTTP_GET, moveToHeightMmHttpHandler);
    server.on("/saveCurrentPosAsFav", HTTP_GET, saveCurrentHeightAsFavHttpHandler);
    server.on("/getHeight", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", String(controller.getHeightRaw()).c_str());
    });
    server.on("/getHeightMm", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", String(controller.getHeightMm()).c_str());
    });
        // These two callbacks are required for echo gen1 and gen3 compatibility
    server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
        // Handle any other body request here...
    });
    server.onNotFound([](AsyncWebServerRequest *request) {
        String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
        if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
        return notFound(request);
    });
    auto strin = std::string(deskName);
    std::replace(strin.begin(), strin.end(), ' ', '-');
    MDNS.begin(strin.c_str());
    MDNS.addService("http", "tcp", 80);
    server.begin();
    Serial.println("HTTP server started");
}

void setupFauxmo(){
    // Set fauxmoESP to not create an internal TCP server and redirect requests to the server on the defined port
    // The TCP port must be 80 for gen3 devices (default is 1901)
    // This has to be done before the call to enable()
    fauxmo.createServer(false);
    fauxmo.setPort(80); // This is required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

    // You can use different ways to invoke alexa to modify the devices state:
    // "Alexa, turn kitchen on" ("kitchen" is the name of the first device below)
    // "Alexa, turn on kitchen"
    // "Alexa, set kitchen to fifty" (50 means 50% of brightness)

    // Add virtual devices
    fauxmo.addDevice(deskName);

    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
        
        Serial.printf("[MAIN] Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

        if (strcmp(device_name, deskName) == 0)
        {
            if (controller.getMemoryPosition(1).has_value() && controller.getMemoryPosition(3).has_value()) {

            unsigned char destinationPosition = state ? 3 : 1;
            controller.moveToHeightRaw(controller.getMemoryPosition(destinationPosition).value());
        }
            /* code */
        }
        
    });
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
    Serial.begin(115200);
    while (!Serial) {
    };
    delay(200);

    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 1); // enable brownout detector

    initFS();
    runner.init();
    runner.addTask(checkConnectionTask);

    wifiManagerSetup();

    if (controller.connect(deskBtAddress)) {
        auto before = millis();
        Serial.printf("Current height: %d mm\n", controller.getHeightMm());
        Serial.printf("Getting height and printing it took: %ldms\n", millis() - before);
    }

    unsigned long startedAt = millis();
    Serial.print(F("After waiting "));
    int connRes = WiFi.waitForConnectResult();
    float waited = (millis() - startedAt);
    Serial.print(waited / 1000);
    Serial.print(F(" secs , WiFi connection result is "));
    Serial.println(connRes);
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("Failed to connect"));
        drd->stop();
        delay(10000);
        ESP.restart();
    } else {
        Serial.print(F("Local IP: "));
        Serial.println(WiFi.localIP());
        setupWebServer();
        setupFauxmo();
    }
    checkConnectionTask.enableDelayed(60000);
}
void loop() {
    delay(1);
    drd->loop();
    controller.loop();
    fauxmo.handle();
    runner.execute();
}

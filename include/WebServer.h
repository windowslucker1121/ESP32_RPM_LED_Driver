#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <FS.h>
#include <SPIFFS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "Settings.h"
#include <Update.h>

#define WS_MSG_DATA_UPDATE 0x01
#define WS_MSG_PING 0x02
#define WS_MSG_PONG 0x03
#define WS_MSG_CONFIG_REQUEST 0x04
#define WS_MSG_CONFIG_RESPONSE 0x05
#define WS_MSG_STYLE_REQUEST 0x06
#define WS_MSG_STYLE_RESPONSE 0x07
#define WS_MSG_TEST_MODE 0x08

#define WS_MSG_CONFIG_UPDATE 0x10

static const uint8_t WS_MSG_STYLE_LIST_REQUEST = 0x20;
static const uint8_t WS_MSG_STYLE_LIST_RESPONSE = 0x21;
static const uint8_t WS_MSG_STYLE_GET_REQUEST = 0x22;
static const uint8_t WS_MSG_STYLE_GET_RESPONSE = 0x23;
static const uint8_t WS_MSG_STYLE_SAVE = 0x24;
static const uint8_t WS_MSG_STYLE_SAVE_RESULT = 0x25;
static const uint8_t WS_MSG_STYLE_DELETE = 0x26;
static const uint8_t WS_MSG_STYLE_DELETE_RESULT = 0x27;

class WebServer
{
public:
    WebServer(int port, Settings *&settingsRef)
        : server(port),
          ws("/ws"),
          settings(settingsRef)
    {
        lastClientActivity = 0;
        clientConnected = false;
    }

    void setup()
    {
        if (!SPIFFS.begin(true))
        {
            logger.logln("SPIFFS Mount Failed");
            return;
        }

        logger.logln("SPIFFS Mounted Successfully");

        if (!SPIFFS.exists("/main.html"))
        {
            logger.logln("main.html not found in SPIFFS! Did you forget to build the Filesystem image?");
        }
        else
        {
            logger.logln("main.html found.");
        }
        if (!SPIFFS.exists("/main.css"))
        {
            logger.logln("main.css not found in SPIFFS! Did you forget to build the Filesystem image?");
        }
        else
        {
            logger.logln("main.css found.");
        }
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                {
                    request->redirect("/main.html");
                });
        server.on("/main.html", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                    request->send(SPIFFS, "/main.html", "text/html"); 
                });
        server.on("/main.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { 
                    request->send(SPIFFS, "/main.css", "text/css"); 
                });

        server.on("/style-editor.html", HTTP_GET, [this](AsyncWebServerRequest *request)
                { 
                    request->send(SPIFFS, "/style-editor.html", "text/html"); 
                });

        server.on("/firmware-update.html", HTTP_GET, [](AsyncWebServerRequest *request) {
            request->send(SPIFFS, "/firmware-update.html", "text/html");
        });

        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len)
                   { handleWebSocketEvent(server, client, type, arg, data, len); });
        server.addHandler(&ws);

        setupFirmwareUploadHandler();
        setupSPIFFSManagerEndpoints();

        server.begin();
    }

    void loop()
    {
        if (clientConnected && (millis() - lastClientActivity > 5000))
        {
            // logger.logln("WebSocket client timeout, disconnecting");
            ws.closeAll();
            clientConnected = false;
        }

        static unsigned long lastUpdate = 0;
        if (clientConnected && (millis() - lastUpdate > 500))
        {
            sendSensorData();
            lastUpdate = millis();
        }
    }

    AsyncWebServer &getServer()
    {
        return server;
    }

    void handleStyleListRequest(AsyncWebSocketClient *client)
    {
        std::vector<std::string> styleNames = LedDisplayStyleRegistry::getInstance().getRegisteredStyleNames();
        std::vector<String> customStyles = LedDisplayStyleRegistry::getInstance().getCustomStyleNames();

        for (const auto &customStyle : customStyles)
        {
            auto it = std::remove(styleNames.begin(), styleNames.end(), customStyle.c_str());
            styleNames.erase(it, styleNames.end());
        }

        logger.logln("Registered styles:");
        for (const auto &name : styleNames)
        {
            String strName = String(name.c_str());
            logger.logln("  - " + strName);
        }

        logger.logln("Custom styles:");
        for (const auto &name : customStyles)
        {
            logger.logln("  - " + name);
        }

        size_t totalSize = 2; // Type byte + count byte
        for (const auto &name : styleNames)
        {
            totalSize += 1 + name.length(); // Length byte + name
        }
        for (const auto &name : customStyles)
        {
            totalSize += 1 + name.length(); // Length byte + name
        }

        uint8_t *response = new uint8_t[totalSize];
        response[0] = WS_MSG_STYLE_LIST_RESPONSE;
        response[1] = styleNames.size() + customStyles.size();

        size_t pos = 2;
        for (const auto &name : styleNames)
        {
            uint8_t nameLen = name.length();
            response[pos++] = nameLen;
            memcpy(response + pos, name.c_str(), nameLen);
            pos += nameLen;
        }
        for (const auto &name : customStyles)
        {
            uint8_t nameLen = name.length();
            response[pos++] = nameLen;
            memcpy(response + pos, name.c_str(), nameLen);
            pos += nameLen;
        }

        client->binary(response, totalSize);
        logger.logln("Sent style list response to client");
        delete[] response;
    }

    void handleStyleGetRequest(AsyncWebSocketClient *client, const String &styleName)
    {
        String styleJson;
        bool found = false;

        if (styleName == "LeftToRight" || styleName == "EqualFromBothSides" ||
            styleName == "FlashingLeftToRight" || styleName == "RightToLeft" )
        {
            StaticJsonDocument<512> doc;
            doc["name"] = styleName;

            if (styleName == "LeftToRight")
            {
                doc["pattern"] = 0; // LEFT_TO_RIGHT
                doc["enableFlashing"] = false;
            }
            else if (styleName == "EqualFromBothSides")
            {
                doc["pattern"] = 3; // EDGES_TO_CENTER
                doc["enableFlashing"] = false;
            }
            else if (styleName == "FlashingLeftToRight")
            {
                doc["pattern"] = 0; // LEFT_TO_RIGHT
                doc["enableFlashing"] = true;
            } 
            else if (styleName == "RightToLeft")
            {
                doc["pattern"] = 1; // RIGHT_TO_LEFT
                doc["enableFlashing"] = false;
            }

            JsonObject colors = doc.createNestedObject("colors");
            colors["lowR"] = 0;
            colors["lowG"] = 255;
            colors["lowB"] = 0;
            colors["midR"] = 255;
            colors["midG"] = 255;
            colors["midB"] = 0;
            colors["highR"] = 255;
            colors["highG"] = 0;
            colors["highB"] = 0;
            colors["lowPercent"] = 40;
            colors["highPercent"] = 30;

            doc["flashInterval"] = 80;

            serializeJson(doc, styleJson);
            found = true;
        }
        else
        {
            Preferences preferences;
            if (preferences.begin("ledStyles", true))
            {
                styleJson = preferences.getString(styleName.c_str(), "");
                preferences.end();
                found = styleJson.length() > 0;
            }
        }

        if (found)
        {
            size_t totalSize = 1 + styleJson.length();
            uint8_t *response = new uint8_t[totalSize];
            response[0] = WS_MSG_STYLE_GET_RESPONSE;
            memcpy(response + 1, styleJson.c_str(), styleJson.length());

            client->binary(response, totalSize);
            delete[] response;
        }
        else
        {
            uint8_t response[2] = {WS_MSG_STYLE_GET_RESPONSE, 0};
            client->binary(response, 2);
        }
    }

    void handleStyleDeleteRequest(AsyncWebSocketClient *client, const String &styleName)
    {
        if (styleName == "LeftToRight" || styleName == "EqualFromBothSides" ||
            styleName == "FlashingLeftToRight" || styleName == "RightToLeft")
        {
            uint8_t response[2] = {WS_MSG_STYLE_DELETE_RESULT, 0}; // Failure
            client->binary(response, 2);
            return;
        }

        bool success = LedDisplayStyleRegistry::getInstance().deleteCustomStyle(styleName);

        uint8_t response[2] = {WS_MSG_STYLE_DELETE_RESULT, success ? 1 : 0};
        client->binary(response, 2);
    }

private:
    AsyncWebServer server;
    AsyncWebSocket ws;
    Settings *&settings;
    bool clientConnected;
    unsigned long lastClientActivity;

    void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        switch (type)
        {
        case WS_EVT_CONNECT:
            // logger.logln(String("WebSocket client connected from ") + client->remoteIP().toString());
            clientConnected = true;
            lastClientActivity = millis();
            break;

        case WS_EVT_DISCONNECT:
            // logger.logln(String("WebSocket client disconnected from ") + client->remoteIP().toString());
            if (ws.count() == 0)
            {
                clientConnected = false;
            }
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(client, data, len, arg);
            break;

        case WS_EVT_ERROR:
            logger.logln("WebSocket error");
            break;
        }
    }
    void handleWebSocketMessage(AsyncWebSocketClient *client, uint8_t *data, size_t len, void *arg)
    {
        if (len < 1)
            return;

        lastClientActivity = millis();

        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        // logger.logln("WebSocket message received, opcode: " + String(info->opcode) + ", length: " + String(len));
        if (info->opcode == WS_BINARY)
        {
            if (len > 0)
            {
                uint8_t messageType = data[0];
                // logger.logln("Received binary message of type: " + String(messageType));

                switch (messageType)
                {
                case WS_MSG_STYLE_LIST_REQUEST:
                    logger.logln("Received style list request");
                    handleStyleListRequest(client);
                    break;

                case WS_MSG_STYLE_GET_REQUEST:
                    logger.logln("Received style get request");
                    if (len > 1)
                    {
                        String styleName((char *)(data + 1), len - 1);
                        handleStyleGetRequest(client, styleName);
                    }
                    break;

                case WS_MSG_STYLE_DELETE:
                    logger.logln("Received style delete request");
                    if (len > 1)
                    {
                        String styleName((char *)(data + 1), len - 1);
                        handleStyleDeleteRequest(client, styleName);
                    }
                    break;
                case WS_MSG_PING:
                    sendPong(client);
                    break;

                case WS_MSG_CONFIG_REQUEST:
                    logger.logln("Received config request");
                    sendConfigData(client);
                    break;

                case WS_MSG_STYLE_REQUEST:
                    logger.logln("Received style request");
                    sendStyleData(client);
                    break;

                case WS_MSG_TEST_MODE:
                    logger.logln("Test Mode Activated!");
                    break;
                case WS_MSG_STYLE_SAVE:
                {
                    logger.logln("Received style save request");
                    data[len] = 0;
                    String jsonString = String((char *)data);

                    logger.logln("Received JSON: " + jsonString);
                    jsonString.remove(0, 1);

                    StaticJsonDocument<1024> doc;
                    DeserializationError error = deserializeJson(doc, jsonString);

                    if (!error)
                    {
                        logger.logln("Parsed JSON successfully");
                        if (doc.containsKey("name") && doc.containsKey("pattern") && doc.containsKey("colors"))
                        {
                            String name = doc["name"].as<String>();
                            logger.logln("Saving Style name: " + name);

                            bool success = LedDisplayStyleRegistry::getInstance().saveCustomStyle(name, jsonString);
                            logger.logln("Style save result: " + String(success ? "Success" : "Failure"));

                            logger.logln("Current availble styles:");
                            std::vector<String> customStyles = LedDisplayStyleRegistry::getInstance().getCustomStyleNames();
                            for (const auto &style : customStyles)
                            {
                                logger.logln(" - " + style);
                            }
                            uint8_t response[2] = {WS_MSG_STYLE_SAVE_RESULT, success ? 1 : 0};
                            client->binary(response, 2);
                        }
                    }
                    else
                    {
                        logger.logln("Failed to parse JSON: " + String(error.c_str()));
                        uint8_t response[2] = {WS_MSG_STYLE_SAVE_RESULT, 0};
                        client->binary(response, 2);
                    }
                    break;
                }
                case WS_MSG_CONFIG_UPDATE:
                {
                    int minHz = data[1] | (data[2] << 8);
                    int maxHz = data[3] | (data[4] << 8);
                    bool isEnabled = (data[5] != 0);
                    int brightness = data[6] | (data[7] << 8);
                    int bootupAnimationTime = data[8] | (data[9] << 8);
                    
                    settings->setReaderMinFrequency(minHz);
                    settings->setReaderMaxFrequency(maxHz);
                    settings->setIsEnabled(isEnabled);
                    settings->setLedBrightness(brightness);
                    settings->setBootupAnimationTime(bootupAnimationTime);
                    
                    logger.logln("Received config update: minHz=" + String(minHz) +
                    ", maxHz=" + String(maxHz) +
                    ", isEnabled=" + String(isEnabled) +
                    ", brightness=" + String(brightness) +
                    ", bootupAnimationTime=" + String(bootupAnimationTime));
                    if (len > 5)
                    {
                        String style((char *)&data[10], len - 10);
                        logger.logln("Received style update: " + style);
                        settings->setStyle(style.c_str());
                    }

                    settings->SaveSettings();
                    uint8_t response[1] = {0x11};
                    client->binary(response, 1);
                    break;
                }
                default:
                    logger.logln("Received unknown binary message type: " + String(messageType));
                    break;
                }
            }
        }
        else
        {
            logger.logln("Received text message, opcode: " + String(info->opcode));
            logger.logln("Text message: " + String((char *)data, len));
            logger.logln("Message will be ignored due to missing implementation");
        }
    }

    void sendPong(AsyncWebSocketClient *client)
    {
        uint8_t buffer[1] = {WS_MSG_PONG};
        client->binary(buffer, 1);
    }

    void sendSensorData()
    {
        if (!clientConnected)
            return;

        int currentHz = settings->getCurrentAnalogValue();
        int mappedValue = settings->getCurrentMappedValue();

        // Format: [MSG_TYPE, Hz_LSB, Hz_MSB, MappedVal]
        uint8_t buffer[4];
        buffer[0] = WS_MSG_DATA_UPDATE;
        buffer[1] = currentHz & 0xFF;
        buffer[2] = (currentHz >> 8) & 0xFF;
        buffer[3] = mappedValue;

        ws.binaryAll(buffer, sizeof(buffer));
    }

    void sendConfigData(AsyncWebSocketClient *client)
    {
        int minHz = settings->getReaderMinFrequency();
        int maxHz = settings->getReaderMaxFrequency();
        bool isEnabled = settings->getIsEnabled();
        int brightness = settings->getLedBrightness();
        int bootupAnimationTime = settings->getBootupAnimationTime();

        // Format: [MSG_TYPE, minHz_LSB, minHz_MSB, maxHz_LSB, maxHz_MSB]
        uint8_t buffer[10];
        buffer[0] = WS_MSG_CONFIG_RESPONSE;
        buffer[1] = minHz & 0xFF;
        buffer[2] = (minHz >> 8) & 0xFF;
        buffer[3] = maxHz & 0xFF;
        buffer[4] = (maxHz >> 8) & 0xFF;
        buffer[5] = isEnabled ? 1 : 0;
        buffer[6] = brightness & 0xFF;
        buffer[7] = (brightness >> 8) & 0xFF;
        buffer[8] = bootupAnimationTime & 0xFF;
        buffer[9] = (bootupAnimationTime >> 8) & 0xFF;
        logger.logln("Sending config data: " + String(minHz) + " - " + String(maxHz) + ", enabled: " + String(isEnabled) +", brightness: " + String(brightness) + ", bootupAnimationTime: " + String(bootupAnimationTime));
        client->binary(buffer, sizeof(buffer));
    }

    void sendStyleData(AsyncWebSocketClient *client)
    {
        LedDisplayStyleRegistry &registry = LedDisplayStyleRegistry::getInstance();
        std::vector<std::string> styles = registry.getRegisteredStyleNames();
        String currentStyle = settings->getDisplayStyleName();

        size_t totalSize = 2; // Type byte + count byte
        totalSize += currentStyle.length() + 1; // +1 for length byte

        for (const auto &style : styles)
        {
            totalSize += style.length() + 1;
        }

        uint8_t *buffer = new uint8_t[totalSize];
        size_t pos = 0;

        buffer[pos++] = WS_MSG_STYLE_RESPONSE;

        buffer[pos++] = styles.size();

        buffer[pos++] = currentStyle.length();
        memcpy(buffer + pos, currentStyle.c_str(), currentStyle.length());
        pos += currentStyle.length();

        for (const auto &style : styles)
        {
            buffer[pos++] = style.length();
            memcpy(buffer + pos, style.c_str(), style.length());
            pos += style.length();
        }

        client->binary(buffer, totalSize);
        delete[] buffer;
    }

    void setupFirmwareUploadHandler() {
        server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
            bool updateSuccess = !Update.hasError();
            AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", updateSuccess ? "Update Success" : "Update Failed");
            response->addHeader("Connection", "close");
            request->send(response);
            ESP.restart();
        }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                logger.logln("Firmware upload started: " + filename);
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            }

            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }

            if (final) {
                if (Update.end(true)) {
                    logger.logln("Firmware upload completed successfully.");
                } else {
                    Update.printError(Serial);
                }
            }
        });
    }

    void setupSPIFFSManagerEndpoints() {
        server.on("/list-files", HTTP_GET, [](AsyncWebServerRequest *request) {
            File root = SPIFFS.open("/");
            String json = "[";
            File file = root.openNextFile();
            while (file) {
                if (json.length() > 1) json += ",";
                json += "\"" + String(file.name()) + "\"";
                file = root.openNextFile();
            }
            json += "]";
            request->send(200, "application/json", json);
        });

        server.on("/file", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (!request->hasParam("name")) {
                request->send(400, "text/plain", "Missing file name");
                return;
            }
            String fileName = request->getParam("name")->value();
            if (!fileName.startsWith("/")) {
                fileName = "/" + fileName;
            }
            if (!SPIFFS.exists(fileName)) {
                request->send(404, "text/plain", "File not found");
                return;
            }
            request->send(SPIFFS, fileName, "application/octet-stream");
        });

        server.on("/delete-file", HTTP_DELETE, [](AsyncWebServerRequest *request) {
            if (!request->hasParam("name")) {
                request->send(400, "text/plain", "Missing file name");
                return;
            }
            String fileName = request->getParam("name")->value();
            if (!SPIFFS.exists(fileName)) {
                request->send(404, "text/plain", "File not found");
                return;
            }
            SPIFFS.remove(fileName);
            request->send(200, "text/plain", "File deleted");
        });

        server.on("/upload-file", HTTP_POST, [](AsyncWebServerRequest *request) {
            request->send(200, "text/plain", "File uploaded");
        }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
            if (!index) {
                File file = SPIFFS.open("/" + filename, FILE_WRITE);
                if (!file) {
                    request->send(500, "text/plain", "Failed to open file for writing");
                    return;
                }
                file.close();
            }
            File file = SPIFFS.open("/" + filename, FILE_APPEND);
            if (file) {
                file.write(data, len);
                file.close();
            }
            if (final) {
                Serial.printf("File upload completed: %s\n", filename.c_str());
            }
        });

        server.on("/save-file", HTTP_POST, [](AsyncWebServerRequest *request) {
            logger.logln("Save file request received");
            

            String fileName = "";
            String fileContent = "";
            std::vector<String> prequiredParams = {"name", "content"};
            for (size_t i = 0; i < request->params(); i++) {
                const AsyncWebParameter *p = request->getParam(i);
                // Serial.println("Parameter: " + p->name() + " = " + p->value());
                if (p->name() == "name")
                {
                    fileName =  "/" + p->value();
                    // Serial.println("File name: " + fileName);
                }
                else if (p->name() == "content")
                {
                    fileContent = p->value();
                    // Serial.println("File content length: " + String(fileContent.length()));
                }
                prequiredParams.erase(std::remove(prequiredParams.begin(), prequiredParams.end(), p->name()), prequiredParams.end());
            }
            
            if (!prequiredParams.empty()) {
                logger.logln("Missing required parameters in save request: " + String(prequiredParams.size()));
                request->send(400, "text/plain", "Missing file name or content");
                return;
            }
            
            logger.logln("Saving file: " + fileName + ", size: " + String(fileContent.length()));
            
            File file = SPIFFS.open(fileName, FILE_WRITE);
            if (!file) {
                logger.logln("Failed to open file for writing: " + fileName);
                request->send(500, "text/plain", "Failed to open file for writing");
                return;
            }
            
            size_t written = file.print(fileContent);
            file.close();
            
            if (written > 0) {
                logger.logln("File saved successfully: " + fileName + ", " + String(written) + " bytes written");
                request->send(200, "text/plain", "File saved successfully");
            } else {
                logger.logln("Failed to write content to file: " + fileName);
                request->send(500, "text/plain", "Failed to write file content");
            }
        });
        
    }
};

#endif // WEBSERVER_H

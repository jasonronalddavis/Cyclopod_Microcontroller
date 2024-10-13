#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>







const char* ssId = "iPhone";
const char* pasword = "Barnes!!";

// API Key
const char* apiKey = "AIzaSyB_eFvcXt8CfCLd-fMd83Ze0bcwHRBdFFc";

// Google Cloud Speech-to-Text Server
const char* serVer = "speech.googleapis.com";  // Use the host only, without HTTPS prefix.

// SSL root certificate
const char* root_Ca = 
"-----BEGIN CERTIFICATE-----\n"
"MIIFYjCCBEqgAwIBAgIQd70NbNs2+RrqIQ/E8FjTDTANBgkqhkiG9w0BAQsFADBX\n"
"MQswCQYDVQQGEwJCRTEZMBcGA1UEChMQR2xvYmFsU2lnbiBudi1zYTEQMA4GA1UE\n"
"CxMHUm9vdCBDQTEbMBkGA1UEAxMSR2xvYmFsU2lnbiBSb290IENBMB4XDTIwMDYx\n"
"OTAwMDA0MloXDTI4MDEyODAwMDA0MlowRzELMAkGA1UEBhMCVVMxIjAgBgNVBAoT\n"
"GUdvb2dsZSBUcnVzdCBTZXJ2aWNlcyBMTEMxFDASBgNVBAMTC0dUUyBSb290IFIx\n"
"MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEAthECix7joXebO9y/lD63\n"
"ladAPKH9gvl9MgaCcfb2jH/76Nu8ai6Xl6OMS/kr9rH5zoQdsfnFl97vufKj6bwS\n"
"iV6nqlKr+CMny6SxnGPb15l+8Ape62im9MZaRw1NEDPjTrETo8gYbEvs/AmQ351k\n"
"KSUjB6G00j0uYODP0gmHu81I8E3CwnqIiru6z1kZ1q+PsAewnjHxgsHA3y6mbWwZ\n"
"DrXYfiYaRQM9sHmklCitD38m5agI/pboPGiUU+6DOogrFZYJsuB6jC511pzrp1Zk\n"
"j5ZPaK49l8KEj8C8QMALXL32h7M1bKwYUH+E4EzNktMg6TO8UpmvMrUpsyUqtEj5\n"
"cuHKZPfmghCN6J3Cioj6OGaK/GP5Afl4/Xtcd/p2h/rs37EOeZVXtL0m79YB0esW\n"
"CruOC7XFxYpVq9Os6pFLKcwZpDIlTirxZUTQAs6qzkm06p98g7BAe+dDq6dso499\n"
"iYH6TKX/1Y7DzkvgtdizjkXPdsDtQCv9Uw+wp9U7DbGKogPeMa3Md+pvez7W35Ei\n"
"Eua++tgy/BBjFFFy3l3WFpO9KWgz7zpm7AeKJt8T11dleCfeXkkUAKIAf5qoIbap\n"
"sZWwpbkNFhHax2xIPEDgfg1azVY80ZcFuctL7TlLnMQ/0lUTbiSw1nH69MG6zO0b\n"
"9f6BQdgAmD06yK56mDcYBZUCAwEAAaOCATgwggE0MA4GA1UdDwEB/wQEAwIBhjAP\n"
"BgNVHRMBAf8EBTADAQH/MB0GA1UdDgQWBBTkrysmcRorSCeFL1JmLO/wiRNxPjAf\n"
"BgNVHSMEGDAWgBRge2YaRQ2XyolQL30EzTSo//z9SzBgBggrBgEFBQcBAQRUMFIw\n"
"JQYIKwYBBQUHMAGGGWh0dHA6Ly9vY3NwLnBraS5nb29nL2dzcjEwKQYIKwYBBQUH\n"
"MAKGHWh0dHA6Ly9wa2kuZ29vZy9nc3IxL2dzcjEuY3J0MDIGA1UdHwQrMCkwJ6Al\n"
"oCOGIWh0dHA6Ly9jcmwucGtpLmdvb2cvZ3NyMS9nc3IxLmNybDA7BgNVHSAENDAy\n"
"MAgGBmeBDAECATAIBgZngQwBAgIwDQYLKwYBBAHWeQIFAwIwDQYLKwYBBAHWeQIF\n"
"AwMwDQYJKoZIhvcNAQELBQADggEBADSkHrEoo9C0dhemMXoh6dFSPsjbdBZBiLg9\n"
"NR3t5P+T4Vxfq7vqfM/b5A3Ri1fyJm9bvhdGaJQ3b2t6yMAYN/olUazsaL+yyEn9\n"
"WprKASOshIArAoyZl+tJaox118fessmXn1hIVw41oeQa1v1vg4Fv74zPl6/AhSrw\n"
"9U5pCZEt4Wi4wStz6dTZ/CLANx8LZh1J7QJVj2fhMtfTJr9w4z30Z209fOU0iOMy\n"
"+qduBmpvvYuR7hZL6Dupszfnw0Skfths18dG9ZKb59UhvmaSGZRVbNQpsg3BZlvi\n"
"d0lIKO2d1xozclOzgjXPYovJJIultzkMu34qQb9Sz/yilrbCgj8=\n"
"-----END CERTIFICATE-----\n";




CloudSpeechClient::CloudSpeechClient(Authentication authentication) {
     this->authentication = authentication;
    WiFi.begin(ssId, pasword);
    while (WiFi.status() != WL_CONNECTED) delay(1000);
    client.setCACert(root_Ca);
    
    // Set timeout to 30 seconds
    client.setTimeout(30000); 
    
    if (!client.connect(serVer, 443)) {
        Serial.println("Connection failed!");
    }
}



// Modify PrintHttpBody2 to send data in smaller chunks
// Send WAV data in smaller, more manageable chunks
void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
    String buffer;
    buffer.reserve(512); // Adjust the buffer size to 512 bytes

    // Encode and buffer the padded header
    String enc = base64::encode(reinterpret_cast<const uint8_t*>(audio->paddedHeader), sizeof(audio->paddedHeader));
    enc.replace("\n", "");
    buffer += enc;

    // Encode and buffer the WAV data in chunks
    for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
        enc = base64::encode(reinterpret_cast<const uint8_t*>(audio->wavData[j]), audio->dividedWavDataSize);
        enc.replace("\n", "");
        buffer += enc;

        // Send buffer when it reaches 512-byte size
        if (buffer.length() >= 512) {
            Serial.println("Sending chunk to server...");
            client.print(buffer);  // Send the buffer
            Serial.println("Chunk sent.");
            buffer = "";           // Clear the buffer for the next chunk
        }
    }

    // Send any remaining data in the buffer
    if (buffer.length() > 0) {
        Serial.println("Sending last chunk to server...");
        client.print(buffer);
        Serial.println("Last chunk sent.");
    }
}


CloudSpeechClient::~CloudSpeechClient() {
    // Clean up resources here if necessary
    Serial.println("CloudSpeechClient Destructor Called");
    client.stop();  // Close the client connection
}

void CloudSpeechClient::Transcribe(Audio* audio) {
    Serial.println("Transcription started...");

    if (!client.connect(serVer, 443)) {
        Serial.println("Connection failed!");
        return;
    }

    Serial.println("Connected to server");

    // For testing purposes, send "Hello World" to see if the connection works
    client.print("Hello World");
    Serial.println("Hello World sent.");

    // Wait for the response
    unsigned long startTime = millis();
    while (!client.available()) {
        if (millis() - startTime > 5000) {  // 5-second timeout
            Serial.println("No response from server.");
            return;
        }
        delay(100);
    }

    String My_Answer = "";
    while (client.available()) {
        char temp = client.read();
        My_Answer += temp;
    }

    Serial.println("Response received");
    Serial.print("Server Response: ");
    Serial.println(My_Answer);
}

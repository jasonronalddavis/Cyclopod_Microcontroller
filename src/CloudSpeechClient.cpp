#include "CloudSpeechClient.h"
#include "network_param.h"
#include <base64.h>
#include <ArduinoJson.h>







const char* ssId = "iPhone";
const char* pasword = "Barnes!!";

// API Key
const char* apiKeyy = "AIzaSyB_eFvcXt8CfCLd-fMd83Ze0bcwHRBdFFc";

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

String ans;


  
  this->authentication = authentication;
  WiFi.begin(ssId, pasword);
  while (WiFi.status() != WL_CONNECTED) delay(1000);
  client.setCACert(root_Ca);
  if (!client.connect(server, 443)) Serial.println("Connection failed!");
}

CloudSpeechClient::~CloudSpeechClient() {
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(Audio* audio) {
  // Base64 encode the padded header
    String enc = base64::encode((uint8_t*)audio->paddedHeader, sizeof(audio->paddedHeader));
    enc.replace("\n", "");  // Remove any newline characters
    client.print(enc);      // Send encoded header

    // Encode and send each chunk of WAV data
    char** wavData = audio->wavData;
    for (int j = 0; j < audio->wavDataSize / audio->dividedWavDataSize; ++j) {
        enc = base64::encode((uint8_t*)wavData[j], audio->dividedWavDataSize);
        enc.replace("\n", "");  // Remove any newline characters
        client.print(enc);      // Send each encoded chunk
    }
}

void CloudSpeechClient::Transcribe(Audio* audio) {
    // Base64-encode the audio data
    audio->EncodeToBase64();
    String My_Answer = "";

    // Create the body for the request
    String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-US\"},\"audio\":{\"content\":\"";
    String HttpBody3 = "\"}}\r\n\r\n";

    // Calculate the base64-encoded data length
    int httpBody2Length = (audio->wavDataSize + sizeof(audio->paddedHeader)) * 4 / 3; // 4/3 is from base64 encoding
    String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());

    // Construct the full HTTP header with the correct content length
    String fullHttpHeader = String("POST /v1/speech:recognize?key=") + apiKeyy +
                            " HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: " + ContentLength + "\r\n\r\n";

    // Send the HTTP header and body to the server
    client.print(fullHttpHeader);
    client.print(HttpBody1);
    PrintHttpBody2(audio);  // Encode and send the audio data
    client.print(HttpBody3);

    // Wait for and capture the response
    while (client.available()) {
        char temp = client.read();
        My_Answer += temp;
    }

    // Extract JSON data from the response
    int position = My_Answer.indexOf('{');
    if (position != -1) {
        String ans = My_Answer.substring(position);
        Serial.print("JSON data: ");
        Serial.println(ans);
    } else {
        Serial.println("No JSON data found in the response");
    }
}

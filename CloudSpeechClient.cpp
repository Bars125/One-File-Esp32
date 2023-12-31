/*#include "network_param.h"
#include "CloudSpeechClient.h"
#include <base64.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

const char* server = "speech.googleapis.com";
// Certificate for specific region, run:
// openssl s_client -showcerts -connect speech.googleapis.com:443
const char* root_cert =
  "-----BEGIN CERTIFICATE-----\n"
  "MIIFljCCA36gAwIBAgINAgO8U1lrNMcY9QFQZjANBgkqhkiG9w0BAQsFADBHMQsw\n"
  "CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU\n"
  "MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMjAwODEzMDAwMDQyWhcNMjcwOTMwMDAw\n"
  "MDQyWjBGMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp\n"
  "Y2VzIExMQzETMBEGA1UEAxMKR1RTIENBIDFDMzCCASIwDQYJKoZIhvcNAQEBBQAD\n"
  "ggEPADCCAQoCggEBAPWI3+dijB43+DdCkH9sh9D7ZYIl/ejLa6T/belaI+KZ9hzp\n"
  "kgOZE3wJCor6QtZeViSqejOEH9Hpabu5dOxXTGZok3c3VVP+ORBNtzS7XyV3NzsX\n"
  "lOo85Z3VvMO0Q+sup0fvsEQRY9i0QYXdQTBIkxu/t/bgRQIh4JZCF8/ZK2VWNAcm\n"
  "BA2o/X3KLu/qSHw3TT8An4Pf73WELnlXXPxXbhqW//yMmqaZviXZf5YsBvcRKgKA\n"
  "gOtjGDxQSYflispfGStZloEAoPtR28p3CwvJlk/vcEnHXG0g/Zm0tOLKLnf9LdwL\n"
  "tmsTDIwZKxeWmLnwi/agJ7u2441Rj72ux5uxiZ0CAwEAAaOCAYAwggF8MA4GA1Ud\n"
  "DwEB/wQEAwIBhjAdBgNVHSUEFjAUBggrBgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0T\n"
  "AQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQUinR/r4XN7pXNPZzQ4kYU83E1HScwHwYD\n"
  "VR0jBBgwFoAU5K8rJnEaK0gnhS9SZizv8IkTcT4waAYIKwYBBQUHAQEEXDBaMCYG\n"
  "CCsGAQUFBzABhhpodHRwOi8vb2NzcC5wa2kuZ29vZy9ndHNyMTAwBggrBgEFBQcw\n"
  "AoYkaHR0cDovL3BraS5nb29nL3JlcG8vY2VydHMvZ3RzcjEuZGVyMDQGA1UdHwQt\n"
  "MCswKaAnoCWGI2h0dHA6Ly9jcmwucGtpLmdvb2cvZ3RzcjEvZ3RzcjEuY3JsMFcG\n"
  "A1UdIARQME4wOAYKKwYBBAHWeQIFAzAqMCgGCCsGAQUFBwIBFhxodHRwczovL3Br\n"
  "aS5nb29nL3JlcG9zaXRvcnkvMAgGBmeBDAECATAIBgZngQwBAgIwDQYJKoZIhvcN\n"
  "AQELBQADggIBAIl9rCBcDDy+mqhXlRu0rvqrpXJxtDaV/d9AEQNMwkYUuxQkq/BQ\n"
  "cSLbrcRuf8/xam/IgxvYzolfh2yHuKkMo5uhYpSTld9brmYZCwKWnvy15xBpPnrL\n"
  "RklfRuFBsdeYTWU0AIAaP0+fbH9JAIFTQaSSIYKCGvGjRFsqUBITTcFTNvNCCK9U\n"
  "+o53UxtkOCcXCb1YyRt8OS1b887U7ZfbFAO/CVMkH8IMBHmYJvJh8VNS/UKMG2Yr\n"
  "PxWhu//2m+OBmgEGcYk1KCTd4b3rGS3hSMs9WYNRtHTGnXzGsYZbr8w0xNPM1IER\n"
  "lQCh9BIiAfq0g3GvjLeMcySsN1PCAJA/Ef5c7TaUEDu9Ka7ixzpiO2xj2YC/WXGs\n"
  "Yye5TBeg2vZzFb8q3o/zpWwygTMD0IZRcZk0upONXbVRWPeyk+gB9lm+cZv9TSjO\n"
  "z23HFtz30dZGm6fKa+l3D/2gthsjgx0QGtkJAITgRNOidSOzNIb2ILCkXhAd4FJG\n"
  "AJ2xDx8hcFH1mt0G/FX0Kw4zd8NLQsLxdxP8c4CU6x+7Nz/OAipmsHMdMqUybDKw\n"
  "juDEI/9bfU1lcKwrmz3O2+BtjjKAvpafkmO8l7tdufThcV4q5O8DIrGKZTqPwJNl\n"
  "1IXNDw9bg1kWRxYtnCQ6yICmJhSFm/Y3m6xv+cXDBlHz4n/FsRC6UfTd\n"
  "-----END CERTIFICATE-----\n";

CloudSpeechClient::CloudSpeechClient(Authentication authentication) {

  this->authentication = authentication;
  Serial.println("47 Authentication CloudSpeechClient");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("50 CloudSpeechClient: Wifi isn't connected.");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    delay(1000);
  }

  client.setCACert(root_cert);
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
}

//String ans;
CloudSpeechClient::~CloudSpeechClient() {
  Serial.println("63 Delete CloudSpeechClient");
  client.stop();
  WiFi.disconnect();
}

void CloudSpeechClient::PrintHttpBody2(File file) {
  const int bufferSize = 512;
  byte buffer[bufferSize];
  size_t bytesRead;
  Serial.println("70 CloudSpeechClient PrintHttpBody2");

  // Read and encode the file content in chunks
  while (file.available() && client.connected()) {
    //Serial.println("74 CloudSpeechClient. File Avaliable");  /// ????????????
    bytesRead = file.read(buffer, bufferSize);
    if (bytesRead > 0) {
      String enc = base64::encode(buffer, bytesRead);
      enc.replace("\n", "");  // delete last "\n"
      client.print(enc);      // HttpBody2
    }
  }
}

void CloudSpeechClient::Transcribe(File file) {
  Serial.println("85 CloudSpeechClient - Transcribe");
  String HttpBody1 = "{\"config\":{\"encoding\":\"LINEAR16\",\"sampleRateHertz\":16000,\"languageCode\":\"en-IN\"},\"audio\":{\"content\":\"";
  String HttpBody3 = "\"}}\r\n\r\n";
  int httpBody2Length = file.size() * 4 / 3;  // 4/3 is from base64 encoding
  String ContentLength = String(HttpBody1.length() + httpBody2Length + HttpBody3.length());
  String HttpHeader;

  HttpHeader = String("POST /v1/speech:recognize?key=") + ApiKey + String(" HTTP/1.1\r\nHost: speech.googleapis.com\r\nContent-Type: application/json\r\nContent-Length: ") + ContentLength + String("\r\n\r\n");

  client.print(HttpHeader);
  client.print(HttpBody1);
  PrintHttpBody2(file);
  client.print(HttpBody3);

  // Wait for response N seconds
  unsigned long timeout = millis();
  while (!client.available() && (millis() - timeout < 10000)) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");

  //client.available() function checks if there is data available to be read from the client.
  //It returns the number of bytes available. If no data is available, it returns 0
  String response;
  while (client.available()) {
    char temp = client.read();
    response += temp;
  }

  // Find the starting position of the recognized text in the response
  int position = response.indexOf("transcript");
  if (position != -1) {
    // Extract the recognized text
    String transcript = response.substring(position + 13);  // "transcript" is 13 characters long
    transcript = transcript.substring(0, transcript.indexOf("\""));

    // Output the recognized text to the Serial Monitor
    Serial.print("Recognized Text: ");
    Serial.println(transcript);
  } else {
    Serial.println("Error: Unable to extract recognized text from response.");
  }
  /*
  if (!client.available()) {
    Serial.println("Client Not available. Waiting...");
    while (!client.available()) {}
  } else {
    Serial.println("Client available.");

    while (client.available()) {
      if (client.read() > 0) {
        Serial.println("114 client-read() > 0");
        char temp = client.read();
        Response += temp;
      } else {
        break;
      }
    }
  }
  
  Serial.print("My Answer - ");
  Serial.println(My_Answer);
  int postion = My_Answer.indexOf('{');
  Serial.println(postion);
  ans = My_Answer.substring(postion);
  Serial.print("Json data - ");
  Serial.print(ans);
  
}*/


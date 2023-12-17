/*#ifndef _CLOUDSPEECHCLIENT_H
#define _CLOUDSPEECHCLIENT_H

#include <WiFiClientSecure.h>
#include <FS.h>

enum Authentication {
  USE_ACCESSTOKEN,
  USE_APIKEY
};

class CloudSpeechClient {
  WiFiClientSecure client;
  void PrintHttpBody2(File file);
  Authentication authentication;

public:
  CloudSpeechClient(Authentication authentication);
  ~CloudSpeechClient();
  void Transcribe(File file);
};

#endif // _CLOUDSPEECHCLIENT_H

*/
#ifndef _NETWORK_PARAM_H
#define _NETWORK_PARAM_H

#define WIFI_SSID "Galaxy A526AD7"
#define WIFI_PASSWORD "pqmm9039"

#include <Arduino.h>



// Getting Access Token : 
// At first, you should get service account key (JSON file).
// Type below command in Google Cloud Shell to get AccessToken: 
// $ gcloud auth activate-service-account --key-file=KEY_FILE   (KEY_FILE is your service account key file)
// $ gcloud auth print-access-token
// The Access Token is expired in an hour.
// Google recommends to use Access Token.
//const String AccessToken = "";

// It is also possible to use "API Key" instead of "Access Token". It doesn't have time limit.
//const String ApiKey = "Your_API_Key";

#endif  // _NETWORK_PARAM_H

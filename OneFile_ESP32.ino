#define record_Pin 27          // record audio while holding a button
#define THRESHOLD 40           // Greater the value, more the sensitivity (wake-up Pin)
#define wifi_state_Led 26      // wifi status LED
#define Sending_record_Led 14  // recording finished, sending
#define waitTime 10000         // after this time ESP will fall asleep

#define WIFI_SSID "Galaxy A526AD7"
#define WIFI_PASSWORD "pqmm9039"

#define I2S_SD GPIO_NUM_32
#define I2S_WS GPIO_NUM_25
#define I2S_SCK GPIO_NUM_33

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE (16000)
#define I2S_SAMPLE_BITS (16)
#define I2S_READ_LEN (16 * 1024)
#define RECORD_TIME (5)  //Seconds
#define I2S_CHANNEL_NUM (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

#include <Arduino.h>
#include <driver/i2s.h>
#include <Wire.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <FS.h>

File file;
const char filename[] = "/recording.wav";
const int headerSize = 44;
bool isWIFIConnected = false;
bool isRecorded = false;

//func prototypes

void SPIFFSInit();
void wavHeader(byte* header, int wavSize);
void listSPIFFS();
void customDelay(unsigned int milliseconds);
void print_wakeup_touchpad();
void start_deep_sleep();
void i2s_Init();
void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len);
void i2s_adc();
void uploadFileGoogle();
void connectToWiFi();

void setup() {

  // Setting up the Serial Monitor
  Serial.begin(115200);
  customDelay(500);
  Serial.println("Serial Monitor is ON");

  // Led indication setup
  pinMode(wifi_state_Led, OUTPUT);
  pinMode(Sending_record_Led, OUTPUT);
  pinMode(record_Pin, INPUT_PULLUP);
  digitalWrite(wifi_state_Led, 0);  // wifi status LED make sure it's 0
  digitalWrite(Sending_record_Led, 0);
  // Wake up settings
  print_wakeup_touchpad();
  touchSleepWakeUpEnable(T3, THRESHOLD);  // Setup sleep, wakeup on Touch Pad 3 (GPIO15)

  // ===== Programm  starts Here =====

  SPIFFSInit();  // initialize the SPI Flash File System
  i2s_Init();    // initialize the I2S interface

  unsigned int startTime = millis();
  int Count = 10;

  // main cycle: recoding after pressing the button; quit when wifi conn-ion is lost;
  while (1) {

    if (WiFi.status() != WL_CONNECTED) {
      connectToWiFi();
    }

    do {
      if (digitalRead(record_Pin) == LOW) {  // when the button was clicked
        Serial.println("Record Button's Pressed!");
        i2s_adc();
        break;
      } else {
        Serial.println("Left to Sleep: " + String(Count--));
        //Count--;
        customDelay(1000);
      }
    } while (Count != 0);

    if (isWIFIConnected && isRecorded) {
      digitalWrite(Sending_record_Led, 1);
      uploadFileGoogle();
      break;
    } else {
      Serial.println("CHAO-1 (when there was no record)!");
      customDelay(500);
      start_deep_sleep();
    }
  }
  Serial.println("CHAO-2 (main cycle, after recording)!");
  customDelay(500);
  start_deep_sleep();
}

void customDelay(unsigned int milliseconds) {
  unsigned int startTime = millis();
  while (millis() - startTime < milliseconds) {
  }
}

void print_wakeup_touchpad() {
  touch_pad_t touchPin = esp_sleep_get_touchpad_wakeup_status();
  if (touchPin == 3) {
    Serial.println("Wakeup caused by touchpad on GPIO 15");
  } else {
    Serial.println("Wakeup not by touchpad");
  }
}

void start_deep_sleep() {
  Serial.println("");
  Serial.println("Going to sleep now");
  customDelay(100);
  esp_deep_sleep_start();
}

void i2s_Init() {
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 64,
    .dma_buf_len = 1024,
    .use_apll = 1
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
}

void i2s_adc_data_scale(uint8_t* d_buff, uint8_t* s_buff, uint32_t len) {
  uint32_t j = 0;
  uint32_t dac_value = 0;
  for (int i = 0; i < len; i += 2) {
    dac_value = ((((uint16_t)(s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
    d_buff[j++] = 0;
    d_buff[j++] = dac_value * 256 / 2048;
  }
}

void i2s_adc() {

  int i2s_read_len = I2S_READ_LEN;
  int flash_wr_size = 0;
  size_t bytes_read;

  char* i2s_read_buff = (char*)calloc(i2s_read_len, sizeof(char));
  uint8_t* flash_write_buff = (uint8_t*)calloc(i2s_read_len, sizeof(char));

  i2s_read(I2S_PORT, (void*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
  //i2s_read(I2S_PORT, (void*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);

  Serial.println(" *** Recording Start *** ");

  while (flash_wr_size < FLASH_RECORD_SIZE) {
    //read data from I2S bus, in this case, from ADC.
    i2s_read(I2S_PORT, (void*)i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
    //save original data from I2S(ADC) into flash.
    i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, i2s_read_len);
    file.write((const byte*)flash_write_buff, i2s_read_len);
    flash_wr_size += i2s_read_len;
    Serial.printf("Sound recording %u%%\n", (flash_wr_size * 100 / FLASH_RECORD_SIZE));
    //Serial.printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
  }
  isRecorded = true;
  file.close();

  free(i2s_read_buff);
  i2s_read_buff = NULL;
  free(flash_write_buff);
  flash_write_buff = NULL;

  listSPIFFS();
}

void uploadFileGoogle() {
  file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    Serial.println("FILE IS NOT AVAILABLE!");
    return;
  }

  //sending
  Serial.println("File is sended Nahui !");
  // Читаем данные из файла и передаем их в I2S-интерфейс
  /* while (file.available()) {
    size_t available_bytes = file.available();
    size_t bytes_to_read = available_bytes > 1024 ? 1024 : available_bytes;  // Читаем не более 1024 байт за раз

    uint8_t buffer[bytes_to_read];
    size_t bytes_read = file.read(buffer, bytes_to_read);

    if (bytes_read > 0) {
      i2s_write(I2S_PORT, buffer, bytes_read, portMAX_DELAY);
    }
  }*/

  file.close();
}

void connectToWiFi() {
  isWIFIConnected = false;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  for (byte i; i < 10; i++) {
    Serial.println("Connecting to WiFi...");
    customDelay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      isWIFIConnected = true;
      digitalWrite(wifi_state_Led, 1);
      Serial.println("Connected to Wi-Fi");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      break;
    }
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("CHAO-3 (Wifi-func)!");
    customDelay(500);
    start_deep_sleep();
  }

  /*isWIFIConnected = false;
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    customDelay(500);
    Serial.println("Connecting to WiFi...");
  }

  isWIFIConnected = true;
  digitalWrite(wifi_state_Led, 1);
  Serial.println("Connected to Wi-Fi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());*/
}

void SPIFFSInit() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS initialisation failed!");
  }

  SPIFFS.remove(filename);
  file = SPIFFS.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("File is not available!");
  }

  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);

  file.write(header, headerSize);
  listSPIFFS();
}

void wavHeader(byte* header, int wavSize) {
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;
  header[21] = 0x00;
  header[22] = 0x01;
  header[23] = 0x00;
  header[24] = 0x80;
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;
  header[29] = 0x7D;
  header[30] = 0x01;
  header[31] = 0x00;
  header[32] = 0x02;
  header[33] = 0x00;
  header[34] = 0x10;
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);
}

void listSPIFFS() {
  Serial.println(F("\r\nListing SPIFFS files:"));
  static const char line[] PROGMEM = "";

  Serial.println(FPSTR(line));
  Serial.println(F("File name Size"));
  Serial.println(FPSTR(line));

  fs::File root = SPIFFS.open("/");
  if (!root) {
    Serial.println(F("Failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(F("Not a directory"));
    return;
  }

  fs::File file = root.openNextFile();
  while (file) {

    if (file.isDirectory()) {
      Serial.print("DIR : ");
      String fileName = file.name();
      Serial.print(fileName);
    } else {
      String fileName = file.name();
      Serial.print("  " + fileName);
      // File path can be 31 characters maximum in SPIFFS
      int spaces = 33 - fileName.length();  // Tabulate nicely
      if (spaces < 1)
        spaces = 1;
      while (spaces--) Serial.print(" ");
      String fileSize = (String)file.size();
      spaces = 10 - fileSize.length();  // Tabulate nicely
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      Serial.println(fileSize + " bytes");
    }

    file = root.openNextFile();
  }

  Serial.println(FPSTR(line));
  Serial.println();
  customDelay(1000);
}

void loop() {
}
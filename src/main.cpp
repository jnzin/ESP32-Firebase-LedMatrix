#include <WiFi.h>
#include <string.h>
#include <FirebaseESP32.h>
#include <addons/TokenHelper.h>
#include <Wire.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define WIFI_SSID "Wokwi-GUEST"
#define WIFI_PASSWORD ""

// #define WIFI_SSID "Galaxy S10e0604"
// #define WIFI_PASSWORD "jonascz123"


#define FIREBASE_DATABASE_URL "https://placarteste-f3d3f-default-rtdb.firebaseio.com"
#define FIREBASE_TOPIC "/placar/texto_placar"
#define FIREBASE_API_KEY "AIzaSyDDxxT35nu6DoidikMpza6n_c339J-8Dis"

#define MAX_LED_MATRIX_DEVICES 3
// #define LED_MATRIX_HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define LED_MATRIX_HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define LED_MATRIX_CS_PIN 21
#define LED_MATRIX_CLK_PIN 18
#define LED_MATRIX_DATA_PIN 23

#define ANIMATION_SPEED 80
#define ANIMATION_PAUSE_FRAME 0
#define ANIMATION_EFFECT PA_SCROLL_LEFT


MD_Parola Display = MD_Parola(LED_MATRIX_HARDWARE_TYPE, LED_MATRIX_DATA_PIN, LED_MATRIX_CLK_PIN, LED_MATRIX_CS_PIN, MAX_LED_MATRIX_DEVICES);

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

String newText = "";
String currentText = "";
bool newTextAvaible = true;

void setDisplayText(String h_text) {
  Serial.print("Data: ");
  Serial.println(h_text.c_str());

  newText = h_text.c_str();
  newTextAvaible = true;
}

void streamCallback(StreamData data)
{
  if (data.dataTypeEnum() == firebase_rtdb_data_type_string)
  {
    String dataText = data.to<String>();

    if (dataText != "")
      setDisplayText(dataText);
  }
}

void streamTimeoutCallback(bool timeout)
{
  if(timeout){
    Serial.println("Stream timeout, resume streaming...");
  }
}



void setup() {
  Serial.begin(115200);

  Display.begin();
  Display.setIntensity(0);
  Display.displayClear();
  Display.displayText(currentText.c_str(), PA_CENTER, ANIMATION_SPEED, ANIMATION_PAUSE_FRAME, ANIMATION_EFFECT, ANIMATION_EFFECT);
  

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to Wi-Fi... ");

  while (WiFi.status() != WL_CONNECTED)
  {
      delay(300);
  }
  
  Serial.println("ok");

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  config.database_url = FIREBASE_DATABASE_URL;
  config.api_key = FIREBASE_API_KEY;
  config.token_status_callback = tokenStatusCallback;

  Firebase.reconnectNetwork(true);

  fbdo.setBSSLBufferSize(4096, 1024);

  Serial.print("Sign up new user... ");

  if (Firebase.signUp(&config, &auth, "", ""))
  {
      Serial.println("ok");
  }
  else
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);

  if (!Firebase.beginStream(fbdo, FIREBASE_TOPIC))
  {
    Serial.println(fbdo.errorReason());
  }
  
  Firebase.setStreamCallback(fbdo, streamCallback, streamTimeoutCallback);

  Serial.println("Firebase initialized.");
}

void loop()
{
  if (Display.displayAnimate())
  {
    if (newTextAvaible)
    {
      currentText = newText;
      newTextAvaible = false;
    }

    Display.displayReset();
  }

  if (Firebase.ready()) {
   // ready
  }
} 
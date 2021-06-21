// ---------------------------------------------------------------
//                  -SETTINGS for HouseMonitor
// ---------------------------------------------------------------
//    best gpio pins to use:  D1, D2, D5, D6, D7.


  const char* stitle = "HouseMonitor";                   // title of this sketch
  
  #define _SenderName "HouseMonitor"                     // name of email sender (no spaces)

  const int soundFor = 10;                               // how long the sounder should trigger for (seconds)

  const int EmailAttemptTime = 60;                       // how often to re-attempt failed email sends (seconds)
  const int MaxEmailAttempts = 6;                        // maximum email send attempts   
  
  const uint16_t tempCheckTime = 120;                    // How often to check temperature / humidity is ok (seconds)

  const uint16_t smokeCheckTime = 500;                   // How often to check if smoke detector is triggering (milliseconds)

  const uint16_t movementCheckTime = 200;                // How often to check if radar detector has triggered (milliseconds)

  const bool tempSensing = 0;                            // flag if a temperature sensor is installed
  const int tempSensorPin = D2;                          // dht22 temperature sensor io pin

  const bool smokeDetection = 0;                         // If a smoke detector is attached or not
  const int smokeAlarmPin = D5;                          // pin goes high when smoke alarm is triggering

  const bool sounderAttached = 0;                        // flag if a sounder is present
  const int sounderPin = D7;                             // gpio pin the sounder is on 

  const bool radarDetection = 1;                         // If there is a radar sensor attached
  const int movementSensorPin = D1;                      // radar movement sensor gpio pin

  #define ENABLE_EMAIL 0                                 // Enable E-mail  
  
  #define ENABLE_OTA 1                                   // Enable Over The Air updates (OTA)
  const String OTAPassword = "12345678";                 // Password to enable OTA service (supplied as - http://<ip address>?pwd=xxxx )

  const char HomeLink[] = "/";                           // Where home button on web pages links to (usually "/")

  const char datarefresh[] = "4000";                     // Refresh rate of the updating data on web page (1000 = 1 second)
  const char JavaRefreshTime[] = "500";                  // time delay when loading url in web pages (Javascript)
  
  const byte LogNumber = 100;                            // number of entries to store in the system log

  const int ServerPort = 80;                             // HTTP port to use

  const int lastWebAccessDelay = 3000;                   // Pause radar sensing for this long when wifi is accessed to stop false triggers (ms)

  const byte led = D0;                                   // indicator LED pin - D0/D4 on esp8266 nodemcu, 3 on esp8266-01, 2 on ESP32

  const bool ledBlinkEnabled = 1;                        // If blinking status light is enabled
  const uint16_t ledBlinkRate = 1500;                    // Speed to blink the status LED and carry out some system tasks (milliseconds) 

  const boolean ledON = LOW;                             // Status LED control 
  const boolean ledOFF = HIGH;
  

// ---------------------------------------------------------------

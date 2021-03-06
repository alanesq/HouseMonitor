/*******************************************************************************************************************
 *
 *                             HouseMonitor - monitor house and report any unusual events
 * 
 *                                     https://github.com/alanesq/HouseMonitor
 *                              
 *                           Included files: email.h, Mstandard.h, ota.h, wifi.h and any settings.h files
 *             
 *                           Access via: http://x.x.x.x:6969/pine
 *                           Uses a smoke detector on pin D5 (High = triggering)
 *                           DHT22 temperature/humidity sensor on D2
 *                           RCWL-0516 Radar movement sensor on D1 (High = triggering)
 *                           
 *                           Enable/disable emails toggle with        http://x.x.x.x:6969/pine?email
 *                           Change 'first movement in' setting with  http://x.x.x.x:6969/pine?first=x
 *                           Change 'no movement in' setting with     http://x.x.x.x:6969/pine?none=x
 *                           Change radar trigger level               http://x.x.x.x:6969/pine?trigger=x
 *                           Create test graph data with              http://x.x.x.x:6969/pine?testdata
 *                           Clear graph data with                    http://x.x.x.x:6969/pine?cleardata
 *                           Clear logs                               http://x.x.x.x:6969/pine?clearlog
 *                           Change high temp warning level           http://x.x.x.x:6969/pine?hightemp
 *                           Change low temp warning level            http://x.x.x.x:6969/pine?lowtemp
 *                           Load default settings                    http://x.x.x.x:6969/pine?defaults
 *                           Toggle show all radar triggers in log    http://x.x.x.x:6969/pine?all
 *                                                                              
 *      Note:  To add ESP8266/32 ability to the Arduino IDE enter the below two lines in to FILE/PREFERENCES/BOARDS MANAGER
 *                 http://arduino.esp8266.com/stable/package_esp8266com_index.json
 *                 https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
 *             You can then add them in TOOLS/BOARD/BOARDS MANAGER (search for esp8266 or ESP32)
 *          
 *      First time the ESP starts it will create an access point "ESPPortal" which you need to connect to in order to enter your wifi details.  
 *             default password = "12345678"   (change this in wifi.h)
 *             see: https://randomnerdtutorials.com/wifimanager-with-esp8266-autoconnect-custom-parameter-and-manage-your-ssid-and-password
 *             
 *                                                                                      Created by: www.alanesq.eu5.net
 *      
 ********************************************************************************************************************/

#if (!defined ESP8266)
  #error This sketch is for esp8266 only
#endif


#include "DHTesp.h"         // DHT22 temperature sensor
#include <ESP_EEPROM.h>     // eeprom storage



// ---------------------------------------------------------------
//                             SETTINGS
// ---------------------------------------------------------------


const char* sversion = "21Jun21";        // version of this sketch

const bool serialDebug = 1;              // provide debug info on serial port
const int serialSpeed = 115200;          // Serial data speed to use (74880 will show ESP8266 boot diagnostics?)


#include "settings.h"         // Import sketch settings 
  
  

// ---------------------------------------------------------------


#define ENABLE_OLED 0                                  // Enable OLED display  (not used)

#define ENABLE_GSM 0                                   // Enable GSM board support  (not used)

  
// flags / variables specific to HouseMonitor sketch
bool showAllRadarTriggers = 0;          // if set then all radar triggers will show in the logs (even below movement trigger level)
bool smokeAlarmTriggered = 0;           // goes high when the smoke alarm has triggered
bool tempWarningTriggered = 0;          // goes high when a low/high temperature warning has been issued
bool noMovementWarningTriggered = 0;    // goes high when a no recent movement deteted warning has triggered
float lowestTemp = 0;                   // lowest recorded temperature
float highestTemp = 0;                  // highest recorded temperature
float lowestHumidity = 0;               // lowest recorded humidity
float highestHumidity = 0;              // highest recorded humidity
String lastMovementDetection = "n/a";   // last time movement was detected
bool lastMovementPinState = 0;          // previous state of movement detector gpio pin
float temperatures[24];                 // log previous 24hrs temperature readings (displayed as a graph)
byte logCounter=0;                      // current position in temperature log
uint32_t tempLogTimer = millis();       // timer for temperature log
String lastLogUpdateTime = "n/a";       // last time log was updated
int movements[24];                      // log previous 24hrs movements detected per hour (displayed as a graph)
int movementCounts = 0;                 // movement detections counter (for hourly log)
byte emailWarningMode = 0;              // email/sms sending upon a warning
                                        //    0=no emails/sms sending, 1=send emails, 2=send emails and also sms if it is dark, 3=send both email and sms
uint16_t radarTriggerTriplevel;         // Time radar needs to be triggered to count as motion detected
uint16_t motionFirstTrigger;            // if first motion detected in this many hours send an email
uint16_t noMovementTriggerTime;         // if no movement detected for this many hours trigger a warning 
float lowTempWarningLevel = 10.0;       // Temperature  below which triggers a warning (C)
float highTempWarningLevel = 30.0;      // Temperature  above which triggers a warning (C)
bool overideNoSMS = 0;                  // used to send sms for smoke alarm even if sms sending is disabled
bool justStarted = 1;                   // flag that the esp has just booted (set to 0 after 30 seconds)

bool OTAEnabled = 0;                    // flag if OTA has been enabled (via supply of password)
bool GSMconnected = 0;                  // flag if the gsm module is connected ok
bool wifiok = 0;                        // flag if wifi connection is ok

uint32_t LEDtimer = millis();           // used for flashing the LED
uint32_t TEMPtimer = millis();          // used for timing temperature checks
uint32_t SMOKEtimer = millis();         // used for timing smoke detector checks
uint32_t MOTIONtimer = millis();        // used for timing movement sensor checks 
uint32_t MOTIONdetected = 0;            // last time movement was detected 
uint32_t MOTIONdetectedEvent = millis();// last motion detected event (0 = no delay at boot)
uint32_t MOTIONonTimer = 0;             // time motion detector was triggered for
uint32_t movementDetectionTimer = millis();  // timer for radar movement detection rolling total
uint32_t lastWebAccess = millis();      // log time of last web access (used to supress false radar triggers)
uint32_t sounderTriggerTime = 0;        // log time the sounder was turned on

#include "wifi.h"                       // Load the Wifi / NTP stuff

#include "Mstandard.h"                  // Some standard procedures

#if ENABLE_OTA
  #include "ota.h"                      // Over The Air updates (OTA)
#endif

#if ENABLE_GSM
  #include "gsm.h"                      // GSM board 
#endif

#if ENABLE_OLED
  #include "oled.h"                     // OLED display - i2c version SSD1306
#endif

#if ENABLE_EMAIL
    // #define _SenderName "DadsHouseMonitor"         // name of email sender (no spaces)
    #include "email.h"
    bool emailToSend = 0;                   // set to 1 when there is an email waiting to be sent
    char _recepient[60];                    // email address to send to
    uint32_t lastEmailAttempt = 0;          // last time sending of an email was attempted
    int emailAttemptCounter = 0;            // counter for failed email attempts

#endif


DHTesp dht;



// ---------------------------------------------------------------
//    -SETUP     SETUP     SETUP     SETUP     SETUP     SETUP
// ---------------------------------------------------------------
// setup section (runs once at startup)

void setup() {
    
  if (serialDebug) {
    Serial.begin(serialSpeed); while (!Serial); delay(200);       // start serial comms                                   // serial port
    delay(200);

    Serial.println("\n\n\n");                                    // line feeds
    Serial.println("-----------------------------------");
    Serial.printf("Starting - %s - %s \n", stitle, sversion);
    Serial.println("-----------------------------------");
    Serial.println( "Device type: " + String(ARDUINO_BOARD) + ", chip id: " + String(ESP_getChipId(), HEX));
    // Serial.println(ESP.getFreeSketchSpace());
    #if defined(ESP8266)     
        Serial.println("Chip ID: " + ESP.getChipId());
        rst_info *rinfo = ESP.getResetInfoPtr();
        Serial.println("ResetInfo: " + String((*rinfo).reason) + ": " + ESP.getResetReason());
        Serial.printf("Flash chip size: %d (bytes)\n", ESP.getFlashChipRealSize());
        Serial.printf("Flash chip frequency: %d (Hz)\n", ESP.getFlashChipSpeed());
        Serial.printf("\n");
    #endif
  }

  EEPROM.begin(32);    // initialise eeprom storage (size of storage required)
  readEEPROM();        // read in variables from eeprom

  // start temperature sensor
    dht.setup(tempSensorPin, DHTesp::DHT22);                     // Connect DHT sensor 
 
  // if (serialDebug) Serial.setDebugOutput(true);               // enable extra diagnostic info  
   
  // configure the onboard LED
    pinMode(led, OUTPUT); 
    digitalWrite(led, ledON);                                    // led on until wifi has connected
    
  // configure smoke alarm gpio
    pinMode(smokeAlarmPin, INPUT);

  // configure the sounder gpio pin
    if (sounderAttached) {
      pinMode(sounderPin, OUTPUT);
      digitalWrite(sounderPin , LOW);    
    }

  // configure the radar movement detector gpio
    pinMode(movementSensorPin, INPUT);

  // configure the onboard input button (nodemcu onboard, low when pressed)
    // pinMode(onboardButton, INPUT); 

  startWifiManager();      // Connect to wifi (procedure is in wifi.h)
  
  WiFi.mode(WIFI_STA);     // turn off access point - options are WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
    //    // configure as wifi access point as well
    //    Serial.println("starting access point");
    //    WiFi.softAP("ESP-AP", "password");               // access point settings (Note: password must be 8 characters for some reason - this may no longer be true?)
    //    WiFi.mode(WIFI_AP_STA);                          // enable as both Station and access point - options are WIFI_AP, WIFI_STA, WIFI_AP_STA or WIFI_OFF
    //    IPAddress myIP = WiFi.softAPIP();
    //    if (serialDebug) Serial.print("Access Point Started - IP address: ");
    //    Serial.println(myIP);
  
  // set max/min values
    lowestTemp = readTemp(0);
    highestTemp = readTemp(0);
    lowestHumidity = readTemp(1);
    highestHumidity = readTemp(1);    
    
  // set up web page request handling
    server.on(HomeLink, handleRoot);         // root page
    server.on("/data", handleData);          // This displays information which updates every few seconds (used by root web page)
    server.on("/ping", handlePing);          // ping requested
    server.on("/log", handleLogpage);        // system log
    server.on("/test", handleTest);          // testing page
    server.on("/reboot", handleReboot);      // reboot the esp
    server.on("/ota", handleOTA);            // OTA update page
    server.onNotFound(handleNotFound);       // invalid page requested
  
  // start web server
    if (serialDebug) Serial.println("Starting web server");
    server.begin();

  // log first temperature
    temperatures[0] = readTemp(0);
    if (temperatures[0] > 600.00) temperatures[0] = 0.0;   // if error reading current temp.
    logCounter = 0;
    lastLogUpdateTime = currentTime();                     // log time of last update
    
  // Stop wifi going to sleep (if enabled it causes wifi to drop out randomly especially on esp8266 boards)
    #if defined ESP8266
      WiFi.setSleepMode(WIFI_NONE_SLEEP);     
    #elif defined ESP32
      WiFi.setSleep(false);   
    #endif    

  // Finished connecting to network
    digitalWrite(led, ledOFF);                              // led off
    log_system_message("Started");

}


// ----------------------------------------------------------------
//   -LOOP     LOOP     LOOP     LOOP     LOOP     LOOP     LOOP
// ----------------------------------------------------------------

void loop(void){

    #if defined(ESP8266)
        yield();                      // allow esp8266 to carry out wifi tasks (may restart randomly without this)
    #endif
    
    server.handleClient();            // service any web page requests 

    if (tempSensing) {                // if a temp sensor is installed
      // check temperature / humidity periodically 
        if ((unsigned long)(millis() - TEMPtimer) >= (tempCheckTime * 1000) ) { 
          TEMPtimer = millis();    // reset timer
          checkTemp();             // check temperature and humidity are ok
        }
    }   // if (tempSensing)

    // check if sounder needs turning off
      if (sounderAttached && digitalRead(sounderPin) == HIGH) {
        if ( (unsigned long)(millis() - sounderTriggerTime) >= (soundFor * 1000) ) { 
          digitalWrite(sounderPin, LOW);
          log_system_message("Sounder turned off");
        }
      }
      
  // update graph data
      if ((unsigned long)(millis() - tempLogTimer) >= (1000 * 60 * 60) ) {    // once per hour
        tempLogTimer = millis();              // reset hourly timer
        lastLogUpdateTime = currentTime();    // log time of last update
        // temperature log
          logCounter++;
          if (logCounter >= 24) logCounter = 0;
          temperatures[logCounter] = readTemp(0);
          if (temperatures[logCounter] > 600.00) temperatures[logCounter] = 0.0;   // if error in reading temp.
        // movement log
          movements[logCounter] = movementCounts;
          movementCounts = 0;      // reset counter
      }      


    // check smoke alarm periodically (don't check if just booted)
      if (smokeDetection) {
        if ( (unsigned long)(millis() - SMOKEtimer) >= smokeCheckTime && !justStarted ) { 
          SMOKEtimer = millis();    // reset timer
          if (digitalRead(smokeAlarmPin) && !smokeAlarmTriggered) {     
              delay(500);
              if (digitalRead(smokeAlarmPin)) {            // debounce input
                smokeAlarm();                              // run smoke alarm has triggered procedure
              } else {
                log_system_message("Warning: False trigger detected on smoke alarm");
              }
          }
        }
      }


    // check radar pin status periodically (don't check if just booted or there has been recent web access)
      if (radarDetection) {      // if a radar sensor is installed
        if ( (unsigned long)(millis() - MOTIONtimer) >= movementCheckTime && !justStarted && (unsigned long)(millis() - lastWebAccess) >= lastWebAccessDelay ) { 
          MOTIONtimer = millis();                                // reset timer 
          bool currentPinState = digitalRead(movementSensorPin); // read current radar input pin state
  
          if (lastMovementPinState == 0) {                       // if previous state of radar pin was low
    
            // if state has changed from low to high - i.e. radar sensor has triggered
              if (currentPinState == 1) {     
                  delay(100);
                  if (digitalRead(movementSensorPin) == 1) {     // debounce
                    lastMovementPinState = 1;                    // flag pin is now high
                    MOTIONonTimer = millis();                    // log time motion sensor triggered
                    // make a noise if a sounder is attached
                      if (sounderAttached) {
                        digitalWrite(sounderPin, HIGH);          // turn sounder on
                        sounderTriggerTime = millis();           // log time sounder was turned on  
                      }                          
                  }  
              } 
    
          } else {                                // if previous state of radar pin was high
                   
            // if state has changed from high to low
              if (currentPinState == 0) {     
                  delay(100);
                  if (digitalRead(movementSensorPin) == 0) {     // debounce input
                    lastMovementPinState = 0;                    // flag pin is now low
                    // act depending how long the pin was high for (min. possible is 2 seconds)
                      uint32_t onTime = (unsigned long)((millis() - MOTIONonTimer));
                      if ( onTime >= (radarTriggerTriplevel * 1000) ) radarTriggered(onTime);   // movement detected
                      else if (showAllRadarTriggers) log_system_message("Radar triggered for " + String( (float)(onTime / 1000.0), 1 ) + " seconds");   
                  }
              }                         
          }
        }   

        // check if no movement has been detected for a long time
          if ((unsigned long)(millis() - MOTIONdetectedEvent) > (noMovementTriggerTime * 1000 * 60 * 60) ) {
            noMovementDetected();                 // run no movement detected recently procedure
          }
        
      }   //if (radarDetection)
     

    #if ENABLE_EMAIL
      // check if an email is flagged to be sent
        // if emails disabled or recently booted then cancel any scheduled send
          if (emailWarningMode == 0 || justStarted) {
            if (emailToSend) {
              emailToSend = 0;     
              log_system_message("Email send cancelled");
            }   
          }
        if (emailToSend && emailAttemptCounter < MaxEmailAttempts) {
          if (lastEmailAttempt == 0 || (unsigned long)(millis() - lastEmailAttempt) >= (EmailAttemptTime * 1000)) {    // if long enough since last try
            // try to send the email  
              // tell email.h if it should also send an sms 
                sendSMSflag = 0;    // default to no
                if (overideNoSMS) sendSMSflag = 1;    
                if (emailWarningMode == 3) sendSMSflag = 1;     // SMS enabled
                if (emailWarningMode == 2) {                    // SMS if its night-time only
                    time_t t=now();         // get time from NTP
                    if (IsBST()) t+=3600;   // add one hour if it is British Summer Time
                    int cHour = hour(t);    // get current hour
                    if (cHour > 20 || cHour < 7) sendSMSflag = 1;   
                }          
              if (sendEmail(_recepient, _subject, _message)) {
                // email sent ok
                  emailToSend = 0;                                  // clear flag that there is an email waiting to be sent
                  overideNoSMS = 0;                                 // clear sms flag
                  _recepient[0]=0; _message[0]=0; _subject[0]=0;    // clear all stored email text
                  emailAttemptCounter = 0;                          // reset attempt counter
              } else {
                // email failed to send
                  // log_system_message("Email send attempt failed, will retry in " + String(EmailAttemptTime) + " seconds");
                  lastEmailAttempt = millis();                      // update time of last attempt
                  emailAttemptCounter ++;                           // increment attempt counter
                  if (emailAttemptCounter >= MaxEmailAttempts) {
                    log_system_message("Error: Max email attempts exceded, email send has failed");
                    emailToSend = 0;                                // clear flag that there is an email waiting to be sent
                    overideNoSMS = 0;                               // clear sms flag
                  }
              }
          }
        }  // if emailToSend
      #endif
      
    // clear flag that esp has recently started (used to disable triggers until all has settled down)
        if (justStarted && millis() > 30000) justStarted = 0;
    

    // Periodically change the LED status to indicate all well plus some admin tasks
        if ((unsigned long)(millis() - LEDtimer) >= ledBlinkRate ) {  
            LEDtimer = millis();                              // reset check timer
            bool allOK = 1;                                   // if all checks leave this as 1 then the all ok LED is flashed
            WIFIcheck();                                      // check wifi connection is ok                              
            if (!wifiok) allOK = 0;                           // if wifi is connected ok
            if (timeStatus() != timeSet) allOK = 0;           // if NTP time is updating ok
            time_t t=now();                                   // read current time to ensure NTP auto refresh keeps triggering (otherwise only triggers when time is required causing a delay in response)
            // blink led
              if (ledBlinkEnabled && allOK) digitalWrite(led, !digitalRead(led));  // invert the LED status if all OK
              else digitalWrite(led, ledOFF);                                      // led off
        }

}    // loop 



// ----------------------------------------------------------------
//       -root web page requested    i.e. http://x.x.x.x/
// ----------------------------------------------------------------

void handleRoot() {  

  WiFiClient client = server.client();             // open link with client
  String tstr;                                     // temp store for building line of html
  webheader(client);                               // html page header 

//  // set document title
//      client.print("\n<script>\n");
//      if (emailWarningMode > 0) client.printf("  document.title = \"%s\";\n", stitle); 
//      else client.printf("  document.title = \"%s\";\n", "Warning!"); 
//      client.print("</script>\n");

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = String(cip[0]) +"." + String(cip[1]) + "." + String(cip[2]) + "." + String(cip[3]);
    clientIP = decodeIP(clientIP);               // check for known IP addresses
    // log_system_message("Root page requested from: " + clientIP);  

       
  // action any button presses or commands 

    // if default settings requested
      if (server.hasArg("defaults")) { 
        emailWarningMode = 0;
        noMovementTriggerTime = 12;
        motionFirstTrigger = 60;
        radarTriggerTriplevel = 10;
        highTempWarningLevel = 30.0;
        lowTempWarningLevel = 10.0;
        storeEEPROM();      // store changed setting in eeprom    `
        log_system_message("default settings set via web page (" + clientIP + ")"); 
      }         

    // if clear log requested
      if (server.hasArg("clearlog")) { 
        for (int i=0; i < LogNumber; i++){
          system_message[i]="";
        }          
        system_message_pointer = 0;
        log_system_message("Log cleared via web page (" + clientIP + ")"); 
      }

    // if change to lowTempWarningLevel requested
      if (server.hasArg("lowtemp")) {
        String Tvalue = server.arg("lowtemp");   // read value
        float Tval = Tvalue.toInt();
        if (Tval < 20.0 && Tval >= 0.0) {
          lowTempWarningLevel = Tval;
          storeEEPROM();      // store changed setting in eeprom    
          log_system_message("lowTempWarningLevel changed to " + String(Tval) + " via web page (" + clientIP + ")"); 
        }
      }

    // if change to highTempWarningLevel requested
      if (server.hasArg("hightemp")) {
        String Tvalue = server.arg("hightemp");   // read value
        float Tval = Tvalue.toInt();
        if (Tval > 15.0 && Tval < 40.0) {
          highTempWarningLevel = Tval;
          storeEEPROM();      // store changed setting in eeprom    
          log_system_message("highTempWarningLevel changed to " + String(Tval) + " via web page (" + clientIP + ")"); 
        }
      }      

    // if change to motionFirstTrigger requested
      if (server.hasArg("first")) {
        String Tvalue = server.arg("first");   // read value
        int Tval = Tvalue.toInt();
        if (Tval > 0 && Tval <= 1440) {
          motionFirstTrigger = Tval;
          storeEEPROM();      // store changed setting in eeprom    
          log_system_message("motionFirstTrigger changed to " + String(Tval) + " via web page (" + clientIP + ")"); 
        }
      }    

    // if change to noMovementTriggerTime requested
      if (server.hasArg("none")) {
        String Tvalue = server.arg("none");   // read value
        int Tval = Tvalue.toInt();
        if (Tval >= 0 && Tval <= 168) {
          noMovementTriggerTime = Tval;
          storeEEPROM();      // store changed setting in eeprom    
          log_system_message("noMovementTriggerTime changed to " + String(Tval) + " via web page (" + clientIP + ")"); 
        }
      }         


    // if change to radarTriggerTriplevel requested
      if (server.hasArg("trigger")) {
        String Tvalue = server.arg("trigger");   // read value
        int Tval = Tvalue.toInt();
        if (Tval > 0 && Tval <= 600) {
          radarTriggerTriplevel = Tval;
          storeEEPROM();      // store changed setting in eeprom    
          log_system_message("radarTriggerTriplevel changed to " + String(Tval) + " via web page (" + clientIP + ")"); 
        }
      }         

    #if ENABLE_EMAIL
      // if email sending enable toggle requested
        if (server.hasArg("email")) {
          emailWarningMode++;
          if (emailWarningMode > 3) emailWarningMode = 0;
          log_system_message("Email sending mode changed to " + String(emailWarningMode) + " (" + clientIP + ")"); 
          storeEEPROM();      // store changed setting in eeprom    
        }
    #endif 
    
    // if showAllRadarTriggers toggle requested
      if (server.hasArg("all")) {
        if (showAllRadarTriggers) {
          log_system_message("showAllRadarTriggers set to off via web page (" + clientIP + ")"); 
          showAllRadarTriggers = 0;
        } else {
          log_system_message("showAllRadarTriggers set to on via web page (" + clientIP + ")"); 
          showAllRadarTriggers = 1;
        }  
        storeEEPROM();      // store changed setting in eeprom    
      }          

    // if test log data requested
      if (server.hasArg("testdata")) {
        log_system_message("Test log data created via web page (" + clientIP + ")"); 
        for (int i=0; i < 24; i++) {
          temperatures[i]=random(13,25);
          movements[i]=random(0,20);
        }
        tempLogTimer = millis();   // reset timer
      }      

    // if clear log data requested
      if (server.hasArg("cleardata")) {
        log_system_message("Test log data cleared via web page (" + clientIP + ")"); 
        for (int i=0; i < 24; i++) {
          temperatures[i] = 0;
          movements[i] = 0;
          logCounter = 0;
        }
        tempLogTimer = millis();   // reset timer
      }          
      
    // if button "reset" button was pressed  
      if (server.hasArg("reset")) {
          log_system_message("Reset button was pressed on web page (" + clientIP + ")"); 
          // reset flags
            smokeAlarmTriggered = 0;
            noMovementWarningTriggered = 0;    
            tempWarningTriggered = 0;
            lastMovementDetection = "n/a";
            #if ENABLE_EMAIL
              emailToSend = 0;
            #endif
            MOTIONdetected = millis();      
            MOTIONdetectedEvent = 0;   // last motion detected (zero = reset)
          // reset max/min values
            lowestTemp = readTemp(0);
            highestTemp = readTemp(0);
            lowestHumidity = readTemp(1);
            highestHumidity = readTemp(1);         
      }


  // build the HTML code 

    client.printf("<FORM action='%s' method='post'>\n", HomeLink);     // used by the buttons (action = the page send it to)
    // client.println("<P>");                                               // start of section

    // insert an iframe containing the changing data (updates every few seconds using javascript)
      client.println("<br><iframe id='dataframe' height=320 width=700 frameborder='0'></iframe>");
      // javascript to refresh data display
        client.println("<script>");
        client.printf("setTimeout(function() {document.getElementById('dataframe').src='/data';}, %s );\n", JavaRefreshTime);
        client.printf("window.setInterval(function() {document.getElementById('dataframe').src='/data';}, %s );\n", datarefresh);
        client.println("</script>"); 
     
    // Temperature
      if (tempSensing) client.println("<br>Temperature warning trigger settings: Low=" + String(lowTempWarningLevel, 0) + "C, High=" + String(highTempWarningLevel, 0) + "C");

    // radar      
      if (radarDetection) {
        client.println("<br>Movement detected when radar is triggered for more than " + String(radarTriggerTriplevel) + " seconds");
        if (emailWarningMode > 0) client.println("<br>Notification sent at first movement detected in over " + String(motionFirstTrigger) + " mins");
        if (noMovementTriggerTime > 0) client.println("<br>Warning sent if no movement detected for " + String(noMovementTriggerTime) + " hours");
      }

    #if ENABLE_EMAIL
      // email / SMS
        client.print("<br>");
        if (emailWarningMode == 0) client.printf("%sE-mail and SMS sending disabled%s", colRed, colEnd);
        else if (emailWarningMode == 1) client.println("E-mail sending enabled");
        else if (emailWarningMode == 2) client.println("E-mail sending enabled, SMS sending only at night");
        else if (emailWarningMode == 3) client.println("E-mail and SMS sending enabled");      
    #endif


    // graphs using Javascript

      client.println("<br><br><small>Previous 24hrs - last updated: " + lastLogUpdateTime + "</small><br>");  
      client.print (R"=====(
        <canvas id="myCanvas" width="500" height="200" style="border:2px solid #c3c3c3;">
            Your browser does not support the canvas element.
        </canvas>
        <script>
            var gitems=24, gheight=100, gwidth=500, i;
            var canvas = document.getElementById("myCanvas");
            var ctx = canvas.getContext("2d");
            ctx.fillStyle = "#000000";
            ctx.font = "12px Arial";
      )=====");
      if (tempSensing) client.println("ctx.fillText(\"Temperatures\", 200, 16);");
      if (radarDetection) client.println("ctx.fillText(\"Movement detections per hour\", 160, 116);");
      client.print("ctx.font = \"7px Arial\";");
  
      // draw temperature log graph
      
        // find highest readings recorded
            float highestTemp = 0;
            int highestMovements = 0;
            for (int i=0; i < 24; i++) {
                if (temperatures[i] > highestTemp && temperatures[i] < 600.00) highestTemp = temperatures[i];
                if (movements[i] > highestMovements) highestMovements = movements[i];
            }
      
        int tcnt = logCounter;
        int gdat;
        for (int i=0; i < 24; i++) {
          tcnt++;
          if (tcnt >= 24) tcnt=0;
            // temperatures
              if (tempSensing) {        // if temp sensor installed
                // temperatures[tcnt]=i;    // for testing 
                gdat = map((float)temperatures[tcnt],(float)0,(float)highestTemp+1.0,(int)0,(int)80);   // scale temperature to graph axis (0-80)
                client.println("ctx.fillStyle = \"#00FF00\";");    // bar colour
                client.println("ctx.fillRect( (" + String(i) + " * (gwidth / gitems)),gheight,20-2,-" + String(gdat) + ");");
                client.println("ctx.fillStyle = \"#000000\";");    // text colour
                client.print("ctx.fillText( \"");
                  client.print(temperatures[tcnt],1);
                  client.println("\", " + String(i) + " * (gwidth / gitems) + 1, gheight - 2);");    
              }
            // movement detections
              if (radarDetection) {     // if radar module installed
                gdat = map(movements[tcnt],0,highestMovements+1,0,80);
                client.println("ctx.fillStyle = \"#00FF00\";");    // bar colour
                client.println("ctx.fillRect( (" + String(i) + " * (gwidth / gitems)),gheight + 100,20-2,-" + String(gdat) + ");");
                client.println("ctx.fillStyle = \"#000000\";");    // text colour
                client.print("ctx.fillText( \"");
                  client.print(movements[tcnt],1);
                  client.println("\", " + String(i) + " * (gwidth / gitems) + 1, gheight + 100 - 2);");   
              }
        }
              
      client.println("</script>"); 

       
    // reset button 
      client.write("<br><input style='height: 30px;' name='reset' value='Reset' type='submit'>\n");

  
    // close page
      //client.write("</P>");                       // end of section    
      client.write("</form>\n");                  // end form section (used by buttons etc.)
      webfooter(client);                          // html page footer
      delay(5);        
      client.stop();

}

  
// ----------------------------------------------------------------
//     -data web page requested     i.e. http://x.x.x.x/data
// ----------------------------------------------------------------
//   This shows information on the root web page which refreshes every few seconds

void handleData(){

  WiFiClient client = server.client();          // open link with client
  String tstr;                                  // temp store for building lines of html;

  // send standard html header
    client.write("HTTP/1.1 200 OK\r\n");
    client.write("Content-Type: text/html\r\n");
    client.write("Connection: close\r\n");
    client.write("\r\n");
    client.write("<!DOCTYPE HTML>\n");

  tstr = currentTime();   
  client.print(tstr);   

  // smoke alarm
    if (smokeDetection) {
      client.print("<br>");
      if (!smokeAlarmTriggered) client.print("No smoke has been detected");
      else client.printf("%sSmoke has been detected%s", colRed, colEnd);
      if (digitalRead(smokeAlarmPin)) client.printf(" - %sSmoke detector is currently triggered%s", colRed, colEnd);
    }
    
  // movement detector
    if (radarDetection) {
      client.print("<br>");
      client.print("Last movement detected: " + lastMovementDetection);
      if (digitalRead(movementSensorPin)) {
        client.printf(" - %sMovement sensor triggered%s", colRed, colEnd);
      }
      if (noMovementWarningTriggered) {
        client.printf("- %sA no recent movement warning triggered%s", colRed, colEnd);
      }
    }


  // Most recent log entry
    client.print("<br>");
    client.println("Log: " + system_message[system_message_pointer]);    

  if (tempSensing) {
    // Temperature
      client.print("<br><br>");
      client.print("Current temperature: " + printVal(readTemp(0), 0));
      if (tempWarningTriggered) client.printf(" - %sA temperature warning triggered%s", colRed, colEnd);
      client.print("<br>");
      client.print("&ensp;Lowest recorded temperature: " + printVal(lowestTemp, 0) + ", Highest: " + printVal(highestTemp, 0));
  
    // Humidity    
    client.print("<br><br>");
      client.print("Current humidity: " + printVal(readTemp(1), 1));
      client.print("<br>");
      client.print("&ensp;Lowest recorded humidity: " + printVal(lowestHumidity, 1) + ", Highest: " + printVal(highestHumidity, 1));
  }
      
  // Misc. status line   (Note: '&ensp;' =  add a space)
    client.print("<br><br>");
    if (OTAEnabled) client.printf("%s(OTA ENABLED)%s&ensp;", colRed, colEnd);  
    #if ENABLE_EMAIL
      if (emailToSend) client.printf("%s(An E-mail is being sent)%s&ensp;", colRed, colEnd);
    #endif
    if (justStarted) client.printf("%s(Recently Started)%s&ensp;", colRed, colEnd);
    if (showAllRadarTriggers && radarDetection) client.print("(Log all radar triggers)&ensp;");
    if (sounderAttached && digitalRead(sounderPin) == HIGH) client.printf("%s(Sounder on)%s&ensp;", colRed, colEnd);
       
  // close html page
    client.println("<br></body></html>");
    delay(10);      
    client.stop();
}


// convert a temperature or humidity float value to a String
String printVal(float fval, bool rtype) {
  String sval = String(fval,1);
  if (rtype == 0) sval += "C";
  else sval += "%";
  if (fval > 600.0) return "Error";
  else return sval;
}


// ----------------------------------------------------------------
//      -ping web page requested     i.e. http://x.x.x.x/ping
// ----------------------------------------------------------------

void handlePing(){

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = String(cip[0]) +"." + String(cip[1]) + "." + String(cip[2]) + "." + String(cip[3]);
    clientIP = decodeIP(clientIP);               // check for known IP addresses
    // log_system_message("Ping page requested from: " + clientIP);  
    
  String message = "OK - Last movement:  " + lastMovementDetection;
  server.send(404, "text/plain", message);   // send reply as plain text
  
}



// ----------------------------------------------------------------
//           -smoke alarm has triggered
// ----------------------------------------------------------------

void smokeAlarm() {
  if (!smokeDetection) return;             // there is no smoke alarm attached
  log_system_message("The smoke alarm has triggered!"); 
  if (smokeAlarmTriggered) return;         // if smoke alarm has already been triggered
  smokeAlarmTriggered = 1;                 // flag that the smoke alarm has been triggered

  #if ENABLE_EMAIL
    // send an email
      _recepient[0]=0; _message[0]=0; _subject[0]=0;                  // clear any existing text
      strcat(_recepient, _emailReceiver);                             // email address to send it to
      strcat(_subject,stitle);
      strcat(_subject,": smoke alarm");
      strcat(_message,"The smoke alarm has triggered");
      emailToSend=1; lastEmailAttempt=0; emailAttemptCounter=0;       // set flags that there is an email ready to be sent
      overideNoSMS = 1;                                               // send sms even if flagged as disabled
  #endif
  
}   // smokeAlarm



// ----------------------------------------------------------------
//               -radar motion detector has triggered
// ----------------------------------------------------------------
// triggered when total time radar was triggered during previous set timeframe exceeds a set level

void radarTriggered(uint16_t totalOnTime) {

  MOTIONdetected = millis();                 // log time of last detection
 
  // if ((unsigned long)(millis() - MOTIONdetectedEvent) < (3 * 1000) ) return;     // too soon since last trigger event

  String dtime = String( (float)(totalOnTime / 1000.0), 1 ) + " seconds";     // time radar was triggered as a string
  log_system_message("MOVEMENT WAS DETECTED! (" + dtime + ")" );  
  lastMovementDetection = currentTime() + "(" + dtime + ")";                  // store last time of detection as a string    
  movementCounts++;                                                           // increment counter for hourly logs         

  #if ENABLE_EMAIL
    // if this is the first detection for several hours send an email (zero = has been reset)
      if ( (MOTIONdetectedEvent == 0) || ((unsigned long)(millis() - MOTIONdetectedEvent) > (1000 * 60 * motionFirstTrigger)) ) {
        _recepient[0]=0; _message[0]=0; _subject[0]=0;                  // clear any existing text
        strcat(_recepient, _emailReceiver);                             // email address to send it to
        strcat(_subject,stitle);
        strcat(_subject,": Movement detected");
        strcat(_message,"First movement detected (in a long time)");
        emailToSend=1; lastEmailAttempt=0; emailAttemptCounter=0;       // set flags that there is an email ready to be sent
        overideNoSMS=0;
      }
  #endif
  
  noMovementWarningTriggered = 0;                                     // clear no recent movement warning flag if it is set
  MOTIONdetectedEvent = millis();                                     // log time of last detection 
    
}   // movementDetected


// ----------------------------------------------------------------
//           -no movement detected recently
// ----------------------------------------------------------------
// triggered if no movement has been detected for certain number of hours (noMovementTriggerTime)

void noMovementDetected() {

  if (noMovementTriggerTime == 0) return;                             // check if feature is enabled

  if (noMovementWarningTriggered) return;                             // check if warning has already been triggered
  
  log_system_message("No recent movement detected"); 
  noMovementWarningTriggered = 1;                                     // flag warning has triggered

  #if ENABLE_EMAIL
    // send an email
      _recepient[0]=0; _message[0]=0; _subject[0]=0;                    // clear any existing text
      strcat(_recepient, _emailReceiver);                               // email address to send it to
      strcat(_subject,stitle);
      strcat(_subject,": No recent movement");
      strcat(_message,"No recent movement has been detected");
      emailToSend=1; lastEmailAttempt=0; emailAttemptCounter=0;       // set flags that there is an email ready to be sent  
  #endif
  
}



// ----------------------------------------------------------------
//           -Check temperature and humidity are ok
// ----------------------------------------------------------------

void checkTemp() {

  if (tempWarningTriggered) return;       // temp warning has already triggered
  if (!tempSensing) return;               // no temp sensor installed

  float current = readTemp(0);             // read current temperature
  if (current == 666.0) return;            // error reading temperature

  if (current < lowTempWarningLevel || current > highTempWarningLevel) {
    // make sure it is not an error
      delay(200);
      current = readTemp(0); 
      if (current < lowTempWarningLevel || current > highTempWarningLevel) {
        // trigger temperature warning
          tempWarningTriggered = 1;           // flag that a warning has been triggered
          log_system_message("Temperature warning has triggered, current temp. = " + String(current) + "C"); 

          #if ENABLE_EMAIL
            // send an email
              _recepient[0]=0; _message[0]=0; _subject[0]=0;                  // clear any existing text
              strcat(_recepient, _emailReceiver);                               // email address to send it to
              strcat(_subject,stitle); 
              strcat(_subject,": Temperature warning");
              strcat(_message,"A temperature outside of normal paramerer warning has triggered");
              emailToSend=1; lastEmailAttempt=0; emailAttemptCounter=0;       // set flags that there is an email ready to be sent   
          #endif  
      }
  }
  
}   // checkTemp



// ----------------------------------------------------------------
//           -Read temperature sensor
// ----------------------------------------------------------------
// returns current temperature or humidity (666=error)
// 0=read temp, 1=read humidity

float readTemp(bool which) {

  if (!tempSensing) return 0;               // no sensor installed

  if (which == 0) {
  
    // get temperature
      //delay(dht.getMinimumSamplingPeriod());
      float t = dht.getTemperature();
      if (isnan(t)) {
        delay(300);
        t = dht.getTemperature();
      }
      if (isnan(t)) {
        log_system_message("Error reading temperature");
        t=666.00;   // error
      }
      else {
        if (t > highestTemp) highestTemp = t;
        if (t < lowestTemp) lowestTemp =t;
      }
      return t;
      
  } else {

    // Get humidity 
      //delay(dht.getMinimumSamplingPeriod());
      float h = dht.getHumidity();
      if (isnan(h)) {
        delay(300);
        h = dht.getHumidity();
      }
      if (isnan(h)) {
        log_system_message("Error reading humidity");
        h=666.00;   // error
      } else {
        if (h > highestHumidity) highestHumidity = h;
        if (h < lowestHumidity) lowestHumidity =h;
      }
      return h;
  }
  
}    // readTemp



// ----------------------------------------------------------------
//                 -store variables in to eeprom
// ----------------------------------------------------------------
// Note: if over 32 bytes increase value in SETUP section

bool storeEEPROM() {
  
  EEPROM.put(0, emailWarningMode);                  // 1 byte
  EEPROM.put(2, noMovementTriggerTime);             // 2 bytes
  EEPROM.put(4, motionFirstTrigger);                // 2 bytes
  EEPROM.put(8, radarTriggerTriplevel);             // 2 bytes
  EEPROM.put(11, highTempWarningLevel);             // 4 bytes
  EEPROM.put(15, lowTempWarningLevel);              // 4 bytes
  EEPROM.put(21, showAllRadarTriggers);             // 1 bytes  

  // write to eeprom
    bool ok = EEPROM.commit();
    if (!ok) log_system_message("ERROR storing setting in EEProm");
    return ok;
  
}



// ----------------------------------------------------------------
//                 -read variables from eeprom
// ----------------------------------------------------------------

void readEEPROM() {

  EEPROM.get(0, emailWarningMode);                  // 1 byte
  EEPROM.get(2, noMovementTriggerTime);             // 2 bytes
  EEPROM.get(4, motionFirstTrigger);                // 2 bytes
  EEPROM.get(8, radarTriggerTriplevel);             // 2 bytes
  EEPROM.get(11, highTempWarningLevel);             // 4 bytes
  EEPROM.get(15, lowTempWarningLevel);              // 4 bytes  
  EEPROM.get(21, showAllRadarTriggers);             // 1 bytes  

  // check values are reasonable
    if (noMovementTriggerTime < 0 || noMovementTriggerTime > 168) noMovementTriggerTime = 12;
    if (motionFirstTrigger <= 0 || motionFirstTrigger > 1440) motionFirstTrigger = 60;
    if (radarTriggerTriplevel <= 0 || radarTriggerTriplevel > 600) radarTriggerTriplevel = 10;
    if (highTempWarningLevel < 20.0 || highTempWarningLevel > 40.0) highTempWarningLevel = 30.0;
    if (lowTempWarningLevel < 0.0 || lowTempWarningLevel > 20.0) lowTempWarningLevel = 10;
    if (emailWarningMode < 0 || emailWarningMode > 3) emailWarningMode = 0;
}



// ----------------------------------------------------------------
//           -testing page     i.e. http://x.x.x.x/test
// ----------------------------------------------------------------

void handleTest(){

  WiFiClient client = server.client();          // open link with client

  // log page request including clients IP address
    IPAddress cip = client.remoteIP();
    String clientIP = String(cip[0]) +"." + String(cip[1]) + "." + String(cip[2]) + "." + String(cip[3]);
    clientIP = decodeIP(clientIP);               // check for known IP addresses
    log_system_message("Test page requested from: " + clientIP);  
  
  webheader(client);                 // add the standard html header
  client.println("<br>TEST PAGE<br><br>");


  // ---------------------------- test section here ------------------------------



    #if ENABLE_EMAIL
      client.println("<br>Sending an email<br>");
      _recepient[0]=0; _message[0]=0; _subject[0]=0;                  // clear any existing text
      strcat(_recepient, _emailReceiver);                             // email address to send it to
      strcat(_subject,stitle);
      strcat(_subject,": testing");
      strcat(_message,"test message");
      emailToSend=1; lastEmailAttempt=0; emailAttemptCounter=0;       // set flags that there is an email ready to be sent
      overideNoSMS = 0;                                               // send sms even if flagged as disabled
    #endif


  
  // -----------------------------------------------------------------------------


  // end html page
    webfooter(client);            // add the standard web page footer
    delay(5);
    client.stop();
}


// --------------------------- E N D -----------------------------

<h1>House Monitor for use with the Arduino IDE</h1>
<p align="center"><img src="/images/esp32cam.jpeg" width="80%"/></p>

<table><tr>
  <td><img src="/screengrab.png" /></td>
</tr></table>   

This is a sketch I created to keep an eye on a volnerable family member who lives alone.  
It has a temperature/humidity sensor, radar motion sensor and smoke alarm built in and monitors these and sends me an email or text message if anything out of the ordinary occurs.
e.g. if no one has been detected moving around for several hours or if the temperature goes above or below set values etc.
I am posting it here in case it is of any interest/help to anyone else.

Note: to access the web page use address:    http://x.x.x.x:6969/pine 

It uses an esp8266 development board, DHT22 temperature/humidity sensor, RCWL-0516 Radar movement sensor and a standard smoke detector.
The smoke detector I have used as a "Lifesaver Micro model i9040EV" which I found will work on a 5volt supply and then there is a test point on the circuit board which provides 5v when triggered which can go directly to a gpio pin on the esp8266.  I then cut the wire to the sounder as I want it to be silent and also it will beep periodically to tell you the battery voltage is low.
Note: Of course modifying a smoke alarm like this should not be used as a primary smoke alarm.


You can change settings via the below browser links
  Enable/disable emails toggle with        http://x.x.x.x:6969/pine?email
  Enable/disable sms toggle with           http://x.x.x.x:6969/pine?sms
  Change 'first movement in' setting with  http://x.x.x.x:6969/pine?first=x
  Change 'no movement in' setting with     http://x.x.x.x:6969/pine?none=x
  Change movement detection timeframewith  http://x.x.x.x:6969/pine?timeframe=x
  Change radar trigger level               http://x.x.x.x:6969/pine?trigger=x
  Create test graph data with              http://x.x.x.x:6969/pine?testdata
  Clear graph data with                    http://x.x.x.x:6969/pine?cleardata
  Clear logs                               http://x.x.x.x:6969/pine?clearlog
  Change high temp warning level           http://x.x.x.x:6969/pine?hightemp
  Change low temp warning level            http://x.x.x.x:6969/pine?lowtemp
  Load default settings                    http://x.x.x.x:6969/pine?defaults


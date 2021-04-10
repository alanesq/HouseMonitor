<h1>House Monitor for use with the Arduino IDE</h1>
<p align="center"><img src="/images/esp32cam.jpeg" width="80%"/></p>

<table><tr>
  <td><img src="/screengrab.png"></td>
</tr></table>   

This is a sketch I created to keep an eye on a vulnerable family member who lives alone.  
It has a temperature/humidity sensor, radar motion sensor and smoke alarm built in.  It monitors these and sends me an email or text message if anything out of the ordinary occurs.
<br>e.g. if no one has been detected moving around for several hours or if the temperature goes above or below set values etc.
<br>I am posting it here in case it is of any interest/help to anyone else.

It uses an esp8266 development board, DHT22 temperature/humidity sensor, RCWL-0516 Radar movement sensor and a standard smoke detector.
The smoke detector I have used as a "Lifesaver Micro model i9040EV" which I found will work on a 5volt supply and then there is a test point on the circuit board which provides 5v when triggered which can go directly to a gpio pin on the esp8266.  I then cut the wire to the sounder as I want it to be silent and also it will beep periodically to tell you the battery voltage is low.
<br>Note: Of course modifying a smoke alarm like this should not be used as a primary smoke alarm.

You will need to enter your email settings in to email.h (if using gmail you need to enable pop access and reduce security level)
<br>It uses WifiManager for wifi credentials so you will need to connect to it via its own ssid first to enter your wifi password


<br>You can change settings via the below browser links
<br>  Enable/disable emails toggle with        http://x.x.x.x/?email
<br>  Enable/disable sms toggle with           http://x.x.x.x/?sms
<br>  Change 'first movement in' setting with  http://x.x.x.x/?first=x
<br>  Change 'no movement in' setting with     http://x.x.x.x/?none=x
<br>  Change movement detection timeframewith  http://x.x.x.x/?timeframe=x
<br>  Change radar trigger level               http://x.x.x.x/?trigger=x
<br>  Create test graph data with              http://x.x.x.x/?testdata
<br>  Clear graph data with                    http://x.x.x.x/?cleardata
<br>  Clear logs                               http://x.x.x.x/?clearlog
<br>  Change high temp warning level           http://x.x.x.x/?hightemp
<br>  Change low temp warning level            http://x.x.x.x/?lowtemp
<br>  Load default settings                    http://x.x.x.x/?defaults


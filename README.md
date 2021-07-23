<h1>House Monitor for use with the Arduino IDE</h1>

<table><tr>
  <td><img src="/screengrab.png"></td>
  <td><img src="/screengrab-log.png"></td>
</tr></table>   

This is a sketch I created to keep an eye on a vulnerable family member who lives alone although it is also useful just for keeping an eye on an empty property / outbuilding etc..  
It can monitor temperature/humidity, have a built in smoke detector and uses a cheap radar motion sensor to detect if someone is in the area.  It monitors these and sends me an email or text message if anything out of the ordinary occurs.
You can just use the radar motion detection in which case you just need an esp8266 and radar module costing around £4 total and wiring it could not be easier (just requiring power to each board and a single wire between them).  And of course you can modify the code to your own personal requirements.
<br>I am posting it here in case it is of any interest/help to anyone else.

It is for an esp8266 although it should be able to be converted to use esp32 without too much difficulty, main thing would be gpio pins used.
It can use a DHT22 temperature/humidity sensor, RCWL-0516 Radar movement sensor and a standard smoke detector.
The smoke detector I have used as a "Lifesaver Micro model i9040EV" which I found will work on a 5volt supply and then there is a test point on the circuit board which provides 5v when triggered which can go directly to a gpio pin on the esp8266.  I then cut the wire to the sounder as I want it to be silent and also it will beep periodically to tell you the battery voltage is low.
<br>Note: Of course modifying a smoke alarm like this means it should not be used as a primary smoke alarm.

You will need to enter your email settings in to email.h (if using gmail you need to enable pop access and reduce security level)
<br>It uses WifiManager for wifi credentials so you will need to connect to it via its own ssid first to enter your wifi password
<br>I have found gmails security to mean it proves unreliable and find gmx.com to be more relaibe / easy to set up (you just need to enable POP access).


<br>You can change settings via the below browser links
<br>  Enable/disable emails toggle with        http://x.x.x.x/?email
<br>  Change 'first movement in' setting with  http://x.x.x.x/?first=x
<br>  Change 'no movement in' setting with     http://x.x.x.x/?none=x
<br>  Change radar trigger level               http://x.x.x.x/?trigger=x
<br>  Create test graph data with              http://x.x.x.x/?testdata
<br>  Clear graph data with                    http://x.x.x.x/?cleardata
<br>  Clear logs                               http://x.x.x.x/?clearlog
<br>  Change high temp warning level           http://x.x.x.x/?hightemp
<br>  Change low temp warning level            http://x.x.x.x/?lowtemp
<br>  Load default settings                    http://x.x.x.x/?defaults

<br><br>
Notes:
If you do not have wifi at the location this device will be at I have found that a good solution (in the U.K.) is to use a cheap (from eBay) Huawei E5332 Mobile Wi-Fi with a Three pay as you go sim, buying monthly data packages.
I then use noip.com to give a virtual fixed ip address
Note: Most other networks do not assign an unique IP address to the device so it is impossible to connect to it from the internet.

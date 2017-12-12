# Let the ESP8266 switch your FRITZ!DECT200 socket
This is my first Arduino sketch and is based on an [article](http://www.open4me.de/index.php/2016/10/fritzbox-esp8266-laesst-telefon-klingeln/) by [Sven Bursch-Osewold](https://github.com/littleyoda).

## Hardware
I'm using this sketch on a [D1 mini](https://www.amazon.de/gp/product/B06XPCR921/) clone in conjunction with a FRITZ!Box 7490 running Fritz!OS 06.92 and a FRITZ!DECT200 actor version 03.87.


## Configuration
For using this sketch you just have to enter your Wi-Fi details, a FRITZ!Box user that is allowed to change smart home settings (I recommend to setup one for this use, just in case disaster strikes), and the AIN of the FRITZ!DECT200 socket you want to switch. The latter one can be found in the smart home section of the FRITZ!Box.

Optionally you can also disable the use of DHCP and configure the network manually. I did this to deny internet access to the ESP8266 in my Fritz!Box and assign an IP I want, but for just denying internet access to the device you can also let the FRITZ!Box alway asign the same IP.



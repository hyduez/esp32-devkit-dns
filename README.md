Simple DNS hosted on an ESP32 DevKit v1 (no PSRAM). Add your URLs on blockedDomains[] and that's all.

```shell
$ arduino-cli compile --fqbn esp32:esp32:esp32 .
$ arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```

To monitorize it you can use `picocom /dev/ttyUSB0 -b 115200 --imap lfcrlt`, `minicom -D /dev/ttyUSB0 -b 115200` or view live logs on the web server.

How to use: Once you have logged in to your own SSID (*ProtectMe-dns* by default) and entered your credentials (protectme), go to 198.168.4.1, select your network, and log in. ESP32 will now on the network you selected, it will display via Serial which is its IP. Usually it's 192.168.1.28. Config panel will be avaliable at http://192.168.1.28/config.

Remember:
- 198.168.4.1: for the first time or something bad happened to the network connection
- 198.168.1.28: inside of your network, put it on `resolv.conf` on linux or go to your settings -> wifi's advanced configuration app.

<img width="1921" height="1044" alt="image" src="https://github.com/user-attachments/assets/eae37629-47a1-44af-b0eb-862f40ac3272" />

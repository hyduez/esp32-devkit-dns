Simple DNS hosted on an ESP32 DevKit v1 (no PSRAM). Add your URLs on blockedDomains[] and that's all. Enable DOH in the settings panel; it affects performance slightly. Hide your traffic from your ISP over HTTPS Encrypted Resolution.

```shell
$ arduino-cli compile --fqbn esp32:esp32:esp32 .
$ arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 .
```

To monitorize it you can use `picocom /dev/ttyUSB0 -b 115200 --imap lfcrlt`, `minicom -D /dev/ttyUSB0 -b 115200` or view live logs on the web server (credentials: root, protectme).

How to use: Once you have logged in to your own SSID (*ProtectMe-net* by default) and entered your credentials (protectme), go to 198.168.4.1, select your network, and log in. ESP32 will now on the network you selected, it will display via Serial which is its IP. Usually it's 192.168.1.28. Config panel will be avaliable at http://192.168.1.28/config.

Remember:
- 198.168.4.1: inside of ESP32 network, for the first time or something bad happened to the network connection
- 198.168.1.28: inside of your network, put it on `resolv.conf` on linux or go to your settings -> wifi's advanced configuration and DNS Resolution.

Default Credentials:
- SSID: ProtectMe-net
- Password: protectme
- User (for Config Panel): root

<details>
  <summary>ESP32 DevKit DNS Showcase</summary>
  <img width="1921" height="1044" alt="image" src="https://github.com/user-attachments/assets/35cd6eae-7be8-496c-b547-af8104d5f409" />
</details>
<details>
  <summary>DOH Test (DNS Leaks)</summary>
  <img width="1011" height="682" alt="image" src="https://github.com/user-attachments/assets/438875c6-b6ac-4e9f-bc4e-4d1d6a71172e" />
</details>

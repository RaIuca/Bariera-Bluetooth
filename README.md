# Bariera-Bluetooth

```diff
Descriere :
```

Bariera bluetooth cu sistem de pontaj.

Suporta pana la 25600 de useri.

Afiseaza pontajul in format [An][Luna][Zi][Ora][Secunda]

```diff
Componente :
```

  -[Arduino pro mini 5V / 16MHz](https://www.optimusdigital.ro/en/compatible-with-arduino-pro-mini/72-development-board-compatible-with-arduino-pro-mini-atmega328p.html?search_query=arduino+pro+mini&results=70)
  
  -[EEPROM serial AT24C256](https://www.optimusdigital.ro/en/memories/632-modul-eeprom-at24c256.html)
  
  -[Servomotor SG](https://www.optimusdigital.ro/en/servomotors/26-sg90-micro-servo-motor.html?search_query=sg90&results=4)
  
  -[Modul 4.0 Bluetooth](https://www.optimusdigital.ro/en/wireless-bluetooth/862-modul-bluetooth-40-cu-adaptor-compatibil-33v-si-5v.html?search_query=modul+bluetooth+&results=51)
  
  -[RTC PCF8523](https://www.optimusdigital.ro/en/others/3329-pcf8523-adafruit-rtc-module.html?search_query=RTC&results=43)
  
  -[Baterie LiPo](https://www.optimusdigital.ro/en/7-v-batteries/1173-lipo-zippy-37v-138-mah-20c-battery.html?search_query=lipo+3.7V&results=33)
  
  -[Incarcator TP4056](https://www.optimusdigital.ro/ro/electronica-de-putere-incarcatoare/80-incarcator-de-baterii-tp4056-1a.html)


```diff
Circuit :

```
![alt text](https://i.ibb.co/VY6X1HD/Circuit-incarcator.png)

```diff
Compilare si incarcare :
```

git clone https://github.com/RaIuca/Bariera-Bluetooth

cd Bariera-Bluetooth

make qflash

```diff
Conexiune programator
```
![alt text](https://gksteel.ru/wp-content/uploads/2018/04/bxtransc.png)

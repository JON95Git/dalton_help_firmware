# Dalton Help - Firmware
Firmware do projeto Danton Help

## Descrição

O projeto do firmware baseia-se no uso do microcontrolador ESP32, da fabricante Espressif, para leitura do sensor RGB APDS-9960 via protocolo I2C, posterior conversão dos dados para o padrão HSV e escrita da cor identificada no display LCD 16x2 via PCF8574A.

O código foi escrito em linguagem C, na IDE Eclipse C/C++ (CDT).
Foi utilizado o pack de firmware padrão do fabricante, o ESP_IDF (https://github.com/espressif/esp-idf) no ambiente Linux (Ubuntu 16.04 64 bits).

## Build

Para se realizar o processo de build do firmware, é necessário seguir o tutorial escrito pelo próprio fabricante, disponível em: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html



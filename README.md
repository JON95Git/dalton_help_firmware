# Dalton Help - Firmware
Firmware do projeto Danton Help

## Descrição

O projeto do firmware foi desenvolvido para o microcontrolador ESP32 WROOM32, da fabricante Espressif. Basicamente é realizada a leitura do sensor RGB APDS-9960 via protocolo I2C, posterior conversão dos dados para o padrão HSV, escrita da cor identificada no display LCD 16x2 via PCF8574A e é realizada conexão com um host via Wi-fi.

O código foi escrito em linguagem C, na IDE Eclipse C/C++ (CDT). Cada tarefa no firmware está segmentada em uma "``task``" (tarefa) do  FreeRTOS. 
Foi utilizado o pack de firmware padrão do fabricante, o ESP_IDF (https://github.com/espressif/esp-idf) no ambiente Linux (Ubuntu 16.04 64 bits).

## Build

Para se realizar o processo de build do firmware, é necessário seguir o tutorial escrito pelo próprio fabricante, disponível em: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html



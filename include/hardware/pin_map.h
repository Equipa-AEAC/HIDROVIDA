#ifndef HARDWARE_PIN_MAP_H
#define HARDWARE_PIN_MAP_H

#include <Arduino.h>

// ============================================================================
// Mapa unico de pinos do projeto
// ============================================================================

namespace HardwarePinos {

// Sensores analogicos e digitais do sistema hidroponico.
constexpr uint8_t PINO_DS18B20_DADOS = 3;
constexpr uint8_t PINO_PH_ANALOGICO = 4;
constexpr uint8_t PINO_TDS_ANALOGICO = 5;
constexpr uint8_t PINO_TURBIDEZ_ANALOGICO = 6;
constexpr uint8_t PINO_REPOSICAO_SENSOR_LIMITE = 7;

// Barramento I2C principal: LCD 20x4, RTC DS3231 e sensor do tanque 1.
constexpr uint8_t PINO_TANQUE1_SDA = 8;
constexpr uint8_t PINO_TANQUE1_SCL = 9;
constexpr uint8_t PinoI2cSda = PINO_TANQUE1_SDA;
constexpr uint8_t PinoI2cScl = PINO_TANQUE1_SCL;

// Bombas doseadoras.
constexpr uint8_t PINO_BOMBA_MICRONUTRIENTES = 10;
constexpr uint8_t PINO_BOMBA_NITRATO_CALCIO = 11;
constexpr uint8_t PINO_BOMBA_NITRATO_POTASSIO = 12;
constexpr uint8_t PINO_BOMBA_SULFATO_MAGNESIO = 13;

// Segundo barramento I2C para o tanque 2.
constexpr uint8_t PINO_TANQUE2_SDA = 15;
constexpr uint8_t PINO_TANQUE2_SCL = 16;

// Relays de atuacao.
constexpr uint8_t PINO_RELE_BOMBA_CIRCULACAO = 17;
constexpr uint8_t PINO_RELE_SISTEMA_LUZ = 18;
constexpr uint8_t PinoReleBombaCirculacao = PINO_RELE_BOMBA_CIRCULACAO;
constexpr uint8_t PinoReleSistemaLuz = PINO_RELE_SISTEMA_LUZ;

// Sensores auxiliares.
constexpr uint8_t PINO_DHT22_DADOS = 1;
constexpr uint8_t PINO_CO2_ANALOGICO = 2;

// Botoes do painel local.
// Estes pinos foram deslocados para evitar conflito com os canais analogicos
// do sistema hidroponico principal (pH, TDS e turbidez).
constexpr uint8_t PinoBotaoEsquerda = 35;
constexpr uint8_t PinoBotaoDireita = 36;
constexpr uint8_t PinoBotaoAceitar = 37;

// Saidas digitais de atuacao.
#ifdef LED_BUILTIN
constexpr uint8_t PinoLedEmbutido = LED_BUILTIN;
#else
constexpr uint8_t PinoLedEmbutido = 2;
#endif

} // namespace HardwarePinos

namespace HardwareNiveis {

// Rele tipico de 5V com acionamento em nivel baixo.
constexpr uint8_t NivelReleAtivo = LOW;
constexpr uint8_t NivelReleInativo = HIGH;

// LED de diagnostico em nivel alto.
constexpr uint8_t NivelLedLigado = HIGH;
constexpr uint8_t NivelLedDesligado = LOW;

} // namespace HardwareNiveis

#endif // HARDWARE_PIN_MAP_H

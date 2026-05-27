#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_PH.h>
#ifdef ReceivedBufferLength
#undef ReceivedBufferLength
#endif
#include <DHT.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "config.h"
#include "garden.h"
#include "hardware/pin_map.h"
#include "hydro_control.h"
#include "hydro_lcd_data.h"
#include "services/blynk_service.h"

namespace {

using HardwarePinos::PINO_BOMBA_MICRONUTRIENTES;
using HardwarePinos::PINO_BOMBA_NITRATO_CALCIO;
using HardwarePinos::PINO_BOMBA_NITRATO_POTASSIO;
using HardwarePinos::PINO_BOMBA_SULFATO_MAGNESIO;
using HardwarePinos::PINO_DHT22_DADOS;
using HardwarePinos::PINO_DS18B20_DADOS;
using HardwarePinos::PINO_PH_ANALOGICO;
using HardwarePinos::PINO_RELE_BOMBA_CIRCULACAO;
using HardwarePinos::PINO_RELE_SISTEMA_LUZ;
using HardwarePinos::PINO_REPOSICAO_SENSOR_LIMITE;
using HardwarePinos::PINO_TANQUE1_SCL;
using HardwarePinos::PINO_TANQUE1_SDA;
using HardwarePinos::PINO_TANQUE2_SCL;
using HardwarePinos::PINO_TANQUE2_SDA;
using HardwarePinos::PINO_TDS_ANALOGICO;
using HardwarePinos::PINO_TURBIDEZ_ANALOGICO;

constexpr uint8_t DHT22_TIPO = DHT22;

constexpr uint32_t SISTEMA_INTERVALO_LEITURA_MS = 1000;
constexpr uint32_t DS18B20_TEMPO_CONVERSAO_MS = 750;
constexpr uint32_t DHT22_INTERVALO_LEITURA_MS = 2500;

constexpr uint8_t ADC_NUMERO_AMOSTRAS_GERAL = 12;
constexpr uint8_t ADC_NUMERO_AMOSTRAS_TDS = 25;
constexpr float ADC_CONTAGEM_MAXIMA = 4095.0f;
constexpr float ADC_REFERENCIA_MV = 3300.0f;
constexpr float TEMPERATURA_COMPENSACAO_PADRAO_C = 25.0f;

constexpr float TDS_TENSAO_MAXIMA_VALIDA_MV = 2300.0f;
constexpr float TDS_TENSAO_MINIMA_VALIDA_MV = 20.0f;
constexpr float TDS_PPM_MAXIMO_VALIDO = 2200.0f;
constexpr float TDS_VARIACAO_MAXIMA_POR_CICLO = 350.0f;
constexpr float TDS_FATOR_SUAVIZACAO = 0.30f;

constexpr float TURBIDEZ_TENSAO_LIMPA_MV = 2500.0f;
constexpr float TURBIDEZ_TENSAO_SUJA_MV = 1500.0f;

constexpr float TANQUE_1_ALTURA_CM = 10.0f;
constexpr float TANQUE_2_ALTURA_CM = 18.0f;
constexpr float GROVE_NIVEL_FAIXA_SENSOR_CM = 10.0f;

constexpr uint8_t GROVE_NIVEL_ENDERECO_BAIXO_PRIMARIO = 0x77;
constexpr uint8_t GROVE_NIVEL_ENDERECO_ALTO_PRIMARIO = 0x78;
constexpr uint8_t GROVE_NIVEL_ENDERECO_BAIXO_ALTERNATIVO = 0x3B;
constexpr uint8_t GROVE_NIVEL_ENDERECO_ALTO_ALTERNATIVO = 0x3C;

constexpr bool REPOSICAO_SENSOR_ATIVO_EM_BAIXO = true;

constexpr uint8_t NIVEL_BOMBA_NUTRIENTE_ATIVO = HIGH;
constexpr uint8_t NIVEL_BOMBA_NUTRIENTE_INATIVO = LOW;

struct SaidaDigital {
  uint8_t saidaPino;
  const char* saidaNome;
  uint8_t saidaNivelAtivo;
  uint8_t saidaNivelInativo;
  bool saidaLigada;
};

struct RelogioSoftware {
  uint8_t relogioHora;
  uint8_t relogioMinuto;
  uint8_t relogioSegundo;
  uint32_t relogioUltimoTickMs;
};

enum IndiceBombaNutriente : uint8_t {
  BombaMicro = 0,
  BombaCalcio = 1,
  BombaPotassio = 2,
  BombaMagnesio = 3,
  BombaNutrienteTotal = 4,
};

constexpr uint16_t AUTODOSE_TRIGGER_PPM = 350;
constexpr uint16_t AUTODOSE_TARGET_PPM = 425;
constexpr uint16_t AUTODOSE_LIMITE_ALTO_PPM = 550;
constexpr uint32_t AUTODOSE_MISTURA_MS = 90000;
constexpr uint32_t AUTODOSE_BLOQUEIO_MS = 300000;
constexpr uint8_t AUTODOSE_MAX_CICLOS = 3;
constexpr uint32_t AUTODOSE_PULSO_BOMBAS_MS[BombaNutrienteTotal] = {1200, 1500, 1200, 1000};

enum class EstadoAutoDose : uint8_t {
  Desligado = 0,
  Pronto,
  Doseando,
  Mistura,
  TdsInvalido,
  Manual,
  Bloqueado,
  TdsAlto
};

TwoWire barramentoI2cTanque2 = TwoWire(1);
OneWire barramentoOneWireDs18b20(PINO_DS18B20_DADOS);
DallasTemperature sensorDs18b20(&barramentoOneWireDs18b20);
DFRobot_ESP_PH sensorPh;
DHT sensorDht22(PINO_DHT22_DADOS, DHT22_TIPO);

SaidaDigital saidasBombasNutrientes[BombaNutrienteTotal] = {
    {PINO_BOMBA_MICRONUTRIENTES, "Micronutrientes", NIVEL_BOMBA_NUTRIENTE_ATIVO, NIVEL_BOMBA_NUTRIENTE_INATIVO, false},
    {PINO_BOMBA_NITRATO_CALCIO, "Nitrato calcio", NIVEL_BOMBA_NUTRIENTE_ATIVO, NIVEL_BOMBA_NUTRIENTE_INATIVO, false},
    {PINO_BOMBA_NITRATO_POTASSIO, "Nitrato potassio", NIVEL_BOMBA_NUTRIENTE_ATIVO, NIVEL_BOMBA_NUTRIENTE_INATIVO, false},
    {PINO_BOMBA_SULFATO_MAGNESIO, "Sulfato magnesio", NIVEL_BOMBA_NUTRIENTE_ATIVO, NIVEL_BOMBA_NUTRIENTE_INATIVO, false},
};

SaidaDigital saidaReleCirculacao = {
    PINO_RELE_BOMBA_CIRCULACAO,
    "Circulacao",
    HardwareNiveis::NivelReleAtivo,
    HardwareNiveis::NivelReleInativo,
    false};

SaidaDigital saidaReleLuz = {
    PINO_RELE_SISTEMA_LUZ,
    "Luz",
    HardwareNiveis::NivelReleAtivo,
    HardwareNiveis::NivelReleInativo,
    false};

HydroHorarioSimples horarioSistemaLuz = {false, 8, 0, 20, 0};
HydroHorarioSimples horarioSistemaCirculacao = {false, 9, 0, 9, 15};
RelogioSoftware relogioSistema = {12, 0, 0, 0};

float leituraDs18b20TemperaturaAguaC = NAN;
float leituraPhValor = NAN;
float leituraPhTensaoMv = NAN;

float leituraTdsValorPpm = NAN;
float leituraTdsTensaoMv = NAN;
bool leituraTdsValida = false;
bool leituraTdsTemHistorico = false;
float leituraTdsUltimoValorPpm = NAN;
char leituraTdsEstado[16] = "INIT";

float leituraTurbidezTensaoMv = NAN;
float leituraDht22HumidadePercent = NAN;
float leituraDht22TemperaturaAmbienteC = NAN;

float leituraTanque1NivelCm = NAN;
float leituraTanque2NivelCm = NAN;
float leituraTanque1Percent = NAN;
float leituraTanque2Percent = NAN;
bool leituraReposicaoOk = false;
bool sistemaAtivo = true;

uint32_t sistemaUltimoCicloSensoresMs = 0;
uint32_t ds18b20UltimoPedidoMs = 0;
uint32_t dht22UltimaLeituraMs = 0;
bool ds18b20ConversaoPendente = false;

bool autodoseAtivo = false;
EstadoAutoDose autodoseEstado = EstadoAutoDose::Desligado;
uint8_t autodoseIndiceBombaAtual = 0;
uint8_t autodoseCiclosExecutados = 0;
uint32_t autodoseMomentoEstadoMs = 0;
uint32_t autodoseBloqueadoAteMs = 0;
uint32_t autodoseBombaDesligarMs[BombaNutrienteTotal] = {0};
bool autodoseBombaTemporizada[BombaNutrienteTotal] = {false};

void aplicarEstadoSaida(SaidaDigital& saida, bool ligar);
void definirEstadoBombaNutrienteInterno(uint8_t indiceBomba, bool ligada, bool temporizada);

void inicializarRelogioSistema(void) {
  const char horaCompilacao[] = __TIME__;
  relogioSistema.relogioHora = static_cast<uint8_t>(((horaCompilacao[0] - '0') * 10) + (horaCompilacao[1] - '0'));
  relogioSistema.relogioMinuto = static_cast<uint8_t>(((horaCompilacao[3] - '0') * 10) + (horaCompilacao[4] - '0'));
  relogioSistema.relogioSegundo = static_cast<uint8_t>(((horaCompilacao[6] - '0') * 10) + (horaCompilacao[7] - '0'));
  relogioSistema.relogioUltimoTickMs = millis();
}

void normalizarHorario(HydroHorarioSimples& horario) {
  horario.horarioHoraInicio %= 24;
  horario.horarioMinutoInicio %= 60;
  horario.horarioHoraFim %= 24;
  horario.horarioMinutoFim %= 60;
}

void atualizarRelogioSistema(void) {
  const uint32_t momentoAtualMs = millis();
  while (momentoAtualMs - relogioSistema.relogioUltimoTickMs >= 1000u) {
    relogioSistema.relogioUltimoTickMs += 1000u;
    relogioSistema.relogioSegundo++;

    if (relogioSistema.relogioSegundo >= 60) {
      relogioSistema.relogioSegundo = 0;
      relogioSistema.relogioMinuto++;
    }

    if (relogioSistema.relogioMinuto >= 60) {
      relogioSistema.relogioMinuto = 0;
      relogioSistema.relogioHora++;
    }

    if (relogioSistema.relogioHora >= 24) {
      relogioSistema.relogioHora = 0;
    }
  }
}

uint16_t converterHoraMinutoParaTotalMinutos(uint8_t hora, uint8_t minuto) {
  return static_cast<uint16_t>(hora % 24) * 60u + static_cast<uint16_t>(minuto % 60);
}

bool horarioIncluiMomento(const HydroHorarioSimples& horario, uint8_t horaAtual, uint8_t minutoAtual) {
  if (!horario.horarioAtivo) {
    return false;
  }

  const uint16_t minutoInicio = converterHoraMinutoParaTotalMinutos(horario.horarioHoraInicio, horario.horarioMinutoInicio);
  const uint16_t minutoFim = converterHoraMinutoParaTotalMinutos(horario.horarioHoraFim, horario.horarioMinutoFim);
  const uint16_t minutoAtualTotal = converterHoraMinutoParaTotalMinutos(horaAtual, minutoAtual);

  if (minutoInicio == minutoFim) {
    return true;
  }

  if (minutoInicio < minutoFim) {
    return minutoAtualTotal >= minutoInicio && minutoAtualTotal < minutoFim;
  }

  return minutoAtualTotal >= minutoInicio || minutoAtualTotal < minutoFim;
}

void aplicarHorarioRele(SaidaDigital& saidaRele, const HydroHorarioSimples& horario) {
  if (!horario.horarioAtivo) {
    return;
  }

  const bool deveLigar = horarioIncluiMomento(horario, relogioSistema.relogioHora, relogioSistema.relogioMinuto);
  if (saidaRele.saidaLigada != deveLigar) {
    aplicarEstadoSaida(saidaRele, deveLigar);
    Serial.printf("[HORARIO] %s -> %s\n", saidaRele.saidaNome, deveLigar ? "ON" : "OFF");
  }
}

void copiarTextoSeguro(char* destino, size_t tamanhoDestino, const char* texto) {
  if (destino == nullptr || tamanhoDestino == 0) {
    return;
  }

  snprintf(destino, tamanhoDestino, "%s", texto);
}

void formatarValorLcd(char* destino, size_t tamanhoDestino, float valor, uint8_t casasDecimais, const char* unidade) {
  if (!std::isfinite(valor)) {
    copiarTextoSeguro(destino, tamanhoDestino, "--");
    return;
  }

  snprintf(destino, tamanhoDestino, "%.*f%s", casasDecimais, valor, unidade);
}

void configurarSaidaSegura(SaidaDigital& saida) {
  digitalWrite(saida.saidaPino, saida.saidaNivelInativo);
  pinMode(saida.saidaPino, OUTPUT);
  saida.saidaLigada = false;
}

void aplicarEstadoSaida(SaidaDigital& saida, bool ligar) {
  digitalWrite(saida.saidaPino, ligar ? saida.saidaNivelAtivo : saida.saidaNivelInativo);
  saida.saidaLigada = ligar;
}

void definirEstadoBombaNutrienteInterno(uint8_t indiceBomba, bool ligada, bool temporizada) {
  if (indiceBomba >= BombaNutrienteTotal) {
    return;
  }

  aplicarEstadoSaida(saidasBombasNutrientes[indiceBomba], ligada);
  autodoseBombaTemporizada[indiceBomba] = temporizada && ligada;
  autodoseBombaDesligarMs[indiceBomba] = ligada && temporizada ? millis() + AUTODOSE_PULSO_BOMBAS_MS[indiceBomba] : 0;
}

bool existeAlgumaBombaNutrienteLigada(void) {
  for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
    if (saidasBombasNutrientes[indiceBomba].saidaLigada) {
      return true;
    }
  }

  return false;
}

bool existeAlgumaBombaAutodoseAtiva(void) {
  for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
    if (autodoseBombaTemporizada[indiceBomba]) {
      return true;
    }
  }

  return false;
}

void desligarBombasAutodose(void) {
  for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
    if (autodoseBombaTemporizada[indiceBomba]) {
      definirEstadoBombaNutrienteInterno(indiceBomba, false, false);
    }
  }
}

void atualizarBombasTemporizadasAutodose(uint32_t momentoAtualMs) {
  for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
    if (!autodoseBombaTemporizada[indiceBomba]) {
      continue;
    }

    if (static_cast<int32_t>(momentoAtualMs - autodoseBombaDesligarMs[indiceBomba]) >= 0) {
      definirEstadoBombaNutrienteInterno(indiceBomba, false, false);
      Serial.printf("[AUTODOSE] %s -> OFF\n", saidasBombasNutrientes[indiceBomba].saidaNome);
    }
  }
}

const char* textoEstadoAutodose(EstadoAutoDose estado) {
  switch (estado) {
    case EstadoAutoDose::Desligado:
      return "OFF";
    case EstadoAutoDose::Pronto:
      return "READY";
    case EstadoAutoDose::Doseando:
      return "DOSING";
    case EstadoAutoDose::Mistura:
      return "MIX";
    case EstadoAutoDose::TdsInvalido:
      return "TDS ERR";
    case EstadoAutoDose::Manual:
      return "MANUAL";
    case EstadoAutoDose::Bloqueado:
      return "LOCK";
    case EstadoAutoDose::TdsAlto:
      return "HIGH";
    default:
      return "--";
  }
}

void iniciarPassoAutodoseAtual(void) {
  if (autodoseIndiceBombaAtual >= BombaNutrienteTotal) {
    return;
  }

  definirEstadoBombaNutrienteInterno(autodoseIndiceBombaAtual, true, true);
  Serial.printf("[AUTODOSE] %s -> ON\n", saidasBombasNutrientes[autodoseIndiceBombaAtual].saidaNome);
  autodoseIndiceBombaAtual++;
}

void atualizarEstadoAutodose(uint32_t momentoAtualMs) {
  if (!sistemaAtivo) {
    desligarBombasAutodose();
    autodoseEstado = EstadoAutoDose::Desligado;
    autodoseIndiceBombaAtual = 0;
    autodoseCiclosExecutados = 0;
    return;
  }

  if (!autodoseAtivo) {
    desligarBombasAutodose();
    autodoseEstado = EstadoAutoDose::Desligado;
    autodoseIndiceBombaAtual = 0;
    autodoseCiclosExecutados = 0;
    return;
  }

  if (!leituraTdsValida || !std::isfinite(leituraTdsValorPpm)) {
    if (!existeAlgumaBombaAutodoseAtiva()) {
      autodoseEstado = EstadoAutoDose::TdsInvalido;
    }
    return;
  }

  if (leituraTdsValorPpm >= AUTODOSE_LIMITE_ALTO_PPM) {
    if (!existeAlgumaBombaAutodoseAtiva()) {
      autodoseEstado = EstadoAutoDose::TdsAlto;
      autodoseCiclosExecutados = 0;
    }
    return;
  }

  if (existeAlgumaBombaNutrienteLigada() && !existeAlgumaBombaAutodoseAtiva() && autodoseEstado != EstadoAutoDose::Doseando) {
    autodoseEstado = EstadoAutoDose::Manual;
    return;
  }

  if (autodoseEstado == EstadoAutoDose::Bloqueado) {
    if (static_cast<int32_t>(momentoAtualMs - autodoseBloqueadoAteMs) < 0) {
      return;
    }

    autodoseCiclosExecutados = 0;
    autodoseEstado = EstadoAutoDose::Pronto;
  }

  if (autodoseEstado == EstadoAutoDose::Doseando) {
    if (existeAlgumaBombaAutodoseAtiva()) {
      return;
    }

    if (autodoseIndiceBombaAtual < BombaNutrienteTotal) {
      iniciarPassoAutodoseAtual();
      return;
    }

    autodoseEstado = EstadoAutoDose::Mistura;
    autodoseMomentoEstadoMs = momentoAtualMs;
    autodoseCiclosExecutados++;
    return;
  }

  if (autodoseEstado == EstadoAutoDose::Mistura) {
    if (momentoAtualMs - autodoseMomentoEstadoMs < AUTODOSE_MISTURA_MS) {
      return;
    }

    if (!leituraTdsValida || !std::isfinite(leituraTdsValorPpm)) {
      autodoseEstado = EstadoAutoDose::TdsInvalido;
      return;
    }

    if (leituraTdsValorPpm < AUTODOSE_TRIGGER_PPM) {
      if (autodoseCiclosExecutados >= AUTODOSE_MAX_CICLOS) {
        autodoseEstado = EstadoAutoDose::Bloqueado;
        autodoseBloqueadoAteMs = momentoAtualMs + AUTODOSE_BLOQUEIO_MS;
        return;
      }

      autodoseEstado = EstadoAutoDose::Doseando;
      autodoseIndiceBombaAtual = 0;
      iniciarPassoAutodoseAtual();
      return;
    }

    autodoseEstado = leituraTdsValorPpm >= AUTODOSE_LIMITE_ALTO_PPM ? EstadoAutoDose::TdsAlto : EstadoAutoDose::Pronto;
    autodoseCiclosExecutados = 0;
    return;
  }

  if (leituraTdsValorPpm < AUTODOSE_TRIGGER_PPM) {
    autodoseEstado = EstadoAutoDose::Doseando;
    autodoseIndiceBombaAtual = 0;
    autodoseCiclosExecutados = 0;
    iniciarPassoAutodoseAtual();
    return;
  }

  autodoseEstado = EstadoAutoDose::Pronto;
}

float lerMediaAdcMv(uint8_t pinoAdc, uint8_t numeroAmostras) {
  uint32_t somaContagens = 0;

  for (uint8_t indiceAmostra = 0; indiceAmostra < numeroAmostras; indiceAmostra++) {
    somaContagens += static_cast<uint32_t>(analogRead(pinoAdc));
    delay(4);
  }

  const float mediaContagens = static_cast<float>(somaContagens) / static_cast<float>(numeroAmostras);
  return (mediaContagens / ADC_CONTAGEM_MAXIMA) * ADC_REFERENCIA_MV;
}

float lerMediaAdcFiltradaMv(uint8_t pinoAdc, uint8_t numeroAmostras) {
  uint16_t amostras[ADC_NUMERO_AMOSTRAS_TDS] = {0};
  const uint8_t totalAmostras = std::min(numeroAmostras, ADC_NUMERO_AMOSTRAS_TDS);

  for (uint8_t indiceAmostra = 0; indiceAmostra < totalAmostras; indiceAmostra++) {
    amostras[indiceAmostra] = static_cast<uint16_t>(analogRead(pinoAdc));
    delay(4);
  }

  std::sort(amostras, amostras + totalAmostras);

  const uint8_t indiceInicial = totalAmostras / 5;
  const uint8_t indiceFinal = totalAmostras - indiceInicial;
  uint32_t somaContagens = 0;
  uint8_t totalUtil = 0;

  for (uint8_t indice = indiceInicial; indice < indiceFinal; indice++) {
    somaContagens += amostras[indice];
    totalUtil++;
  }

  if (totalUtil == 0) {
    return NAN;
  }

  const float mediaContagens = static_cast<float>(somaContagens) / static_cast<float>(totalUtil);
  return (mediaContagens / ADC_CONTAGEM_MAXIMA) * ADC_REFERENCIA_MV;
}

float calcularTdsPpm(float tensaoMv, float temperaturaC) {
  if (!std::isfinite(tensaoMv)) {
    return NAN;
  }

  const float tensaoV = tensaoMv / 1000.0f;
  const float ecBrutaUsCm =
      133.42f * tensaoV * tensaoV * tensaoV -
      255.86f * tensaoV * tensaoV +
      857.39f * tensaoV;

  const float temperaturaCompensacao = std::isfinite(temperaturaC) ? temperaturaC : TEMPERATURA_COMPENSACAO_PADRAO_C;
  const float ecCompensadaUsCm = ecBrutaUsCm / (1.0f + 0.02f * (temperaturaCompensacao - 25.0f));

  return std::max(0.0f, ecCompensadaUsCm * 0.5f);
}

float filtrarValorTds(float tdsPpmNovo, bool& leituraFoiFiltrada) {
  leituraFoiFiltrada = false;

  if (!leituraTdsTemHistorico || !std::isfinite(leituraTdsUltimoValorPpm)) {
    leituraTdsUltimoValorPpm = tdsPpmNovo;
    leituraTdsTemHistorico = true;
    return tdsPpmNovo;
  }

  float tdsPpmLimitado = tdsPpmNovo;
  const float deltaTds = tdsPpmNovo - leituraTdsUltimoValorPpm;
  if (fabsf(deltaTds) > TDS_VARIACAO_MAXIMA_POR_CICLO) {
    tdsPpmLimitado = leituraTdsUltimoValorPpm +
                     (deltaTds > 0.0f ? TDS_VARIACAO_MAXIMA_POR_CICLO : -TDS_VARIACAO_MAXIMA_POR_CICLO);
    leituraFoiFiltrada = true;
  }

  leituraTdsUltimoValorPpm =
      (leituraTdsUltimoValorPpm * (1.0f - TDS_FATOR_SUAVIZACAO)) +
      (tdsPpmLimitado * TDS_FATOR_SUAVIZACAO);

  return leituraTdsUltimoValorPpm;
}

float lerDs18b20TemperaturaAguaC(void) {
  const float temperaturaC = sensorDs18b20.getTempCByIndex(0);
  if (temperaturaC == DEVICE_DISCONNECTED_C) {
    return NAN;
  }
  return temperaturaC;
}

float lerPhValor(void) {
  const float temperaturaCompensacao =
      std::isfinite(leituraDs18b20TemperaturaAguaC) ? leituraDs18b20TemperaturaAguaC : TEMPERATURA_COMPENSACAO_PADRAO_C;
  leituraPhTensaoMv = lerMediaAdcMv(PINO_PH_ANALOGICO, ADC_NUMERO_AMOSTRAS_GERAL);
  return sensorPh.readPH(leituraPhTensaoMv, temperaturaCompensacao);
}

float lerTdsValorPpm(void) {
  leituraTdsTensaoMv = lerMediaAdcFiltradaMv(PINO_TDS_ANALOGICO, ADC_NUMERO_AMOSTRAS_TDS);

  if (!std::isfinite(leituraTdsTensaoMv)) {
    copiarTextoSeguro(leituraTdsEstado, sizeof(leituraTdsEstado), "ERR");
    leituraTdsValida = false;
    return leituraTdsTemHistorico ? leituraTdsUltimoValorPpm : NAN;
  }

  if (leituraTdsTensaoMv < TDS_TENSAO_MINIMA_VALIDA_MV) {
    copiarTextoSeguro(leituraTdsEstado, sizeof(leituraTdsEstado), "LOW");
    leituraTdsValida = true;
    leituraTdsUltimoValorPpm = 0.0f;
    leituraTdsTemHistorico = true;
    return 0.0f;
  }

  if (leituraTdsTensaoMv > TDS_TENSAO_MAXIMA_VALIDA_MV) {
    copiarTextoSeguro(leituraTdsEstado, sizeof(leituraTdsEstado), "OVR");
    leituraTdsValida = false;
    return leituraTdsTemHistorico ? leituraTdsUltimoValorPpm : NAN;
  }

  const float tdsPpmBruto = calcularTdsPpm(leituraTdsTensaoMv, leituraDs18b20TemperaturaAguaC);
  if (!std::isfinite(tdsPpmBruto) || tdsPpmBruto > TDS_PPM_MAXIMO_VALIDO) {
    copiarTextoSeguro(leituraTdsEstado, sizeof(leituraTdsEstado), "INV");
    leituraTdsValida = false;
    return leituraTdsTemHistorico ? leituraTdsUltimoValorPpm : NAN;
  }

  bool leituraFoiFiltrada = false;
  const float tdsPpmFiltrado = filtrarValorTds(tdsPpmBruto, leituraFoiFiltrada);
  copiarTextoSeguro(leituraTdsEstado, sizeof(leituraTdsEstado), leituraFoiFiltrada ? "FILT" : "OK");
  leituraTdsValida = true;
  return tdsPpmFiltrado;
}

float lerTurbidezTensaoMv(void) {
  return lerMediaAdcMv(PINO_TURBIDEZ_ANALOGICO, ADC_NUMERO_AMOSTRAS_GERAL);
}

void atualizarLeituraDht22(void) {
  const uint32_t momentoAtualMs = millis();
  if ((dht22UltimaLeituraMs != 0) && (momentoAtualMs - dht22UltimaLeituraMs < DHT22_INTERVALO_LEITURA_MS)) {
    return;
  }

  dht22UltimaLeituraMs = momentoAtualMs;

  const float humidadeLida = sensorDht22.readHumidity();
  const float temperaturaLida = sensorDht22.readTemperature();

  if (std::isfinite(humidadeLida)) {
    leituraDht22HumidadePercent = humidadeLida;
  }

  if (std::isfinite(temperaturaLida)) {
    leituraDht22TemperaturaAmbienteC = temperaturaLida;
  }
}

bool lerGroveNivelFrame(TwoWire& barramentoI2c, uint8_t enderecoBaixo, uint8_t enderecoAlto, uint8_t* bufferBaixo, uint8_t* bufferAlto) {
  const int bytesBaixos = barramentoI2c.requestFrom(static_cast<int>(enderecoBaixo), 8);
  if (bytesBaixos != 8) {
    return false;
  }

  for (int indice = 0; indice < 8; indice++) {
    bufferBaixo[indice] = barramentoI2c.read();
  }

  const int bytesAltos = barramentoI2c.requestFrom(static_cast<int>(enderecoAlto), 12);
  if (bytesAltos != 12) {
    return false;
  }

  for (int indice = 0; indice < 12; indice++) {
    bufferAlto[indice] = barramentoI2c.read();
  }

  return true;
}

int converterGroveNivelSecoes(const uint8_t* bufferBaixo, const uint8_t* bufferAlto) {
  constexpr int limiarToque = 100;
  uint32_t mascaraToque = 0;

  for (int indice = 0; indice < 8; indice++) {
    if (bufferBaixo[indice] > limiarToque) {
      mascaraToque |= static_cast<uint32_t>(1) << indice;
    }
  }

  for (int indice = 0; indice < 12; indice++) {
    if (bufferAlto[indice] > limiarToque) {
      mascaraToque |= static_cast<uint32_t>(1) << (8 + indice);
    }
  }

  int secoesAtivas = 0;
  while (mascaraToque & 0x01u) {
    secoesAtivas++;
    mascaraToque >>= 1;
  }

  return secoesAtivas;
}

float lerNivelTanqueCm(TwoWire& barramentoI2c) {
  uint8_t bufferBaixo[8] = {0};
  uint8_t bufferAlto[12] = {0};

  bool leituraOk = lerGroveNivelFrame(
      barramentoI2c,
      GROVE_NIVEL_ENDERECO_BAIXO_PRIMARIO,
      GROVE_NIVEL_ENDERECO_ALTO_PRIMARIO,
      bufferBaixo,
      bufferAlto);

  if (!leituraOk) {
    leituraOk = lerGroveNivelFrame(
        barramentoI2c,
        GROVE_NIVEL_ENDERECO_BAIXO_ALTERNATIVO,
        GROVE_NIVEL_ENDERECO_ALTO_ALTERNATIVO,
        bufferBaixo,
        bufferAlto);
  }

  if (!leituraOk) {
    return NAN;
  }

  const int secoesAtivas = converterGroveNivelSecoes(bufferBaixo, bufferAlto);
  return constrain(static_cast<float>(secoesAtivas) * 0.5f, 0.0f, GROVE_NIVEL_FAIXA_SENSOR_CM);
}

float converterNivelParaPercent(float nivelCm, float alturaTotalCm) {
  if (!std::isfinite(nivelCm) || alturaTotalCm <= 0.0f) {
    return NAN;
  }

  return constrain((nivelCm / alturaTotalCm) * 100.0f, 0.0f, 100.0f);
}

float sanitizarValorFaixa(float valor, float minimo, float maximo) {
  if (!std::isfinite(valor)) {
    return NAN;
  }

  if (valor < minimo || valor > maximo) {
    return NAN;
  }

  return valor;
}

float calcularNivelAguaTelemetriaPercent(void) {
  const bool tanque1Valido = std::isfinite(leituraTanque1Percent);
  const bool tanque2Valido = std::isfinite(leituraTanque2Percent);

  if (tanque1Valido && tanque2Valido) {
    return (leituraTanque1Percent + leituraTanque2Percent) * 0.5f;
  }

  if (tanque1Valido) {
    return leituraTanque1Percent;
  }

  if (tanque2Valido) {
    return leituraTanque2Percent;
  }

  return NAN;
}

bool lerEstadoReposicao(void) {
  const bool sensorAtivo = REPOSICAO_SENSOR_ATIVO_EM_BAIXO
                               ? (digitalRead(PINO_REPOSICAO_SENSOR_LIMITE) == LOW)
                               : (digitalRead(PINO_REPOSICAO_SENSOR_LIMITE) == HIGH);
  return sensorAtivo;
}

const char* textoReposicao(void) {
  return leituraReposicaoOk ? "OK" : "LOW";
}

const char* textoSaida(bool ligada) {
  return ligada ? "ON" : "OFF";
}

const char* textoTurbidez(float tensaoMv) {
  if (!std::isfinite(tensaoMv)) {
    return "ERR";
  }
  if (tensaoMv >= TURBIDEZ_TENSAO_LIMPA_MV) {
    return "CLEAR";
  }
  if (tensaoMv <= TURBIDEZ_TENSAO_SUJA_MV) {
    return "DIRTY";
  }
  return "MID";
}

const char* textoEstadoSistemaBlynk(void) {
  if (!sistemaAtivo) {
    return "DISABLED";
  }

  if (!leituraReposicaoOk) {
    return "REFILL LOW";
  }

  if (!leituraTdsValida) {
    return "TDS INVALID";
  }

  if (existeAlgumaBombaAutodoseAtiva()) {
    return "AUTO DOSING";
  }

  if (existeAlgumaBombaNutrienteLigada()) {
    return "MANUAL PUMP";
  }

  if (autodoseAtivo) {
    switch (autodoseEstado) {
      case EstadoAutoDose::Pronto:
        return "AUTO READY";
      case EstadoAutoDose::Mistura:
        return "AUTO MIX";
      case EstadoAutoDose::Bloqueado:
        return "AUTO LOCK";
      case EstadoAutoDose::TdsAlto:
        return "TDS HIGH";
      case EstadoAutoDose::TdsInvalido:
        return "TDS INVALID";
      case EstadoAutoDose::Manual:
        return "MANUAL HOLD";
      case EstadoAutoDose::Doseando:
        return "AUTO DOSING";
      case EstadoAutoDose::Desligado:
      default:
        return "AUTO OFF";
    }
  }

  return "MANUAL READY";
}

void atualizarDadosLcd(void) {
  formatarValorLcd(HydroLcdData::lcdValorTemperaturaAgua, sizeof(HydroLcdData::lcdValorTemperaturaAgua), leituraDs18b20TemperaturaAguaC, 1, "C");
  formatarValorLcd(HydroLcdData::lcdValorPh, sizeof(HydroLcdData::lcdValorPh), leituraPhValor, 2, "");

  if (leituraTdsValida && std::isfinite(leituraTdsValorPpm)) {
    formatarValorLcd(HydroLcdData::lcdValorTds, sizeof(HydroLcdData::lcdValorTds), leituraTdsValorPpm, 0, "ppm");
  } else if (std::isfinite(leituraTdsValorPpm)) {
    formatarValorLcd(HydroLcdData::lcdValorTds, sizeof(HydroLcdData::lcdValorTds), leituraTdsValorPpm, 0, "ppm");
  } else {
    copiarTextoSeguro(HydroLcdData::lcdValorTds, sizeof(HydroLcdData::lcdValorTds), "--");
  }

  formatarValorLcd(HydroLcdData::lcdValorTdsTensao, sizeof(HydroLcdData::lcdValorTdsTensao), leituraTdsTensaoMv, 0, "mV");
  copiarTextoSeguro(HydroLcdData::lcdValorTdsEstado, sizeof(HydroLcdData::lcdValorTdsEstado), leituraTdsEstado);
  formatarValorLcd(HydroLcdData::lcdValorTurbidez, sizeof(HydroLcdData::lcdValorTurbidez), leituraTurbidezTensaoMv, 0, "mV");

  formatarValorLcd(HydroLcdData::lcdValorTanque1, sizeof(HydroLcdData::lcdValorTanque1), leituraTanque1Percent, 0, "%");
  formatarValorLcd(HydroLcdData::lcdValorTanque2, sizeof(HydroLcdData::lcdValorTanque2), leituraTanque2Percent, 0, "%");
  copiarTextoSeguro(HydroLcdData::lcdValorReposicao, sizeof(HydroLcdData::lcdValorReposicao), textoReposicao());

  formatarValorLcd(HydroLcdData::lcdValorTemperaturaAmbiente, sizeof(HydroLcdData::lcdValorTemperaturaAmbiente), leituraDht22TemperaturaAmbienteC, 1, "C");
  formatarValorLcd(HydroLcdData::lcdValorHumidadeAmbiente, sizeof(HydroLcdData::lcdValorHumidadeAmbiente), leituraDht22HumidadePercent, 0, "%");

  copiarTextoSeguro(HydroLcdData::lcdValorSaidaMicro, sizeof(HydroLcdData::lcdValorSaidaMicro), textoSaida(saidasBombasNutrientes[BombaMicro].saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorSaidaCalcio, sizeof(HydroLcdData::lcdValorSaidaCalcio), textoSaida(saidasBombasNutrientes[BombaCalcio].saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorSaidaPotassio, sizeof(HydroLcdData::lcdValorSaidaPotassio), textoSaida(saidasBombasNutrientes[BombaPotassio].saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorSaidaMagnesio, sizeof(HydroLcdData::lcdValorSaidaMagnesio), textoSaida(saidasBombasNutrientes[BombaMagnesio].saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorSaidaCirculacao, sizeof(HydroLcdData::lcdValorSaidaCirculacao), textoSaida(saidaReleCirculacao.saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorSaidaLuz, sizeof(HydroLcdData::lcdValorSaidaLuz), textoSaida(saidaReleLuz.saidaLigada));
  copiarTextoSeguro(HydroLcdData::lcdValorAutoDoseEstado, sizeof(HydroLcdData::lcdValorAutoDoseEstado), textoEstadoAutodose(autodoseEstado));
  copiarTextoSeguro(HydroLcdData::lcdValorAutoDoseModo, sizeof(HydroLcdData::lcdValorAutoDoseModo), autodoseAtivo ? "ON" : "OFF");
}

void imprimirEstadoSerial(void) {
  Serial.printf(
      "pH=%.2f pHmV=%.0f TDS=%.0fppm TDSmV=%.0f TDSestado=%s Turb=%.0fmV(%s) Agua=%.1fC Ar=%.1fC Hum=%.0f%% T1=%.0f%% T2=%.0f%% Repo=%s Micro=%s Cal=%s Pot=%s Mag=%s Circ=%s Luz=%s Auto=%s Estado=%s\n",
      leituraPhValor,
      leituraPhTensaoMv,
      leituraTdsValorPpm,
      leituraTdsTensaoMv,
      leituraTdsEstado,
      leituraTurbidezTensaoMv,
      textoTurbidez(leituraTurbidezTensaoMv),
      leituraDs18b20TemperaturaAguaC,
      leituraDht22TemperaturaAmbienteC,
      leituraDht22HumidadePercent,
      leituraTanque1Percent,
      leituraTanque2Percent,
      textoReposicao(),
      textoSaida(saidasBombasNutrientes[BombaMicro].saidaLigada),
      textoSaida(saidasBombasNutrientes[BombaCalcio].saidaLigada),
      textoSaida(saidasBombasNutrientes[BombaPotassio].saidaLigada),
      textoSaida(saidasBombasNutrientes[BombaMagnesio].saidaLigada),
      textoSaida(saidaReleCirculacao.saidaLigada),
      textoSaida(saidaReleLuz.saidaLigada),
      autodoseAtivo ? "ON" : "OFF",
      textoEstadoAutodose(autodoseEstado));
}

}  // namespace

bool hydroObterEstadoBombaNutriente(uint8_t indiceBomba) {
  if (indiceBomba >= BombaNutrienteTotal) {
    return false;
  }

  return saidasBombasNutrientes[indiceBomba].saidaLigada;
}

void hydroDefinirEstadoBombaNutriente(uint8_t indiceBomba, bool ligada) {
  if (indiceBomba >= BombaNutrienteTotal) {
    return;
  }

  definirEstadoBombaNutrienteInterno(indiceBomba, ligada, false);
  Serial.printf("[GPIO] %s -> %s (GPIO %u)\n",
                saidasBombasNutrientes[indiceBomba].saidaNome,
                ligada ? "ON" : "OFF",
                saidasBombasNutrientes[indiceBomba].saidaPino);
  atualizarDadosLcd();
}

bool hydroObterEstadoReleCirculacao(void) {
  return saidaReleCirculacao.saidaLigada;
}

void hydroDefinirEstadoReleCirculacao(bool ligado) {
  aplicarEstadoSaida(saidaReleCirculacao, ligado);
  Serial.printf("[GPIO] %s -> %s (GPIO %u)\n",
                saidaReleCirculacao.saidaNome,
                ligado ? "ON" : "OFF",
                saidaReleCirculacao.saidaPino);
  atualizarDadosLcd();
}

bool hydroObterEstadoReleLuz(void) {
  return saidaReleLuz.saidaLigada;
}

void hydroDefinirEstadoReleLuz(bool ligado) {
  aplicarEstadoSaida(saidaReleLuz, ligado);
  Serial.printf("[GPIO] %s -> %s (GPIO %u)\n",
                saidaReleLuz.saidaNome,
                ligado ? "ON" : "OFF",
                saidaReleLuz.saidaPino);
  atualizarDadosLcd();
}

bool hydroObterAutoDoseAtivo(void) {
  return autodoseAtivo;
}

void hydroDefinirAutoDoseAtivo(bool ativo) {
  autodoseAtivo = ativo;

  if (!autodoseAtivo) {
    desligarBombasAutodose();
    autodoseEstado = EstadoAutoDose::Desligado;
    autodoseIndiceBombaAtual = 0;
    autodoseCiclosExecutados = 0;
  } else if (autodoseEstado == EstadoAutoDose::Desligado) {
    autodoseEstado = EstadoAutoDose::Pronto;
  }

  atualizarDadosLcd();
}

HydroAutoDoseEstado hydroObterAutoDoseEstado(void) {
  HydroAutoDoseEstado estado = {};
  estado.autoDoseAtivo = autodoseAtivo;
  estado.autoDoseTriggerPpm = AUTODOSE_TRIGGER_PPM;
  estado.autoDoseTargetPpm = AUTODOSE_TARGET_PPM;
  snprintf(estado.autoDoseEstadoTexto, sizeof(estado.autoDoseEstadoTexto), "%s", textoEstadoAutodose(autodoseEstado));
  return estado;
}

bool hydroObterSistemaAtivo(void) {
  return sistemaAtivo;
}

void hydroDefinirSistemaAtivo(bool ativo) {
  sistemaAtivo = ativo;

  if (!sistemaAtivo) {
    hydroDefinirEstadoReleCirculacao(false);
    hydroDefinirEstadoReleLuz(false);
    for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
      definirEstadoBombaNutrienteInterno(indiceBomba, false, false);
    }
  }

  atualizarDadosLcd();
}

bool hydroExecutarDoseRemota(void) {
  if (!sistemaAtivo || !leituraReposicaoOk || !leituraTdsValida || existeAlgumaBombaNutrienteLigada()) {
    return false;
  }

  for (uint8_t indiceBomba = 0; indiceBomba < BombaNutrienteTotal; indiceBomba++) {
    definirEstadoBombaNutrienteInterno(indiceBomba, true, true);
    Serial.printf("[REMOTE DOSE] %s -> ON\n", saidasBombasNutrientes[indiceBomba].saidaNome);
  }

  atualizarDadosLcd();
  return true;
}

void hydroObterTelemetria(HydroTelemetria& telemetria) {
  telemetria.telemetriaPh = sanitizarValorFaixa(leituraPhValor, 0.0f, 14.0f);
  telemetria.telemetriaTdsPpm =
      leituraTdsValida ? sanitizarValorFaixa(leituraTdsValorPpm, 0.0f, TDS_PPM_MAXIMO_VALIDO) : NAN;
  telemetria.telemetriaTemperaturaAguaC = sanitizarValorFaixa(leituraDs18b20TemperaturaAguaC, 0.0f, 60.0f);
  telemetria.telemetriaTemperaturaArC = sanitizarValorFaixa(leituraDht22TemperaturaAmbienteC, -20.0f, 80.0f);
  telemetria.telemetriaHumidadePercent = sanitizarValorFaixa(leituraDht22HumidadePercent, 0.0f, 100.0f);
  telemetria.telemetriaNivelAguaPercent = sanitizarValorFaixa(calcularNivelAguaTelemetriaPercent(), 0.0f, 100.0f);
  telemetria.telemetriaTdsValida = leituraTdsValida;
  telemetria.telemetriaSistemaAtivo = sistemaAtivo;
  telemetria.telemetriaAutoDoseAtivo = autodoseAtivo;

  snprintf(telemetria.telemetriaHoraTexto,
           sizeof(telemetria.telemetriaHoraTexto),
           "%02u:%02u:%02u",
           relogioSistema.relogioHora,
           relogioSistema.relogioMinuto,
           relogioSistema.relogioSegundo);

  snprintf(telemetria.telemetriaEstadoSistema,
           sizeof(telemetria.telemetriaEstadoSistema),
           "%s",
           textoEstadoSistemaBlynk());
}

void hydroObterHoraAtual(uint8_t& hora, uint8_t& minuto, uint8_t& segundo) {
  hora = relogioSistema.relogioHora;
  minuto = relogioSistema.relogioMinuto;
  segundo = relogioSistema.relogioSegundo;
}

void hydroDefinirHoraAtual(uint8_t hora, uint8_t minuto, uint8_t segundo) {
  relogioSistema.relogioHora = hora % 24;
  relogioSistema.relogioMinuto = minuto % 60;
  relogioSistema.relogioSegundo = segundo % 60;
  relogioSistema.relogioUltimoTickMs = millis();
  atualizarDadosLcd();
}

HydroHorarioSimples hydroObterHorarioLuz(void) {
  return horarioSistemaLuz;
}

void hydroDefinirHorarioLuz(const HydroHorarioSimples& horario) {
  horarioSistemaLuz = horario;
  normalizarHorario(horarioSistemaLuz);
  atualizarDadosLcd();
}

HydroHorarioSimples hydroObterHorarioCirculacao(void) {
  return horarioSistemaCirculacao;
}

void hydroDefinirHorarioCirculacao(const HydroHorarioSimples& horario) {
  horarioSistemaCirculacao = horario;
  normalizarHorario(horarioSistemaCirculacao);
  atualizarDadosLcd();
}

void setup() {
  configurarSaidaSegura(saidasBombasNutrientes[BombaMicro]);
  configurarSaidaSegura(saidasBombasNutrientes[BombaCalcio]);
  configurarSaidaSegura(saidasBombasNutrientes[BombaPotassio]);
  configurarSaidaSegura(saidasBombasNutrientes[BombaMagnesio]);
  configurarSaidaSegura(saidaReleCirculacao);
  configurarSaidaSegura(saidaReleLuz);

  Serial.begin(115200);
  delay(200);
  inicializarRelogioSistema();

  Wire.begin(PINO_TANQUE1_SDA, PINO_TANQUE1_SCL);
  Wire.setClock(100000);
  barramentoI2cTanque2.begin(PINO_TANQUE2_SDA, PINO_TANQUE2_SCL, 100000);

  analogReadResolution(12);
  analogSetPinAttenuation(PINO_PH_ANALOGICO, ADC_11db);
  analogSetPinAttenuation(PINO_TDS_ANALOGICO, ADC_11db);
  analogSetPinAttenuation(PINO_TURBIDEZ_ANALOGICO, ADC_11db);

  pinMode(PINO_REPOSICAO_SENSOR_LIMITE, INPUT_PULLUP);

  sensorDs18b20.begin();
  sensorDs18b20.setWaitForConversion(false);
  sensorDs18b20.requestTemperatures();
  ds18b20UltimoPedidoMs = millis();
  ds18b20ConversaoPendente = true;

  sensorPh.begin();
  sensorDht22.begin();

  atualizarDadosLcd();
  Garden::initialize();
  initBlynk();

  Serial.println("[INFO] Sistema simplificado iniciado.");
}

void loop() {
  const uint32_t momentoAtualMs = millis();
  atualizarRelogioSistema();
  handleBlynkCommands();

  if (sistemaAtivo) {
    aplicarHorarioRele(saidaReleLuz, horarioSistemaLuz);
    aplicarHorarioRele(saidaReleCirculacao, horarioSistemaCirculacao);
  } else {
    if (saidaReleLuz.saidaLigada) {
      aplicarEstadoSaida(saidaReleLuz, false);
    }
    if (saidaReleCirculacao.saidaLigada) {
      aplicarEstadoSaida(saidaReleCirculacao, false);
    }
  }

  atualizarBombasTemporizadasAutodose(momentoAtualMs);
  atualizarEstadoAutodose(momentoAtualMs);

  if (ds18b20ConversaoPendente && (momentoAtualMs - ds18b20UltimoPedidoMs >= DS18B20_TEMPO_CONVERSAO_MS)) {
    leituraDs18b20TemperaturaAguaC = lerDs18b20TemperaturaAguaC();
    ds18b20ConversaoPendente = false;
  }

  if (momentoAtualMs - sistemaUltimoCicloSensoresMs >= SISTEMA_INTERVALO_LEITURA_MS) {
    sistemaUltimoCicloSensoresMs = momentoAtualMs;

    if (!ds18b20ConversaoPendente) {
      sensorDs18b20.requestTemperatures();
      ds18b20UltimoPedidoMs = momentoAtualMs;
      ds18b20ConversaoPendente = true;
    }

    leituraPhValor = lerPhValor();
    leituraTdsValorPpm = lerTdsValorPpm();
    leituraTurbidezTensaoMv = lerTurbidezTensaoMv();
    atualizarLeituraDht22();

    leituraTanque1NivelCm = lerNivelTanqueCm(Wire);
    leituraTanque2NivelCm = lerNivelTanqueCm(barramentoI2cTanque2);
    leituraTanque1Percent = converterNivelParaPercent(leituraTanque1NivelCm, TANQUE_1_ALTURA_CM);
    leituraTanque2Percent = converterNivelParaPercent(leituraTanque2NivelCm, TANQUE_2_ALTURA_CM);
    leituraReposicaoOk = lerEstadoReposicao();

    atualizarDadosLcd();
    imprimirEstadoSerial();
  }

  updateBlynkSensors();
  Garden::update();
}

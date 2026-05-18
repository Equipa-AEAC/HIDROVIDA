#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DFRobot_ESP_PH.h>
#ifdef ReceivedBufferLength
#undef ReceivedBufferLength
#endif
#include <GravityTDS.h>
#include <DHT.h>
#include <garden.h>
#include <hydro_lcd_data.h>
#include <hardware/pin_map.h>
#include <cstdio>
#include <cstring>

namespace {

// ============================================================================
// Configuracao de hardware
// ============================================================================

using HardwarePinos::PINO_BOMBA_MICRONUTRIENTES;
using HardwarePinos::PINO_BOMBA_NITRATO_CALCIO;
using HardwarePinos::PINO_BOMBA_NITRATO_POTASSIO;
using HardwarePinos::PINO_BOMBA_SULFATO_MAGNESIO;
using HardwarePinos::PINO_CO2_ANALOGICO;
using HardwarePinos::PINO_DHT22_DADOS;
using HardwarePinos::PINO_DS18B20_DADOS;
using HardwarePinos::PINO_PH_ANALOGICO;
using HardwarePinos::PINO_REPOSICAO_SENSOR_LIMITE;
using HardwarePinos::PINO_TANQUE1_SCL;
using HardwarePinos::PINO_TANQUE1_SDA;
using HardwarePinos::PINO_TANQUE2_SCL;
using HardwarePinos::PINO_TANQUE2_SDA;
using HardwarePinos::PINO_TDS_ANALOGICO;
using HardwarePinos::PINO_TURBIDEZ_ANALOGICO;

constexpr bool SENSOR_DHT22_ATIVO = true;
constexpr bool SENSOR_CO2_ATIVO = false;
constexpr bool SENSOR_REPOSICAO_ATIVO_EM_BAIXO = true;

constexpr uint8_t DHT22_TIPO = DHT22;
constexpr size_t EEPROM_TAMANHO_BYTES = 64;

// ============================================================================
// Temporizacao do sistema
// ============================================================================

constexpr uint32_t SISTEMA_INTERVALO_LEITURA_MS = 1000;
constexpr uint32_t DS18B20_TEMPO_CONVERSAO_MS = 750;
constexpr uint32_t DHT22_INTERVALO_LEITURA_MS = 2500;
constexpr uint32_t NUTRIENTES_COOLDOWN_DOSAGEM_MS = 120000;
constexpr uint32_t NUTRIENTES_TEMPO_MISTURA_MS = 45000;

// ============================================================================
// Parametros ADC
// ============================================================================

constexpr uint8_t ADC_NUMERO_AMOSTRAS = 10;
constexpr float ADC_VALOR_MAXIMO = 4095.0f;
constexpr float ADC_REFERENCIA_MV = 3300.0f;
constexpr float TEMPERATURA_PADRAO_C = 25.0f;

// ============================================================================
// Limites de operacao
// ============================================================================

constexpr float DS18B20_TEMPERATURA_MIN_C = 18.0f;
constexpr float DS18B20_TEMPERATURA_MAX_C = 28.0f;

constexpr float TURBIDEZ_TENSAO_LIMPA_MV = 2500.0f;
constexpr float TURBIDEZ_TENSAO_SUJA_MV = 1500.0f;

constexpr float DHT22_HUMIDADE_MIN_PERCENT = 40.0f;
constexpr float DHT22_HUMIDADE_MAX_PERCENT = 75.0f;
constexpr float CO2_LIMITE_ALTO_PPM = 1400.0f;

constexpr float TANQUE1_ALTURA_TOTAL_CM = 10.0f;
constexpr float TANQUE2_ALTURA_TOTAL_CM = 18.0f;
constexpr float GROVE_NIVEL_FAIXA_SENSOR_CM = 10.0f;
constexpr float TANQUE_PRINCIPAL_NIVEL_BAIXO_PERCENT = 20.0f;

// ============================================================================
// Enderecos I2C dos sensores Grove water level
// ============================================================================

constexpr uint8_t GROVE_NIVEL_ENDERECO_BAIXO_PRIMARIO = 0x77;
constexpr uint8_t GROVE_NIVEL_ENDERECO_ALTO_PRIMARIO = 0x78;
constexpr uint8_t GROVE_NIVEL_ENDERECO_BAIXO_ALTERNATIVO = 0x3B;
constexpr uint8_t GROVE_NIVEL_ENDERECO_ALTO_ALTERNATIVO = 0x3C;

// ============================================================================
// Parametros das bombas e EEPROM
// ============================================================================

constexpr uint8_t BOMBA_NIVEL_LIGADA = LOW;
constexpr uint8_t BOMBA_NIVEL_DESLIGADA = HIGH;

constexpr uint32_t EEPROM_ASSINATURA_CONFIG = 0x4844524Fu;
constexpr int EEPROM_ENDERECO_CONFIG = 0;
constexpr float TDS_AJUSTE_AR_PADRAO_PPM = 40.0f;
constexpr float TDS_MARGEM_AR_PPM = 5.0f;

// ============================================================================
// Estruturas de configuracao
// ============================================================================

struct ConfiguracaoSistema {
  uint32_t configuracaoAssinatura;
  float sensorTdsValorArPpm;
  float sensorTdsLimiteArPpm;
};

enum class FaseCrescimento : uint8_t {
  Germinacao,
  Vegetativo,
  Floracao,
};

enum class EstadoTanqueReposicao : uint8_t {
  Baixo,
  OK,
};

struct CanalBombaNutriente {
  uint8_t bombaPino;
  const char* bombaNome;
  float bombaMlPorSegundo;
  bool bombaLigada;
  uint32_t bombaMomentoDesligarMs;
};

struct PerfilCrescimento {
  FaseCrescimento perfilFase;
  const char* perfilNome;
  float perfilTdsAlvoPpm;
  float perfilTdsAcionamentoPpm;
  float perfilPhMinimo;
  float perfilPhMaximo;
  float perfilDoseMicronutrientesMl;
  float perfilDoseNitratoCalcioMl;
  float perfilDoseNitratoPotassioMl;
  float perfilDoseSulfatoMagnesioMl;
};

// ============================================================================
// Perfil ativo do sistema
// ============================================================================

constexpr FaseCrescimento PERFIL_CRESCIMENTO_ATIVO = FaseCrescimento::Vegetativo;

constexpr PerfilCrescimento PERFIS_CRESCIMENTO[] = {
    {FaseCrescimento::Germinacao, "GERM", 450.0f, 380.0f, 5.6f, 6.2f, 0.8f, 1.0f, 0.8f, 0.6f},
    {FaseCrescimento::Vegetativo, "VEG", 850.0f, 780.0f, 5.8f, 6.2f, 1.2f, 2.2f, 1.8f, 1.0f},
    {FaseCrescimento::Floracao, "FLOR", 1050.0f, 950.0f, 5.8f, 6.3f, 1.0f, 1.8f, 2.6f, 1.2f},
};

// ============================================================================
// Objetos de hardware
// ============================================================================

TwoWire barramentoI2cTanque2 = TwoWire(1);
OneWire barramentoOneWireDs18b20(PINO_DS18B20_DADOS);
DallasTemperature sensorDs18b20(&barramentoOneWireDs18b20);
DFRobot_ESP_PH sensorPh;
GravityTDS sensorTds;
DHT sensorDht22(PINO_DHT22_DADOS, DHT22_TIPO);

// ============================================================================
// Estado persistente do sistema
// ============================================================================

ConfiguracaoSistema configuracaoSistema = {
    EEPROM_ASSINATURA_CONFIG,
    0.0f,
    TDS_AJUSTE_AR_PADRAO_PPM,
};

CanalBombaNutriente canaisBombasNutrientes[] = {
    {PINO_BOMBA_MICRONUTRIENTES, "Micro", 1.00f, false, 0},
    {PINO_BOMBA_NITRATO_CALCIO, "CaNO3", 1.00f, false, 0},
    {PINO_BOMBA_NITRATO_POTASSIO, "KNO3", 1.00f, false, 0},
    {PINO_BOMBA_SULFATO_MAGNESIO, "MgSO4", 1.00f, false, 0},
};

// ============================================================================
// Leituras atuais dos sensores
// ============================================================================

float leituraDs18b20TemperaturaC = NAN;

float leituraPhValor = NAN;
float leituraPhTensaoMv = NAN;

float leituraTdsValorPpm = NAN;
float leituraTdsValorBrutoPpm = NAN;
float leituraTdsEcUsCm = NAN;
float leituraTdsTensaoMv = NAN;

float leituraTurbidezTensaoMv = NAN;
int leituraTurbidezAdc = -1;
int leituraTurbidezAdcMin = 4095;
int leituraTurbidezAdcMax = 0;

float leituraDht22HumidadePercent = NAN;
float leituraCo2ValorPpm = NAN;

float leituraTanque1NivelCm = NAN;
float leituraTanque2NivelCm = NAN;
float leituraTanque1Percent = NAN;
float leituraTanque2Percent = NAN;
EstadoTanqueReposicao leituraReposicaoEstado = EstadoTanqueReposicao::Baixo;

// ============================================================================
// Estado de controlo temporal
// ============================================================================

uint32_t sistemaUltimoCicloSensoresMs = 0;
uint32_t ds18b20UltimoPedidoMs = 0;
uint32_t dht22UltimaLeituraMs = 0;
uint32_t nutrientesUltimaDosagemMs = 0;
uint32_t nutrientesMisturaAteMs = 0;

bool ds18b20ConversaoPendente = false;
void guardarConfiguracaoSistema() {
  EEPROM.put(EEPROM_ENDERECO_CONFIG, configuracaoSistema);
  EEPROM.commit();
}

void carregarConfiguracaoSistema() {
  EEPROM.get(EEPROM_ENDERECO_CONFIG, configuracaoSistema);
  const bool configuracaoInvalida =
      configuracaoSistema.configuracaoAssinatura != EEPROM_ASSINATURA_CONFIG ||
      !isfinite(configuracaoSistema.sensorTdsLimiteArPpm) ||
      configuracaoSistema.sensorTdsLimiteArPpm < 0.0f ||
      configuracaoSistema.sensorTdsLimiteArPpm > 200.0f;

  if (configuracaoInvalida) {
    configuracaoSistema.configuracaoAssinatura = EEPROM_ASSINATURA_CONFIG;
    configuracaoSistema.sensorTdsValorArPpm = 0.0f;
    configuracaoSistema.sensorTdsLimiteArPpm = TDS_AJUSTE_AR_PADRAO_PPM;
    guardarConfiguracaoSistema();
  }
}

const PerfilCrescimento& obterPerfilCrescimentoAtivo() {
  for (const auto& perfilAtual : PERFIS_CRESCIMENTO) {
    if (perfilAtual.perfilFase == PERFIL_CRESCIMENTO_ATIVO) {
      return perfilAtual;
    }
  }
  return PERFIS_CRESCIMENTO[0];
}

// ============================================================================
// Funcoes auxiliares ADC
// ============================================================================

float calcularMediaAdcContagens(uint8_t pinoAdc, uint8_t numeroAmostras) {
  uint32_t somaLeituras = 0;
  for (uint8_t indiceAmostra = 0; indiceAmostra < numeroAmostras; indiceAmostra++) {
    somaLeituras += static_cast<uint32_t>(analogRead(pinoAdc));
    delay(10);
  }
  return static_cast<float>(somaLeituras) / static_cast<float>(numeroAmostras);
}

float calcularMediaAdcMv(uint8_t pinoAdc, uint8_t numeroAmostras) {
  const float mediaContagens = calcularMediaAdcContagens(pinoAdc, numeroAmostras);
  return (mediaContagens / ADC_VALOR_MAXIMO) * ADC_REFERENCIA_MV;
}

// ============================================================================
// Leituras dos sensores analogicos e digitais
// ============================================================================

float lerDs18b20TemperaturaC() {
  const float temperaturaC = sensorDs18b20.getTempCByIndex(0);
  if (temperaturaC == DEVICE_DISCONNECTED_C) {
    return NAN;
  }
  return temperaturaC;
}

float lerPhValor() {
  const float temperaturaCompensacao = isfinite(leituraDs18b20TemperaturaC) ? leituraDs18b20TemperaturaC : TEMPERATURA_PADRAO_C;
  leituraPhTensaoMv = calcularMediaAdcMv(PINO_PH_ANALOGICO, ADC_NUMERO_AMOSTRAS);
  return sensorPh.readPH(leituraPhTensaoMv, temperaturaCompensacao);
}

float lerTdsValorPpm(float temperaturaC) {
  const float temperaturaCompensacao = isfinite(temperaturaC) ? temperaturaC : TEMPERATURA_PADRAO_C;

  leituraTdsTensaoMv = calcularMediaAdcMv(PINO_TDS_ANALOGICO, ADC_NUMERO_AMOSTRAS);

  float somaTdsPpm = 0.0f;
  for (uint8_t indiceAmostra = 0; indiceAmostra < ADC_NUMERO_AMOSTRAS; indiceAmostra++) {
    sensorTds.setTemperature(temperaturaCompensacao);
    sensorTds.update();
    somaTdsPpm += sensorTds.getTdsValue();
    delay(20);
  }

  leituraTdsValorBrutoPpm = somaTdsPpm / static_cast<float>(ADC_NUMERO_AMOSTRAS);
  leituraTdsEcUsCm = sensorTds.getEcValue();

  if (!isfinite(leituraTdsValorBrutoPpm)) {
    return leituraTdsValorBrutoPpm;
  }
  if (leituraTdsValorBrutoPpm <= configuracaoSistema.sensorTdsLimiteArPpm) {
    return 0.0f;
  }
  return leituraTdsValorBrutoPpm;
}

float lerTurbidezTensaoMv() {
  const float mediaAdc = calcularMediaAdcContagens(PINO_TURBIDEZ_ANALOGICO, ADC_NUMERO_AMOSTRAS);

  leituraTurbidezAdc = static_cast<int>(mediaAdc + 0.5f);
  if (leituraTurbidezAdc < leituraTurbidezAdcMin) {
    leituraTurbidezAdcMin = leituraTurbidezAdc;
  }
  if (leituraTurbidezAdc > leituraTurbidezAdcMax) {
    leituraTurbidezAdcMax = leituraTurbidezAdc;
  }

  return (mediaAdc / ADC_VALOR_MAXIMO) * ADC_REFERENCIA_MV;
}

float lerDht22HumidadePercent() {
  if (!SENSOR_DHT22_ATIVO) {
    return NAN;
  }

  const uint32_t momentoAtualMs = millis();
  if ((dht22UltimaLeituraMs != 0) &&
      (momentoAtualMs - dht22UltimaLeituraMs < DHT22_INTERVALO_LEITURA_MS) &&
      isfinite(leituraDht22HumidadePercent)) {
    return leituraDht22HumidadePercent;
  }

  dht22UltimaLeituraMs = momentoAtualMs;
  const float humidadeLida = sensorDht22.readHumidity();
  if (isnan(humidadeLida)) {
    return NAN;
  }
  return constrain(humidadeLida, 0.0f, 100.0f);
}

float lerCo2Ppm() {
  if (!SENSOR_CO2_ATIVO) {
    return NAN;
  }

  const float tensaoCo2Mv = calcularMediaAdcMv(PINO_CO2_ANALOGICO, ADC_NUMERO_AMOSTRAS);
  return constrain((tensaoCo2Mv / ADC_REFERENCIA_MV) * 5000.0f, 0.0f, 5000.0f);
}

// ============================================================================
// Leitura dos sensores Grove de nivel de agua
// ============================================================================

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

float lerTanquePrincipalNivelCm(TwoWire& barramentoI2c) {
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

float converterTanqueNivelParaPercent(float nivelCm, float alturaTanqueCm) {
  if (!isfinite(nivelCm) || alturaTanqueCm <= 0.0f) {
    return NAN;
  }
  return constrain((nivelCm / alturaTanqueCm) * 100.0f, 0.0f, 100.0f);
}

EstadoTanqueReposicao lerReposicaoEstado() {
  const bool sensorAtivo = SENSOR_REPOSICAO_ATIVO_EM_BAIXO
                               ? (digitalRead(PINO_REPOSICAO_SENSOR_LIMITE) == LOW)
                               : (digitalRead(PINO_REPOSICAO_SENSOR_LIMITE) == HIGH);
  return sensorAtivo ? EstadoTanqueReposicao::OK : EstadoTanqueReposicao::Baixo;
}

// ============================================================================
// Funcoes auxiliares de estado
// ============================================================================

const char* textoEstadoReposicao(EstadoTanqueReposicao estadoReposicao) {
  return (estadoReposicao == EstadoTanqueReposicao::OK) ? "OK" : "LOW";
}

const char* textoEstadoFaixa(float valorAtual, float minimoAceitavel, float maximoAceitavel) {
  if (!isfinite(valorAtual)) {
    return "ERR";
  }
  if (valorAtual < minimoAceitavel) {
    return "LOW";
  }
  if (valorAtual > maximoAceitavel) {
    return "HIGH";
  }
  return "OK";
}

const char* textoEstadoTurbidez(float tensaoMv) {
  if (!isfinite(tensaoMv)) {
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

const char* textoEstadoDht22Humidade(float humidadePercent) {
  if (!isfinite(humidadePercent)) {
    return "NA";
  }
  if (humidadePercent < DHT22_HUMIDADE_MIN_PERCENT) {
    return "LOW";
  }
  if (humidadePercent > DHT22_HUMIDADE_MAX_PERCENT) {
    return "HIGH";
  }
  return "OK";
}

const char* textoEstadoCo2(float co2Ppm) {
  if (!isfinite(co2Ppm)) {
    return "NA";
  }
  if (co2Ppm > CO2_LIMITE_ALTO_PPM) {
    return "HIGH";
  }
  return "OK";
}

bool estadoExigeAviso(const char* textoEstado) {
 return strcmp(textoEstado, "OK") != 0 && strcmp(textoEstado, "CLEAR") != 0;
}

void formatarValorLcd(char* destino, size_t tamanhoDestino, float valor, uint8_t casasDecimais, const char* unidade) {
  if (!isfinite(valor)) {
    snprintf(destino, tamanhoDestino, "--");
    return;
  }

  snprintf(destino, tamanhoDestino, "%.*f%s", casasDecimais, valor, unidade);
}

void atualizarDadosLcd() {
  const PerfilCrescimento& perfilAtivo = obterPerfilCrescimentoAtivo();
  bool algumaBombaAtiva = false;

  for (const auto& canalBomba : canaisBombasNutrientes) {
    if (canalBomba.bombaLigada) {
      algumaBombaAtiva = true;
      break;
    }
  }

  formatarValorLcd(HydroLcdData::valorTemperatura, sizeof(HydroLcdData::valorTemperatura), leituraDs18b20TemperaturaC, 1, "C");
  formatarValorLcd(HydroLcdData::valorPh, sizeof(HydroLcdData::valorPh), leituraPhValor, 2, "");
  formatarValorLcd(HydroLcdData::valorTds, sizeof(HydroLcdData::valorTds), leituraTdsValorPpm, 0, "ppm");
  formatarValorLcd(HydroLcdData::valorTanque1, sizeof(HydroLcdData::valorTanque1), leituraTanque1Percent, 0, "%");
  formatarValorLcd(HydroLcdData::valorTanque2, sizeof(HydroLcdData::valorTanque2), leituraTanque2Percent, 0, "%");
  formatarValorLcd(HydroLcdData::valorTurbidez, sizeof(HydroLcdData::valorTurbidez), leituraTurbidezTensaoMv, 0, "mV");
  formatarValorLcd(HydroLcdData::valorHumidade, sizeof(HydroLcdData::valorHumidade), leituraDht22HumidadePercent, 0, "%");
  formatarValorLcd(HydroLcdData::valorCo2, sizeof(HydroLcdData::valorCo2), leituraCo2ValorPpm, 0, "ppm");

  snprintf(HydroLcdData::valorReposicao, sizeof(HydroLcdData::valorReposicao), "%s", textoEstadoReposicao(leituraReposicaoEstado));
  snprintf(HydroLcdData::valorFase, sizeof(HydroLcdData::valorFase), "%s", perfilAtivo.perfilNome);
  snprintf(HydroLcdData::valorDosagem, sizeof(HydroLcdData::valorDosagem), "%s", algumaBombaAtiva ? "ATIVA" : "ESPERA");
}

// ============================================================================
// Controlo das bombas de nutrientes
// ============================================================================

void atualizarBombaSaida(CanalBombaNutriente& canalBomba, bool ligarBomba) {
  digitalWrite(canalBomba.bombaPino, ligarBomba ? BOMBA_NIVEL_LIGADA : BOMBA_NIVEL_DESLIGADA);
  canalBomba.bombaLigada = ligarBomba;
  if (!ligarBomba) {
    canalBomba.bombaMomentoDesligarMs = 0;
  }
}

void desligarTodasBombasNutrientes() {
  for (auto& canalBomba : canaisBombasNutrientes) {
    atualizarBombaSaida(canalBomba, false);
  }
}

bool existeBombaNutrienteLigada() {
  for (const auto& canalBomba : canaisBombasNutrientes) {
    if (canalBomba.bombaLigada) {
      return true;
    }
  }
  return false;
}

void iniciarBombaPorTempoMs(CanalBombaNutriente& canalBomba, uint32_t tempoLigadaMs, uint32_t momentoAtualMs) {
  if (tempoLigadaMs == 0) {
    return;
  }
  atualizarBombaSaida(canalBomba, true);
  canalBomba.bombaMomentoDesligarMs = momentoAtualMs + tempoLigadaMs;
}

void iniciarBombaPorVolumeMl(CanalBombaNutriente& canalBomba, float volumeMl, uint32_t momentoAtualMs) {
  if (volumeMl <= 0.0f || canalBomba.bombaMlPorSegundo <= 0.0f) {
    return;
  }
  const uint32_t tempoLigadaMs = static_cast<uint32_t>((volumeMl / canalBomba.bombaMlPorSegundo) * 1000.0f);
  iniciarBombaPorTempoMs(canalBomba, tempoLigadaMs, momentoAtualMs);
}

void atualizarBombasNutrientes(uint32_t momentoAtualMs) {
  for (auto& canalBomba : canaisBombasNutrientes) {
    if (!canalBomba.bombaLigada) {
      continue;
    }
    if (static_cast<int32_t>(momentoAtualMs - canalBomba.bombaMomentoDesligarMs) >= 0) {
      atualizarBombaSaida(canalBomba, false);
    }
  }
}

bool nutrientesEmCooldown(uint32_t momentoAtualMs) {
  if (nutrientesUltimaDosagemMs == 0) {
    return false;
  }
  return momentoAtualMs - nutrientesUltimaDosagemMs < NUTRIENTES_COOLDOWN_DOSAGEM_MS;
}

bool nutrientesEmMistura(uint32_t momentoAtualMs) {
  return momentoAtualMs < nutrientesMisturaAteMs;
}

bool perfilPermiteDosagem(const PerfilCrescimento& perfilAtivo, uint32_t momentoAtualMs) {
  if (!isfinite(leituraPhValor) || !isfinite(leituraTdsValorPpm)) {
    return false;
  }
  if (leituraTdsValorPpm <= 0.0f) {
    return false;
  }
  if (leituraPhValor < perfilAtivo.perfilPhMinimo || leituraPhValor > perfilAtivo.perfilPhMaximo) {
    return false;
  }
  if (existeBombaNutrienteLigada()) {
    return false;
  }
  if (nutrientesEmCooldown(momentoAtualMs) || nutrientesEmMistura(momentoAtualMs)) {
    return false;
  }
  return leituraTdsValorPpm < perfilAtivo.perfilTdsAcionamentoPpm;
}

void iniciarDosagemAutomatica(const PerfilCrescimento& perfilAtivo, uint32_t momentoAtualMs) {
  const float tdsDeficitPpm = perfilAtivo.perfilTdsAlvoPpm - leituraTdsValorPpm;
  const float fatorCorrecao = constrain(tdsDeficitPpm / 100.0f, 0.6f, 2.0f);

  iniciarBombaPorVolumeMl(canaisBombasNutrientes[0], perfilAtivo.perfilDoseMicronutrientesMl * fatorCorrecao, momentoAtualMs);
  iniciarBombaPorVolumeMl(canaisBombasNutrientes[1], perfilAtivo.perfilDoseNitratoCalcioMl * fatorCorrecao, momentoAtualMs);
  iniciarBombaPorVolumeMl(canaisBombasNutrientes[2], perfilAtivo.perfilDoseNitratoPotassioMl * fatorCorrecao, momentoAtualMs);
  iniciarBombaPorVolumeMl(canaisBombasNutrientes[3], perfilAtivo.perfilDoseSulfatoMagnesioMl * fatorCorrecao, momentoAtualMs);

  nutrientesUltimaDosagemMs = momentoAtualMs;
  nutrientesMisturaAteMs = momentoAtualMs + NUTRIENTES_TEMPO_MISTURA_MS;
}

void atualizarDosagemAutomatica(uint32_t momentoAtualMs) {
  const PerfilCrescimento& perfilAtivo = obterPerfilCrescimentoAtivo();
  if (!perfilPermiteDosagem(perfilAtivo, momentoAtualMs)) {
    return;
  }
  iniciarDosagemAutomatica(perfilAtivo, momentoAtualMs);
}

void imprimirEstadoSerial() {
  const PerfilCrescimento& perfilAtivo = obterPerfilCrescimentoAtivo();
  Serial.printf(
      "Fase=%s Temp=%.1fC pH=%.2f TDS=%.0fppm bruto=%.0f EC=%.0fuS T1=%.0f%% T2=%.0f%% Repo=%s Turb=%.0fmV adc=%d Hum=%.0f%% CO2=%.0fppm Dose=%s\n",
      perfilAtivo.perfilNome,
      leituraDs18b20TemperaturaC,
      leituraPhValor,
      leituraTdsValorPpm,
      leituraTdsValorBrutoPpm,
      leituraTdsEcUsCm,
      leituraTanque1Percent,
      leituraTanque2Percent,
      textoEstadoReposicao(leituraReposicaoEstado),
      leituraTurbidezTensaoMv,
      leituraTurbidezAdc,
      leituraDht22HumidadePercent,
      leituraCo2ValorPpm,
      existeBombaNutrienteLigada() ? "ON" : "WAIT");
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);

  // Barramento I2C principal: LCD, RTC e sensor do tanque 1.
  Wire.begin(PINO_TANQUE1_SDA, PINO_TANQUE1_SCL);
  Wire.setClock(100000);

  // Segundo barramento I2C dedicado ao tanque principal 2.
  barramentoI2cTanque2.begin(PINO_TANQUE2_SDA, PINO_TANQUE2_SCL, 100000);

  // Configuracao dos canais ADC.
  analogReadResolution(12);
  analogSetPinAttenuation(PINO_PH_ANALOGICO, ADC_11db);
  analogSetPinAttenuation(PINO_TDS_ANALOGICO, ADC_11db);
  analogSetPinAttenuation(PINO_TURBIDEZ_ANALOGICO, ADC_11db);
  if (SENSOR_CO2_ATIVO) {
    analogSetPinAttenuation(PINO_CO2_ANALOGICO, ADC_11db);
  }

  EEPROM.begin(EEPROM_TAMANHO_BYTES);
  carregarConfiguracaoSistema();

  sensorDs18b20.begin();
  sensorDs18b20.setWaitForConversion(false);
  sensorDs18b20.requestTemperatures();
  ds18b20UltimoPedidoMs = millis();
  ds18b20ConversaoPendente = true;

  sensorPh.begin();

  sensorTds.setPin(PINO_TDS_ANALOGICO);
  sensorTds.setAref(3.3f);
  sensorTds.setAdcRange(4096.0f);
  sensorTds.begin();

  if (SENSOR_DHT22_ATIVO) {
    sensorDht22.begin();
  }

  pinMode(PINO_REPOSICAO_SENSOR_LIMITE, INPUT_PULLUP);

  for (auto& canalBomba : canaisBombasNutrientes) {
    pinMode(canalBomba.bombaPino, OUTPUT);
  }
  desligarTodasBombasNutrientes();

  atualizarDadosLcd();

  Garden::initialize();

  Serial.println("[INFO] Sistema hidroponico iniciado.");
}

void loop() {
  const uint32_t sistemaMomentoAtualMs = millis();

  atualizarBombasNutrientes(sistemaMomentoAtualMs);

  if (ds18b20ConversaoPendente && (sistemaMomentoAtualMs - ds18b20UltimoPedidoMs >= DS18B20_TEMPO_CONVERSAO_MS)) {
    leituraDs18b20TemperaturaC = lerDs18b20TemperaturaC();
    ds18b20ConversaoPendente = false;
  }

  if (sistemaMomentoAtualMs - sistemaUltimoCicloSensoresMs >= SISTEMA_INTERVALO_LEITURA_MS) {
    sistemaUltimoCicloSensoresMs = sistemaMomentoAtualMs;

    if (!ds18b20ConversaoPendente) {
      sensorDs18b20.requestTemperatures();
      ds18b20UltimoPedidoMs = sistemaMomentoAtualMs;
      ds18b20ConversaoPendente = true;
    }

    leituraPhValor = lerPhValor();
    leituraTdsValorPpm = lerTdsValorPpm(leituraDs18b20TemperaturaC);
    leituraTurbidezTensaoMv = lerTurbidezTensaoMv();
    leituraDht22HumidadePercent = lerDht22HumidadePercent();
    leituraCo2ValorPpm = lerCo2Ppm();

    leituraTanque1NivelCm = lerTanquePrincipalNivelCm(Wire);
    leituraTanque2NivelCm = lerTanquePrincipalNivelCm(barramentoI2cTanque2);
    leituraTanque1Percent = converterTanqueNivelParaPercent(leituraTanque1NivelCm, TANQUE1_ALTURA_TOTAL_CM);
    leituraTanque2Percent = converterTanqueNivelParaPercent(leituraTanque2NivelCm, TANQUE2_ALTURA_TOTAL_CM);
    leituraReposicaoEstado = lerReposicaoEstado();

    atualizarDosagemAutomatica(sistemaMomentoAtualMs);
    atualizarDadosLcd();
    imprimirEstadoSerial();
  }

  Garden::update();
}

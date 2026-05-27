#include "services/blynk_service.h"

#include "config.h"
#include "hydro_control.h"

#include <BlynkSimpleEsp32.h>
#include <WiFi.h>

#include <cmath>
#include <cstring>

namespace {

constexpr uint8_t PINO_VIRTUAL_PH = V0;
constexpr uint8_t PINO_VIRTUAL_TDS = V1;
constexpr uint8_t PINO_VIRTUAL_TEMP_AGUA = V2;
constexpr uint8_t PINO_VIRTUAL_TEMP_AR = V3;
constexpr uint8_t PINO_VIRTUAL_HUMIDADE = V4;
constexpr uint8_t PINO_VIRTUAL_NIVEL_AGUA = V5;

constexpr uint8_t PINO_VIRTUAL_BOMBA = V10;
constexpr uint8_t PINO_VIRTUAL_LUZ = V11;
constexpr uint8_t PINO_VIRTUAL_AUTO = V12;
constexpr uint8_t PINO_VIRTUAL_DOSE = V13;
constexpr uint8_t PINO_VIRTUAL_ENABLE = V14;

constexpr uint8_t PINO_VIRTUAL_ESTADO = V20;
constexpr uint8_t PINO_VIRTUAL_ULTIMO_UPDATE = V21;
constexpr uint8_t PINO_VIRTUAL_CONEXAO = V22;

uint32_t momentoUltimaPublicacaoMs = 0;
uint32_t momentoUltimaTentativaWifiMs = 0;
uint32_t momentoUltimaTentativaBlynkMs = 0;
bool blynkConfigurado = false;
wl_status_t ultimoEstadoWifi = WL_IDLE_STATUS;
bool ultimoEstadoBlynkLigado = false;

bool textoConfigurado(const char* valor, const char* placeholder) {
  return valor != nullptr && valor[0] != '\0' && strcmp(valor, placeholder) != 0;
}

bool wifiConfigurado() {
  return textoConfigurado(HidroVidaConfig::WIFI_SSID, "PASTE_WIFI_SSID_HERE") &&
         textoConfigurado(HidroVidaConfig::WIFI_PASSWORD, "PASTE_WIFI_PASSWORD_HERE");
}

bool blynkConfiguradoComToken() {
  return textoConfigurado(BLYNK_AUTH_TOKEN, "PASTE_TOKEN_HERE");
}

const char* textoEstadoConexao() {
  if (!wifiConfigurado() || !blynkConfiguradoComToken()) {
    return "CONFIG";
  }

  if (WiFi.status() != WL_CONNECTED) {
    return "WIFI OFF";
  }

  return Blynk.connected() ? "ONLINE" : "BLYNK OFF";
}

void publicarFloatSeguro(uint8_t pinoVirtual, float valor) {
  if (!std::isfinite(valor)) {
    return;
  }

  Blynk.virtualWrite(pinoVirtual, valor);
}

float arredondarCasas(float valor, uint8_t casas) {
  if (!std::isfinite(valor)) {
    return valor;
  }

  float fator = 1.0f;
  for (uint8_t indice = 0; indice < casas; indice++) {
    fator *= 10.0f;
  }

  return roundf(valor * fator) / fator;
}

void iniciarWifiSeNecessario() {
  if (!wifiConfigurado()) {
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  const uint32_t momentoAtualMs = millis();
  if (momentoAtualMs - momentoUltimaTentativaWifiMs < HidroVidaConfig::WIFI_RECONNECT_MS) {
    return;
  }

  momentoUltimaTentativaWifiMs = momentoAtualMs;
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(HidroVidaConfig::WIFI_SSID, HidroVidaConfig::WIFI_PASSWORD);
}

void iniciarBlynkSeNecessario() {
  if (!blynkConfiguradoComToken() || WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!blynkConfigurado) {
    Blynk.config(BLYNK_AUTH_TOKEN);
    blynkConfigurado = true;
  }

  if (Blynk.connected()) {
    return;
  }

  const uint32_t momentoAtualMs = millis();
  if (momentoAtualMs - momentoUltimaTentativaBlynkMs < HidroVidaConfig::BLYNK_RECONNECT_MS) {
    return;
  }

  momentoUltimaTentativaBlynkMs = momentoAtualMs;
  Blynk.connect(200);
}

void publicarEstadoControlo() {
  Blynk.virtualWrite(PINO_VIRTUAL_BOMBA, hydroObterEstadoReleCirculacao() ? 1 : 0);
  Blynk.virtualWrite(PINO_VIRTUAL_LUZ, hydroObterEstadoReleLuz() ? 1 : 0);
  Blynk.virtualWrite(PINO_VIRTUAL_AUTO, hydroObterAutoDoseAtivo() ? 1 : 0);
  Blynk.virtualWrite(PINO_VIRTUAL_ENABLE, hydroObterSistemaAtivo() ? 1 : 0);
}

void publicarEstadoSistema(const HydroTelemetria& telemetria) {
  Blynk.virtualWrite(PINO_VIRTUAL_ESTADO, telemetria.telemetriaEstadoSistema);
  Blynk.virtualWrite(PINO_VIRTUAL_ULTIMO_UPDATE, telemetria.telemetriaHoraTexto);
  Blynk.virtualWrite(PINO_VIRTUAL_CONEXAO, textoEstadoConexao());
}

void publicarTelemetriaAtual() {
  HydroTelemetria telemetria = {};
  hydroObterTelemetria(telemetria);

  publicarFloatSeguro(PINO_VIRTUAL_PH, arredondarCasas(telemetria.telemetriaPh, 2));
  publicarFloatSeguro(PINO_VIRTUAL_TDS, arredondarCasas(telemetria.telemetriaTdsPpm, 0));
  publicarFloatSeguro(PINO_VIRTUAL_TEMP_AGUA, arredondarCasas(telemetria.telemetriaTemperaturaAguaC, 1));
  publicarFloatSeguro(PINO_VIRTUAL_TEMP_AR, arredondarCasas(telemetria.telemetriaTemperaturaArC, 1));
  publicarFloatSeguro(PINO_VIRTUAL_HUMIDADE, arredondarCasas(telemetria.telemetriaHumidadePercent, 0));
  publicarFloatSeguro(PINO_VIRTUAL_NIVEL_AGUA, arredondarCasas(telemetria.telemetriaNivelAguaPercent, 0));

  publicarEstadoControlo();
  publicarEstadoSistema(telemetria);

  Serial.printf("[BLYNK TX] pH=%.2f TDS=%.0f Agua=%.1fC Ar=%.1fC Hum=%.0f Nivel=%.0f\n",
                telemetria.telemetriaPh,
                telemetria.telemetriaTdsPpm,
                telemetria.telemetriaTemperaturaAguaC,
                telemetria.telemetriaTemperaturaArC,
                telemetria.telemetriaHumidadePercent,
                telemetria.telemetriaNivelAguaPercent);
}

void registarMudancasLigacao() {
  const wl_status_t estadoWifiAtual = WiFi.status();
  if (estadoWifiAtual != ultimoEstadoWifi) {
    ultimoEstadoWifi = estadoWifiAtual;
    Serial.printf("[BLYNK] WiFi status -> %d\n", static_cast<int>(estadoWifiAtual));
  }

  const bool blynkLigado = Blynk.connected();
  if (blynkLigado != ultimoEstadoBlynkLigado) {
    ultimoEstadoBlynkLigado = blynkLigado;
    Serial.printf("[BLYNK] Cloud -> %s\n", blynkLigado ? "ONLINE" : "OFFLINE");
  }
}

}  // namespace

BLYNK_CONNECTED() {
  publicarTelemetriaAtual();
}

BLYNK_WRITE(V10) {
  if (!hydroObterSistemaAtivo()) {
    publicarEstadoControlo();
    return;
  }

  hydroDefinirEstadoReleCirculacao(param.asInt() != 0);
  publicarEstadoControlo();
}

BLYNK_WRITE(V11) {
  if (!hydroObterSistemaAtivo()) {
    publicarEstadoControlo();
    return;
  }

  hydroDefinirEstadoReleLuz(param.asInt() != 0);
  publicarEstadoControlo();
}

BLYNK_WRITE(V12) {
  hydroDefinirAutoDoseAtivo(param.asInt() != 0);
  publicarEstadoControlo();
}

BLYNK_WRITE(V13) {
  if (param.asInt() != 0) {
    hydroExecutarDoseRemota();
  }

  Blynk.virtualWrite(PINO_VIRTUAL_DOSE, 0);
}

BLYNK_WRITE(V14) {
  hydroDefinirSistemaAtivo(param.asInt() != 0);
  publicarEstadoControlo();
}

void initBlynk() {
  WiFi.mode(WIFI_STA);
  ultimoEstadoWifi = WiFi.status();
  ultimoEstadoBlynkLigado = Blynk.connected();
  iniciarWifiSeNecessario();
}

void updateBlynkSensors() {
  if (!Blynk.connected()) {
    return;
  }

  const uint32_t momentoAtualMs = millis();
  if (momentoAtualMs - momentoUltimaPublicacaoMs < HidroVidaConfig::BLYNK_PUBLICACAO_MS) {
    return;
  }

  momentoUltimaPublicacaoMs = momentoAtualMs;
  publicarTelemetriaAtual();
}

void handleBlynkCommands() {
  iniciarWifiSeNecessario();
  iniciarBlynkSeNecessario();

  if (Blynk.connected()) {
    Blynk.run();
  }

  registarMudancasLigacao();
}

bool isBlynkConnected() {
  return Blynk.connected();
}

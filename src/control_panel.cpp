#include "control_panel.h"

#include <cstdio>
#include <cstring>

#include "hardware/pin_map.h"
#include "hydro_control.h"
#include "hydro_lcd_data.h"

namespace {

constexpr uint32_t BOTAO_DEBOUNCE_MS = 60;
constexpr uint32_t LCD_REFRESH_MS = 250;
constexpr uint8_t SAIDAS_VISIVEIS = 3;
constexpr uint8_t SAIDAS_TOTAL = 7;

struct ItemSaida {
  const char* itemNome;
  bool (*itemObterEstado)(void);
  void (*itemDefinirEstado)(bool ligado);
  bool itemVoltar;
};

bool obterEstadoMicro(void) { return hydroObterEstadoBombaNutriente(0); }
bool obterEstadoCalcio(void) { return hydroObterEstadoBombaNutriente(1); }
bool obterEstadoPotassio(void) { return hydroObterEstadoBombaNutriente(2); }
bool obterEstadoMagnesio(void) { return hydroObterEstadoBombaNutriente(3); }
bool obterEstadoCirculacao(void) { return hydroObterEstadoReleCirculacao(); }
bool obterEstadoLuz(void) { return hydroObterEstadoReleLuz(); }

void definirEstadoMicro(bool ligado) { hydroDefinirEstadoBombaNutriente(0, ligado); }
void definirEstadoCalcio(bool ligado) { hydroDefinirEstadoBombaNutriente(1, ligado); }
void definirEstadoPotassio(bool ligado) { hydroDefinirEstadoBombaNutriente(2, ligado); }
void definirEstadoMagnesio(bool ligado) { hydroDefinirEstadoBombaNutriente(3, ligado); }
void definirEstadoCirculacao(bool ligado) { hydroDefinirEstadoReleCirculacao(ligado); }
void definirEstadoLuz(bool ligado) { hydroDefinirEstadoReleLuz(ligado); }

const ItemSaida ITENS_SAIDA[SAIDAS_TOTAL] = {
    {"Micro", obterEstadoMicro, definirEstadoMicro, false},
    {"Calcio", obterEstadoCalcio, definirEstadoCalcio, false},
    {"Potassio", obterEstadoPotassio, definirEstadoPotassio, false},
    {"Magnesio", obterEstadoMagnesio, definirEstadoMagnesio, false},
    {"Circulacao", obterEstadoCirculacao, definirEstadoCirculacao, false},
    {"Luz", obterEstadoLuz, definirEstadoLuz, false},
    {"< Voltar", nullptr, nullptr, true},
};

const char* textoEstado(bool ligado) {
  return ligado ? "ON" : "OFF";
}

}  // namespace

ControlPanel::ControlPanel(LiquidCrystal_I2C* lcdDisplay) : lcd(lcdDisplay) {
}

void ControlPanel::initialize(void) {
  configurarBotao(botaoEsquerda, HardwarePinos::PinoBotaoEsquerda);
  configurarBotao(botaoDireita, HardwarePinos::PinoBotaoDireita);
  configurarBotao(botaoSelecionar, HardwarePinos::PinoBotaoAceitar);
  limparCacheLinhas();
  forceRefresh();
}

void ControlPanel::forceRefresh(void) {
  refreshForcado = true;
}

void ControlPanel::configurarBotao(EstadoBotao& botao, uint8_t pinoBotao) {
  botao.botaoPino = pinoBotao;
  pinMode(pinoBotao, INPUT_PULLUP);
  botao.botaoLeituraAtual = false;
  botao.botaoLeituraAnterior = false;
  botao.botaoUltimaMudancaMs = 0;
  botao.botaoEventoPressionado = false;
}

void ControlPanel::atualizarBotao(EstadoBotao& botao) {
  const bool leituraAtualPressionado = digitalRead(botao.botaoPino) == LOW;
  const uint32_t momentoAtualMs = millis();

  if (leituraAtualPressionado != botao.botaoLeituraAnterior) {
    botao.botaoUltimaMudancaMs = momentoAtualMs;
    botao.botaoLeituraAnterior = leituraAtualPressionado;
  }

  if ((momentoAtualMs - botao.botaoUltimaMudancaMs) >= BOTAO_DEBOUNCE_MS &&
      leituraAtualPressionado != botao.botaoLeituraAtual) {
    botao.botaoLeituraAtual = leituraAtualPressionado;
    if (botao.botaoLeituraAtual) {
      botao.botaoEventoPressionado = true;
    }
  }
}

bool ControlPanel::consumirEventoBotao(EstadoBotao& botao) {
  if (!botao.botaoEventoPressionado) {
    return false;
  }

  botao.botaoEventoPressionado = false;
  return true;
}

void ControlPanel::entrarModo(ModoPainel novoModo) {
  modoAtual = novoModo;
  if (modoAtual == ModoPainel::Saidas) {
    ajustarJanelaSaidas();
  }
  forceRefresh();
}

void ControlPanel::moverMenuPrincipal(int8_t direcao) {
  int itemNovo = static_cast<int>(itemMenuSelecionado) + direcao;
  const int totalItens = static_cast<int>(ItemMenuPrincipal::Total);

  if (itemNovo < 0) {
    itemNovo = totalItens - 1;
  } else if (itemNovo >= totalItens) {
    itemNovo = 0;
  }

  itemMenuSelecionado = static_cast<ItemMenuPrincipal>(itemNovo);
  forceRefresh();
}

void ControlPanel::executarSelecaoMenuPrincipal(void) {
  switch (itemMenuSelecionado) {
    case ItemMenuPrincipal::Dashboard:
      entrarModo(ModoPainel::Dashboard);
      break;
    case ItemMenuPrincipal::Sensores:
      entrarModo(ModoPainel::SensoresPagina1);
      break;
    case ItemMenuPrincipal::Niveis:
      entrarModo(ModoPainel::Niveis);
      break;
    case ItemMenuPrincipal::Saidas:
      entrarModo(ModoPainel::Saidas);
      break;
    default:
      break;
  }
}

void ControlPanel::moverListaSaidas(int8_t direcao) {
  int itemNovo = static_cast<int>(itemSaidaSelecionado) + direcao;

  if (itemNovo < 0) {
    itemNovo = SAIDAS_TOTAL - 1;
  } else if (itemNovo >= SAIDAS_TOTAL) {
    itemNovo = 0;
  }

  itemSaidaSelecionado = static_cast<uint8_t>(itemNovo);
  ajustarJanelaSaidas();
  forceRefresh();
}

void ControlPanel::executarSelecaoSaidas(void) {
  const ItemSaida& itemSaida = ITENS_SAIDA[itemSaidaSelecionado];
  if (itemSaida.itemVoltar) {
    entrarModo(ModoPainel::MenuPrincipal);
    return;
  }

  if (itemSaida.itemObterEstado != nullptr && itemSaida.itemDefinirEstado != nullptr) {
    itemSaida.itemDefinirEstado(!itemSaida.itemObterEstado());
  }

  forceRefresh();
}

void ControlPanel::ajustarJanelaSaidas(void) {
  if (itemSaidaSelecionado < itemSaidaPrimeiroVisivel) {
    itemSaidaPrimeiroVisivel = itemSaidaSelecionado;
  } else if (itemSaidaSelecionado >= itemSaidaPrimeiroVisivel + SAIDAS_VISIVEIS) {
    itemSaidaPrimeiroVisivel = itemSaidaSelecionado - SAIDAS_VISIVEIS + 1;
  }
}

void ControlPanel::limparCacheLinhas(void) {
  for (uint8_t linha = 0; linha < 4; linha++) {
    linhasRenderizadas[linha][0] = '\0';
  }
}

void ControlPanel::escreverLinhaFormatada(uint8_t linha, const char* texto) {
  char linhaNormalizada[21] = {0};
  snprintf(linhaNormalizada, sizeof(linhaNormalizada), "%-20.20s", texto);

  if (strncmp(linhasRenderizadas[linha], linhaNormalizada, 20) == 0) {
    return;
  }

  lcd->setCursor(0, linha);
  lcd->print(linhaNormalizada);
  strncpy(linhasRenderizadas[linha], linhaNormalizada, sizeof(linhasRenderizadas[linha]) - 1);
  linhasRenderizadas[linha][20] = '\0';
}

void ControlPanel::renderizarMenuPrincipal(void) {
  static const char* itensMenu[] = {
      "Dashboard",
      "Sensores",
      "Niveis",
      "Saidas",
  };

  char linhaBuffer[21] = {0};
  for (uint8_t linha = 0; linha < 4; linha++) {
    const bool linhaSelecionada = linha == static_cast<uint8_t>(itemMenuSelecionado);
    snprintf(linhaBuffer, sizeof(linhaBuffer), "%c %s", linhaSelecionada ? '>' : ' ', itensMenu[linha]);
    escreverLinhaFormatada(linha, linhaBuffer);
  }
}

void ControlPanel::renderizarDashboard(void) {
  char linhaBuffer[21] = {0};

  snprintf(linhaBuffer, sizeof(linhaBuffer), "pH %s TDS %s", HydroLcdData::lcdValorPh, HydroLcdData::lcdValorTds);
  escreverLinhaFormatada(0, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Ag %s Tur %s", HydroLcdData::lcdValorTemperaturaAgua, HydroLcdData::lcdValorTurbidez);
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "T1 %s T2 %s", HydroLcdData::lcdValorTanque1, HydroLcdData::lcdValorTanque2);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Ar %s H %s", HydroLcdData::lcdValorTemperaturaAmbiente, HydroLcdData::lcdValorHumidadeAmbiente);
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarSensoresPagina1(void) {
  char linhaBuffer[21] = {0};

  escreverLinhaFormatada(0, "Sensores 1/2");

  snprintf(linhaBuffer, sizeof(linhaBuffer), "pH: %s", HydroLcdData::lcdValorPh);
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "TDS: %s", HydroLcdData::lcdValorTds);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "TDS mV: %s", HydroLcdData::lcdValorTdsTensao);
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarSensoresPagina2(void) {
  char linhaBuffer[21] = {0};

  escreverLinhaFormatada(0, "Sensores 2/2");

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Estado TDS: %s", HydroLcdData::lcdValorTdsEstado);
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Turbidez: %s", HydroLcdData::lcdValorTurbidez);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Agua: %s", HydroLcdData::lcdValorTemperaturaAgua);
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarNiveis(void) {
  char linhaBuffer[21] = {0};

  escreverLinhaFormatada(0, "Niveis");

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Tanque 1: %s", HydroLcdData::lcdValorTanque1);
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Tanque 2: %s", HydroLcdData::lcdValorTanque2);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Reposic.: %s", HydroLcdData::lcdValorReposicao);
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarSaidas(void) {
  char linhaBuffer[21] = {0};
  escreverLinhaFormatada(0, "Saidas");

  for (uint8_t linha = 0; linha < SAIDAS_VISIVEIS; linha++) {
    const uint8_t indiceItem = itemSaidaPrimeiroVisivel + linha;
    if (indiceItem >= SAIDAS_TOTAL) {
      escreverLinhaFormatada(linha + 1, "");
      continue;
    }

    const ItemSaida& itemSaida = ITENS_SAIDA[indiceItem];
    const bool itemSelecionado = indiceItem == itemSaidaSelecionado;

    if (itemSaida.itemVoltar) {
      snprintf(linhaBuffer, sizeof(linhaBuffer), "%c %s", itemSelecionado ? '>' : ' ', itemSaida.itemNome);
    } else {
      snprintf(linhaBuffer,
               sizeof(linhaBuffer),
               "%c %-10s %s",
               itemSelecionado ? '>' : ' ',
               itemSaida.itemNome,
               textoEstado(itemSaida.itemObterEstado()));
    }

    escreverLinhaFormatada(linha + 1, linhaBuffer);
  }
}

void ControlPanel::renderizar(void) {
  switch (modoAtual) {
    case ModoPainel::MenuPrincipal:
      renderizarMenuPrincipal();
      break;
    case ModoPainel::Dashboard:
      renderizarDashboard();
      break;
    case ModoPainel::SensoresPagina1:
      renderizarSensoresPagina1();
      break;
    case ModoPainel::SensoresPagina2:
      renderizarSensoresPagina2();
      break;
    case ModoPainel::Niveis:
      renderizarNiveis();
      break;
    case ModoPainel::Saidas:
      renderizarSaidas();
      break;
    default:
      break;
  }
}

void ControlPanel::update(void) {
  atualizarBotao(botaoEsquerda);
  atualizarBotao(botaoDireita);
  atualizarBotao(botaoSelecionar);

  const bool eventoEsquerda = consumirEventoBotao(botaoEsquerda);
  const bool eventoDireita = consumirEventoBotao(botaoDireita);
  const bool eventoSelecionar = consumirEventoBotao(botaoSelecionar);

  switch (modoAtual) {
    case ModoPainel::MenuPrincipal:
      if (eventoEsquerda) {
        moverMenuPrincipal(-1);
      }
      if (eventoDireita) {
        moverMenuPrincipal(1);
      }
      if (eventoSelecionar) {
        executarSelecaoMenuPrincipal();
      }
      break;

    case ModoPainel::Dashboard:
      if (eventoEsquerda || eventoSelecionar) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        entrarModo(ModoPainel::SensoresPagina1);
      }
      break;

    case ModoPainel::SensoresPagina1:
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita || eventoSelecionar) {
        entrarModo(ModoPainel::SensoresPagina2);
      }
      break;

    case ModoPainel::SensoresPagina2:
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita || eventoSelecionar) {
        entrarModo(ModoPainel::SensoresPagina1);
      }
      break;

    case ModoPainel::Niveis:
      if (eventoEsquerda || eventoSelecionar) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      break;

    case ModoPainel::Saidas:
      if (eventoEsquerda) {
        moverListaSaidas(-1);
      }
      if (eventoDireita) {
        moverListaSaidas(1);
      }
      if (eventoSelecionar) {
        executarSelecaoSaidas();
      }
      break;

    default:
      break;
  }

  const uint32_t momentoAtualMs = millis();
  if (refreshForcado || (momentoAtualMs - ultimoRefreshMs >= LCD_REFRESH_MS)) {
    ultimoRefreshMs = momentoAtualMs;
    refreshForcado = false;
    renderizar();
  }
}

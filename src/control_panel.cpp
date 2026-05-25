#include "control_panel.h"

#include <cstdio>
#include <cstring>

#include "hardware/pin_map.h"
#include "hydro_control.h"
#include "hydro_lcd_data.h"

namespace {

constexpr uint32_t BOTAO_DEBOUNCE_MS = 60;
constexpr uint32_t LCD_REFRESH_MS = 300;
constexpr uint8_t LINHAS_LCD = 4;
constexpr uint8_t ITENS_LISTA_VISIVEIS = 4;
constexpr uint8_t ITENS_SAIDA_VISIVEIS = 3;

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

const ItemSaida ITENS_SAIDA[] = {
    {"Micro", obterEstadoMicro, definirEstadoMicro, false},
    {"Calcio", obterEstadoCalcio, definirEstadoCalcio, false},
    {"Potassio", obterEstadoPotassio, definirEstadoPotassio, false},
    {"Magnesio", obterEstadoMagnesio, definirEstadoMagnesio, false},
    {"Circulacao", obterEstadoCirculacao, definirEstadoCirculacao, false},
    {"Luz", obterEstadoLuz, definirEstadoLuz, false},
    {"< Voltar", nullptr, nullptr, true},
};

constexpr uint8_t ITENS_SAIDA_TOTAL = sizeof(ITENS_SAIDA) / sizeof(ITENS_SAIDA[0]);

const char* ITENS_MENU_PRINCIPAL[] = {
    "Dashboard",
    "Sensores",
    "Niveis",
    "Saidas",
    "Auto Dose",
    "Horario Luz",
    "Horario Circ.",
    "Relogio",
};

constexpr uint8_t ITENS_MENU_TOTAL = sizeof(ITENS_MENU_PRINCIPAL) / sizeof(ITENS_MENU_PRINCIPAL[0]);

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
  forceRefresh();
}

void ControlPanel::moverMenuPrincipal(int8_t direcao) {
  int indiceNovo = static_cast<int>(itemMenuSelecionado) + direcao;
  if (indiceNovo < 0) {
    indiceNovo = ITENS_MENU_TOTAL - 1;
  } else if (indiceNovo >= ITENS_MENU_TOTAL) {
    indiceNovo = 0;
  }

  itemMenuSelecionado = static_cast<ItemMenuPrincipal>(indiceNovo);

  if (indiceNovo < itemMenuPrimeiroVisivel) {
    itemMenuPrimeiroVisivel = static_cast<uint8_t>(indiceNovo);
  } else if (indiceNovo >= itemMenuPrimeiroVisivel + ITENS_LISTA_VISIVEIS) {
    itemMenuPrimeiroVisivel = static_cast<uint8_t>(indiceNovo - ITENS_LISTA_VISIVEIS + 1);
  }

  forceRefresh();
}

void ControlPanel::executarMenuPrincipal(void) {
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
      itemSaidaSelecionado = 0;
      itemSaidaPrimeiroVisivel = 0;
      entrarModo(ModoPainel::Saidas);
      break;
    case ItemMenuPrincipal::AutoDose:
      itemAutoDoseSelecionado = ItemAutoDose::Ativo;
      entrarModo(ModoPainel::AutoDose);
      break;
    case ItemMenuPrincipal::HorarioLuz:
      itemHorarioSelecionado = 0;
      entrarModo(ModoPainel::HorarioLuz);
      break;
    case ItemMenuPrincipal::HorarioCirculacao:
      itemHorarioSelecionado = 0;
      entrarModo(ModoPainel::HorarioCirculacao);
      break;
    case ItemMenuPrincipal::Relogio:
      itemHorarioSelecionado = 0;
      entrarModo(ModoPainel::Relogio);
      break;
    default:
      break;
  }
}

void ControlPanel::moverListaSaidas(int8_t direcao) {
  int indiceNovo = static_cast<int>(itemSaidaSelecionado) + direcao;
  if (indiceNovo < 0) {
    indiceNovo = ITENS_SAIDA_TOTAL - 1;
  } else if (indiceNovo >= ITENS_SAIDA_TOTAL) {
    indiceNovo = 0;
  }

  itemSaidaSelecionado = static_cast<uint8_t>(indiceNovo);
  ajustarJanelaSaidas();
  forceRefresh();
}

void ControlPanel::executarListaSaidas(void) {
  const ItemSaida& itemSaida = ITENS_SAIDA[itemSaidaSelecionado];
  if (itemSaida.itemVoltar) {
    entrarModo(ModoPainel::MenuPrincipal);
    return;
  }

  itemSaida.itemDefinirEstado(!itemSaida.itemObterEstado());
  forceRefresh();
}

void ControlPanel::ajustarJanelaSaidas(void) {
  if (itemSaidaSelecionado < itemSaidaPrimeiroVisivel) {
    itemSaidaPrimeiroVisivel = itemSaidaSelecionado;
  } else if (itemSaidaSelecionado >= itemSaidaPrimeiroVisivel + ITENS_SAIDA_VISIVEIS) {
    itemSaidaPrimeiroVisivel = itemSaidaSelecionado - ITENS_SAIDA_VISIVEIS + 1;
  }
}

void ControlPanel::moverListaHorario(int8_t direcao) {
  int indiceNovo = static_cast<int>(itemHorarioSelecionado) + direcao;
  if (indiceNovo < 0) {
    indiceNovo = 2;
  } else if (indiceNovo > 2) {
    indiceNovo = 0;
  }

  itemHorarioSelecionado = static_cast<uint8_t>(indiceNovo);
  forceRefresh();
}

void ControlPanel::moverListaAutoDose(int8_t direcao) {
  if (direcao == 0) {
    return;
  }

  itemAutoDoseSelecionado =
      itemAutoDoseSelecionado == ItemAutoDose::Ativo ? ItemAutoDose::Voltar : ItemAutoDose::Ativo;
  forceRefresh();
}

void ControlPanel::executarListaAutoDose(void) {
  if (itemAutoDoseSelecionado == ItemAutoDose::Ativo) {
    hydroDefinirAutoDoseAtivo(!hydroObterAutoDoseAtivo());
  } else {
    entrarModo(ModoPainel::MenuPrincipal);
    return;
  }

  forceRefresh();
}

void ControlPanel::executarListaHorario(void) {
  if (modoAtual == ModoPainel::Relogio) {
    uint8_t hora = 0;
    uint8_t minuto = 0;
    uint8_t segundo = 0;
    hydroObterHoraAtual(hora, minuto, segundo);
    prepararEdicaoHora(ContextoEdicaoHora::RelogioSistema, ModoPainel::Relogio, hora, minuto);
    return;
  }

  HydroHorarioSimples horarioAtual =
      (modoAtual == ModoPainel::HorarioLuz) ? hydroObterHorarioLuz() : hydroObterHorarioCirculacao();

  if (itemHorarioSelecionado == 0) {
    horarioAtual.horarioAtivo = !horarioAtual.horarioAtivo;
  } else if (itemHorarioSelecionado == 1) {
    prepararEdicaoHora(
        modoAtual == ModoPainel::HorarioLuz ? ContextoEdicaoHora::HorarioLuzInicio : ContextoEdicaoHora::HorarioCirculacaoInicio,
        modoAtual,
        horarioAtual.horarioHoraInicio,
        horarioAtual.horarioMinutoInicio);
    return;
  } else if (itemHorarioSelecionado == 2) {
    prepararEdicaoHora(
        modoAtual == ModoPainel::HorarioLuz ? ContextoEdicaoHora::HorarioLuzFim : ContextoEdicaoHora::HorarioCirculacaoFim,
        modoAtual,
        horarioAtual.horarioHoraFim,
        horarioAtual.horarioMinutoFim);
    return;
  }

  if (modoAtual == ModoPainel::HorarioLuz) {
    hydroDefinirHorarioLuz(horarioAtual);
  } else {
    hydroDefinirHorarioCirculacao(horarioAtual);
  }

  forceRefresh();
}

void ControlPanel::prepararEdicaoHora(ContextoEdicaoHora contexto, ModoPainel modoRetorno, uint8_t hora, uint8_t minuto) {
  contextoEdicaoHora = contexto;
  modoRetornoEdicao = modoRetorno;
  valorEdicaoHora = hora % 24;
  valorEdicaoMinuto = minuto % 60;
  campoEdicaoSelecionado = 0;
  entrarModo(ModoPainel::EditarHora);
}

void ControlPanel::atualizarEdicaoHora(int8_t direcao) {
  if (campoEdicaoSelecionado == 0) {
    int horaNova = static_cast<int>(valorEdicaoHora) + direcao;
    if (horaNova < 0) {
      horaNova = 23;
    } else if (horaNova > 23) {
      horaNova = 0;
    }
    valorEdicaoHora = static_cast<uint8_t>(horaNova);
  } else {
    int minutoNovo = static_cast<int>(valorEdicaoMinuto) + direcao;
    if (minutoNovo < 0) {
      minutoNovo = 59;
    } else if (minutoNovo > 59) {
      minutoNovo = 0;
    }
    valorEdicaoMinuto = static_cast<uint8_t>(minutoNovo);
  }

  forceRefresh();
}

void ControlPanel::confirmarEdicaoHora(void) {
  if (campoEdicaoSelecionado == 0) {
    campoEdicaoSelecionado = 1;
    forceRefresh();
    return;
  }

  switch (contextoEdicaoHora) {
    case ContextoEdicaoHora::RelogioSistema:
      hydroDefinirHoraAtual(valorEdicaoHora, valorEdicaoMinuto, 0);
      break;

    case ContextoEdicaoHora::HorarioLuzInicio: {
      HydroHorarioSimples horario = hydroObterHorarioLuz();
      horario.horarioHoraInicio = valorEdicaoHora;
      horario.horarioMinutoInicio = valorEdicaoMinuto;
      hydroDefinirHorarioLuz(horario);
      break;
    }

    case ContextoEdicaoHora::HorarioLuzFim: {
      HydroHorarioSimples horario = hydroObterHorarioLuz();
      horario.horarioHoraFim = valorEdicaoHora;
      horario.horarioMinutoFim = valorEdicaoMinuto;
      hydroDefinirHorarioLuz(horario);
      break;
    }

    case ContextoEdicaoHora::HorarioCirculacaoInicio: {
      HydroHorarioSimples horario = hydroObterHorarioCirculacao();
      horario.horarioHoraInicio = valorEdicaoHora;
      horario.horarioMinutoInicio = valorEdicaoMinuto;
      hydroDefinirHorarioCirculacao(horario);
      break;
    }

    case ContextoEdicaoHora::HorarioCirculacaoFim: {
      HydroHorarioSimples horario = hydroObterHorarioCirculacao();
      horario.horarioHoraFim = valorEdicaoHora;
      horario.horarioMinutoFim = valorEdicaoMinuto;
      hydroDefinirHorarioCirculacao(horario);
      break;
    }

    default:
      break;
  }

  contextoEdicaoHora = ContextoEdicaoHora::Nenhum;
  campoEdicaoSelecionado = 0;
  entrarModo(modoRetornoEdicao);
}

void ControlPanel::limparCacheLinhas(void) {
  for (uint8_t linha = 0; linha < LINHAS_LCD; linha++) {
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
  char linhaBuffer[21] = {0};

  for (uint8_t linha = 0; linha < LINHAS_LCD; linha++) {
    const uint8_t indiceItem = itemMenuPrimeiroVisivel + linha;
    if (indiceItem >= ITENS_MENU_TOTAL) {
      escreverLinhaFormatada(linha, "");
      continue;
    }

    snprintf(linhaBuffer,
             sizeof(linhaBuffer),
             "%c %s",
             indiceItem == static_cast<uint8_t>(itemMenuSelecionado) ? '>' : ' ',
             ITENS_MENU_PRINCIPAL[indiceItem]);
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

  for (uint8_t linha = 0; linha < ITENS_SAIDA_VISIVEIS; linha++) {
    const uint8_t indiceItem = itemSaidaPrimeiroVisivel + linha;
    if (indiceItem >= ITENS_SAIDA_TOTAL) {
      escreverLinhaFormatada(linha + 1, "");
      continue;
    }

    const ItemSaida& itemSaida = ITENS_SAIDA[indiceItem];
    if (itemSaida.itemVoltar) {
      snprintf(linhaBuffer, sizeof(linhaBuffer), "%c %s", indiceItem == itemSaidaSelecionado ? '>' : ' ', itemSaida.itemNome);
    } else {
      snprintf(linhaBuffer,
               sizeof(linhaBuffer),
               "%c %-10s %s",
               indiceItem == itemSaidaSelecionado ? '>' : ' ',
               itemSaida.itemNome,
               textoEstado(itemSaida.itemObterEstado()));
    }

    escreverLinhaFormatada(linha + 1, linhaBuffer);
  }
}

void ControlPanel::renderizarAutoDose(void) {
  char linhaBuffer[21] = {0};
  const HydroAutoDoseEstado estado = hydroObterAutoDoseEstado();

  escreverLinhaFormatada(0, "Auto Dose");
  snprintf(linhaBuffer,
           sizeof(linhaBuffer),
           "%c Ativo: %s",
           itemAutoDoseSelecionado == ItemAutoDose::Ativo ? '>' : ' ',
           estado.autoDoseAtivo ? "ON" : "OFF");
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Estado: %s", estado.autoDoseEstadoTexto);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer,
           sizeof(linhaBuffer),
           "%c < Voltar",
           itemAutoDoseSelecionado == ItemAutoDose::Voltar ? '>' : ' ');
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarHorario(
    const char* titulo,
    bool ativo,
    uint8_t horaInicio,
    uint8_t minutoInicio,
    uint8_t horaFim,
    uint8_t minutoFim) {
  char linhaBuffer[21] = {0};

  escreverLinhaFormatada(0, titulo);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "%c Ativo: %s", itemHorarioSelecionado == 0 ? '>' : ' ', textoEstado(ativo));
  escreverLinhaFormatada(1, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "%c Inicio %02u:%02u", itemHorarioSelecionado == 1 ? '>' : ' ', horaInicio, minutoInicio);
  escreverLinhaFormatada(2, linhaBuffer);

  snprintf(linhaBuffer, sizeof(linhaBuffer), "%c Fim %02u:%02u", itemHorarioSelecionado == 2 ? '>' : ' ', horaFim, minutoFim);
  escreverLinhaFormatada(3, linhaBuffer);
}

void ControlPanel::renderizarRelogio(void) {
  char linhaBuffer[21] = {0};
  uint8_t hora = 0;
  uint8_t minuto = 0;
  uint8_t segundo = 0;
  hydroObterHoraAtual(hora, minuto, segundo);

  escreverLinhaFormatada(0, "Relogio");

  snprintf(linhaBuffer, sizeof(linhaBuffer), "Hora %02u:%02u:%02u", hora, minuto, segundo);
  escreverLinhaFormatada(1, linhaBuffer);
  escreverLinhaFormatada(2, "SEL edita hora");
  escreverLinhaFormatada(3, "ESQ menu");
}

void ControlPanel::renderizarEdicaoHora(void) {
  char linhaBuffer[21] = {0};
  const char* titulo = "Editar hora";

  switch (contextoEdicaoHora) {
    case ContextoEdicaoHora::RelogioSistema:
      titulo = "Editar relogio";
      break;
    case ContextoEdicaoHora::HorarioLuzInicio:
      titulo = "Luz inicio";
      break;
    case ContextoEdicaoHora::HorarioLuzFim:
      titulo = "Luz fim";
      break;
    case ContextoEdicaoHora::HorarioCirculacaoInicio:
      titulo = "Circ inicio";
      break;
    case ContextoEdicaoHora::HorarioCirculacaoFim:
      titulo = "Circ fim";
      break;
    default:
      break;
  }

  escreverLinhaFormatada(0, titulo);
  snprintf(linhaBuffer, sizeof(linhaBuffer), "%c Hora: %02u", campoEdicaoSelecionado == 0 ? '>' : ' ', valorEdicaoHora);
  escreverLinhaFormatada(1, linhaBuffer);
  snprintf(linhaBuffer, sizeof(linhaBuffer), "%c Min: %02u", campoEdicaoSelecionado == 1 ? '>' : ' ', valorEdicaoMinuto);
  escreverLinhaFormatada(2, linhaBuffer);
  escreverLinhaFormatada(3, "SEL guarda/prox");
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
    case ModoPainel::AutoDose:
      renderizarAutoDose();
      break;
    case ModoPainel::HorarioLuz: {
      const HydroHorarioSimples horario = hydroObterHorarioLuz();
      renderizarHorario("Horario luz", horario.horarioAtivo, horario.horarioHoraInicio, horario.horarioMinutoInicio, horario.horarioHoraFim, horario.horarioMinutoFim);
      break;
    }
    case ModoPainel::HorarioCirculacao: {
      const HydroHorarioSimples horario = hydroObterHorarioCirculacao();
      renderizarHorario("Horario circ.", horario.horarioAtivo, horario.horarioHoraInicio, horario.horarioMinutoInicio, horario.horarioHoraFim, horario.horarioMinutoFim);
      break;
    }
    case ModoPainel::Relogio:
      renderizarRelogio();
      break;
    case ModoPainel::EditarHora:
      renderizarEdicaoHora();
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
        executarMenuPrincipal();
      }
      break;

    case ModoPainel::Dashboard:
      if (eventoSelecionar) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        entrarModo(ModoPainel::SensoresPagina1);
      }
      break;

    case ModoPainel::SensoresPagina1:
      if (eventoSelecionar) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        entrarModo(ModoPainel::SensoresPagina2);
      }
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      break;

    case ModoPainel::SensoresPagina2:
      if (eventoSelecionar) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        entrarModo(ModoPainel::SensoresPagina1);
      }
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      break;

    case ModoPainel::Niveis:
      if (eventoSelecionar || eventoEsquerda) {
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
        executarListaSaidas();
      }
      break;

    case ModoPainel::AutoDose:
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        moverListaAutoDose(1);
      }
      if (eventoSelecionar) {
        executarListaAutoDose();
      }
      break;

    case ModoPainel::HorarioLuz:
    case ModoPainel::HorarioCirculacao:
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoDireita) {
        moverListaHorario(1);
      }
      if (eventoSelecionar) {
        executarListaHorario();
      }
      break;

    case ModoPainel::Relogio:
      if (eventoEsquerda) {
        entrarModo(ModoPainel::MenuPrincipal);
      }
      if (eventoSelecionar) {
        executarListaHorario();
      }
      break;

    case ModoPainel::EditarHora:
      if (eventoEsquerda) {
        atualizarEdicaoHora(-1);
      }
      if (eventoDireita) {
        atualizarEdicaoHora(1);
      }
      if (eventoSelecionar) {
        confirmarEdicaoHora();
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

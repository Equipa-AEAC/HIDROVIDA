#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class ControlPanel {
public:
  explicit ControlPanel(LiquidCrystal_I2C* lcdDisplay);

  void initialize(void);
  void update(void);
  void forceRefresh(void);

private:
  enum class ModoPainel : uint8_t {
    MenuPrincipal = 0,
    Dashboard,
    SensoresPagina1,
    SensoresPagina2,
    Niveis,
    Saidas
  };

  enum class ItemMenuPrincipal : uint8_t {
    Dashboard = 0,
    Sensores,
    Niveis,
    Saidas,
    Total
  };

  struct EstadoBotao {
    uint8_t botaoPino;
    bool botaoLeituraAtual;
    bool botaoLeituraAnterior;
    uint32_t botaoUltimaMudancaMs;
    bool botaoEventoPressionado;
  };

  LiquidCrystal_I2C* lcd = nullptr;
  ModoPainel modoAtual = ModoPainel::MenuPrincipal;
  ItemMenuPrincipal itemMenuSelecionado = ItemMenuPrincipal::Dashboard;
  uint8_t itemSaidaSelecionado = 0;
  uint8_t itemSaidaPrimeiroVisivel = 0;
  EstadoBotao botaoEsquerda = {};
  EstadoBotao botaoDireita = {};
  EstadoBotao botaoSelecionar = {};
  uint32_t ultimoRefreshMs = 0;
  bool refreshForcado = true;
  char linhasRenderizadas[4][21] = {{0}};

  void configurarBotao(EstadoBotao& botao, uint8_t pinoBotao);
  void atualizarBotao(EstadoBotao& botao);
  bool consumirEventoBotao(EstadoBotao& botao);

  void entrarModo(ModoPainel novoModo);
  void moverMenuPrincipal(int8_t direcao);
  void executarSelecaoMenuPrincipal(void);

  void moverListaSaidas(int8_t direcao);
  void executarSelecaoSaidas(void);
  void ajustarJanelaSaidas(void);

  void renderizar(void);
  void renderizarMenuPrincipal(void);
  void renderizarDashboard(void);
  void renderizarSensoresPagina1(void);
  void renderizarSensoresPagina2(void);
  void renderizarNiveis(void);
  void renderizarSaidas(void);

  void escreverLinhaFormatada(uint8_t linha, const char* texto);
  void limparCacheLinhas(void);
};

#endif  // CONTROL_PANEL_H

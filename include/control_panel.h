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
    Saidas,
    AutoDose,
    HorarioLuz,
    HorarioCirculacao,
    Relogio,
    EditarHora
  };

  enum class ItemMenuPrincipal : uint8_t {
    Dashboard = 0,
    Sensores,
    Niveis,
    Saidas,
    AutoDose,
    HorarioLuz,
    HorarioCirculacao,
    Relogio,
    Total
  };

  enum class ContextoEdicaoHora : uint8_t {
    Nenhum = 0,
    RelogioSistema,
    HorarioLuzInicio,
    HorarioLuzFim,
    HorarioCirculacaoInicio,
    HorarioCirculacaoFim
  };

  enum class ItemAutoDose : uint8_t {
    Ativo = 0,
    Voltar
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
  ModoPainel modoRetornoEdicao = ModoPainel::MenuPrincipal;
  ItemMenuPrincipal itemMenuSelecionado = ItemMenuPrincipal::Dashboard;
  uint8_t itemMenuPrimeiroVisivel = 0;
  uint8_t itemSaidaSelecionado = 0;
  uint8_t itemSaidaPrimeiroVisivel = 0;
  uint8_t itemHorarioSelecionado = 0;
  ItemAutoDose itemAutoDoseSelecionado = ItemAutoDose::Ativo;
  uint8_t campoEdicaoSelecionado = 0;
  ContextoEdicaoHora contextoEdicaoHora = ContextoEdicaoHora::Nenhum;
  uint8_t valorEdicaoHora = 0;
  uint8_t valorEdicaoMinuto = 0;
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
  void executarMenuPrincipal(void);

  void moverListaSaidas(int8_t direcao);
  void executarListaSaidas(void);
  void ajustarJanelaSaidas(void);

  void moverListaHorario(int8_t direcao);
  void executarListaHorario(void);

  void moverListaAutoDose(int8_t direcao);
  void executarListaAutoDose(void);

  void prepararEdicaoHora(ContextoEdicaoHora contexto, ModoPainel modoRetorno, uint8_t hora, uint8_t minuto);
  void atualizarEdicaoHora(int8_t direcao);
  void confirmarEdicaoHora(void);

  void renderizar(void);
  void renderizarMenuPrincipal(void);
  void renderizarDashboard(void);
  void renderizarSensoresPagina1(void);
  void renderizarSensoresPagina2(void);
  void renderizarNiveis(void);
  void renderizarSaidas(void);
  void renderizarAutoDose(void);
  void renderizarHorario(const char* titulo, bool ativo, uint8_t horaInicio, uint8_t minutoInicio, uint8_t horaFim, uint8_t minutoFim);
  void renderizarRelogio(void);
  void renderizarEdicaoHora(void);

  void escreverLinhaFormatada(uint8_t linha, const char* texto);
  void limparCacheLinhas(void);
};

#endif  // CONTROL_PANEL_H

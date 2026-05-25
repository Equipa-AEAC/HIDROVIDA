#ifndef HYDRO_CONTROL_H
#define HYDRO_CONTROL_H

#include <Arduino.h>

struct HydroHorarioSimples {
  bool horarioAtivo;
  uint8_t horarioHoraInicio;
  uint8_t horarioMinutoInicio;
  uint8_t horarioHoraFim;
  uint8_t horarioMinutoFim;
};

struct HydroAutoDoseEstado {
  bool autoDoseAtivo;
  uint16_t autoDoseTriggerPpm;
  uint16_t autoDoseTargetPpm;
  char autoDoseEstadoTexto[16];
};

bool hydroObterEstadoBombaNutriente(uint8_t indiceBomba);
void hydroDefinirEstadoBombaNutriente(uint8_t indiceBomba, bool ligada);

bool hydroObterEstadoReleCirculacao(void);
void hydroDefinirEstadoReleCirculacao(bool ligado);

bool hydroObterEstadoReleLuz(void);
void hydroDefinirEstadoReleLuz(bool ligado);

void hydroObterHoraAtual(uint8_t& hora, uint8_t& minuto, uint8_t& segundo);
void hydroDefinirHoraAtual(uint8_t hora, uint8_t minuto, uint8_t segundo);

HydroHorarioSimples hydroObterHorarioLuz(void);
void hydroDefinirHorarioLuz(const HydroHorarioSimples& horario);

HydroHorarioSimples hydroObterHorarioCirculacao(void);
void hydroDefinirHorarioCirculacao(const HydroHorarioSimples& horario);

bool hydroObterAutoDoseAtivo(void);
void hydroDefinirAutoDoseAtivo(bool ativo);
HydroAutoDoseEstado hydroObterAutoDoseEstado(void);

#endif  // HYDRO_CONTROL_H

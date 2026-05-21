#ifndef HYDRO_CONTROL_H
#define HYDRO_CONTROL_H

#include <Arduino.h>

bool hydroObterEstadoBombaNutriente(uint8_t indiceBomba);
void hydroDefinirEstadoBombaNutriente(uint8_t indiceBomba, bool ligada);

bool hydroObterEstadoReleCirculacao(void);
void hydroDefinirEstadoReleCirculacao(bool ligado);

bool hydroObterEstadoReleLuz(void);
void hydroDefinirEstadoReleLuz(bool ligado);

#endif  // HYDRO_CONTROL_H

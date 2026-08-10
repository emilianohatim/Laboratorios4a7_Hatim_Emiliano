/*********************************************************************************************************************
Copyright 2016-2026, Laboratorio de Microprocesadores
Facultad de Ciencias Exactas y Tecnologia
Universidad Nacional de Tucuman
http://www.microprocesadores.unt.edu.ar/

Copyright 2016-2026, Emiliano Hatim <emilianohatim01@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*************************************************************************************************/

/** @file rtos_tareas.c
 ** @brief Implementación de las tareas de freertos
 */
/* === Headers files inclusions ================================================================ */

#include "rtos_tareas.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

/* === Private function declarations =========================================================== */

static void IncrementarMinutos(hora_t hora);
static void DecrementarMinutos(hora_t hora);
static void IncrementarHoras(hora_t hora);
static void DecrementarHoras(hora_t hora);
static void SetPunto(board_t placa, uint8_t digito, bool encendido);

/* === Private variable definitions ============================================================ */

/* === Public variable definition  ============================================================= */

/* === Private function definitions ============================================================ */

static void IncrementarMinutos(hora_t hora) {
    hora[3]++;
    if (hora[3] > 9) {
        hora[3] = 0;
        hora[2]++;
    }
    if (hora[2] > 5) {
        hora[2] = 0;
    }
}

static void DecrementarMinutos(hora_t hora) {
    if (hora[3] == 0) {
        hora[3] = 9;
        if (hora[2] == 0) {
            hora[2] = 5;
        } else {
            hora[2]--;
        }
    } else {
        hora[3]--;
    }
}

static void IncrementarHoras(hora_t hora) {
    hora[1]++;
    if (hora[0] == 2 && hora[1] > 3) {
        hora[0] = 0;
        hora[1] = 0;
    } else if (hora[1] > 9) {
        hora[1] = 0;
        hora[0]++;
    }
}

static void DecrementarHoras(hora_t hora) {
    if (hora[1] == 0) {
        if (hora[0] == 0) {
            hora[0] = 2;
            hora[1] = 3;
        } else {
            hora[1] = 9;
            hora[0]--;
        }
    } else {
        hora[1]--;
    }
}

static void SetPunto(board_t placa, uint8_t digito, bool encendido) {
    static bool estado_puntos_locales[4] = {false, false, false, false};
    if (estado_puntos_locales[digito] != encendido) {
        DisplayToggleDots(placa->display, digito, digito);
        estado_puntos_locales[digito] = encendido;
    }
}

/* === Public function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void TareaDisplay(void * Parametros) {
    board_t placa = (board_t)Parametros;
    const TickType_t xFrecuencia = pdMS_TO_TICKS(1);
    TickType_t xUltimoDespertar = xTaskGetTickCount();
    while (1) {
        DisplayRefresh(placa->display);
        vTaskDelayUntil(&xUltimoDespertar, xFrecuencia);
    }
}

void TareaContarTiempo(void * Parametros) {
    clock_t reloj = (clock_t)Parametros;
    const TickType_t xFrecuencia = pdMS_TO_TICKS(1000);
    TickType_t xUltimoDespertar = xTaskGetTickCount();
    while (1) {
        ClockNewTick(reloj);
        vTaskDelayUntil(&xUltimoDespertar, xFrecuencia);
    }
}

void TareaTeclado(void * Parametros) {
    board_t placa = (board_t)Parametros;
    const TickType_t xFrecuencia = pdMS_TO_TICKS(PERIODO_TECLADO_MS);
    TickType_t xUltimoDespertar = xTaskGetTickCount();
    uint16_t contador_f1 = 0;
    uint16_t contador_f2 = 0;
    while (1) {
        if (DigitalInputGetState(placa->f1)) {
            contador_f1++;
            if (contador_f1 == TICKS_PULSACION_LARGA) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F1_LARGO);
            }
        } else {
            contador_f1 = 0;
        }
        if (DigitalInputGetState(placa->f2)) {
            contador_f2++;
            if (contador_f2 == TICKS_PULSACION_LARGA) {
                xEventGroupSetBits(eventos_teclado, EVENTO_F2_LARGO);
            }
        } else {
            contador_f2 = 0;
        }
        if (DigitalInputHasActivated(placa->f3)) {
            xEventGroupSetBits(eventos_teclado, EVENTO_F3);
        }
        if (DigitalInputHasActivated(placa->f4)) {
            xEventGroupSetBits(eventos_teclado, EVENTO_F4);
        }
        if (DigitalInputHasActivated(placa->aceptar)) {
            xEventGroupSetBits(eventos_teclado, EVENTO_ACEPTAR);
        }
        if (DigitalInputHasActivated(placa->cancelar)) {
            xEventGroupSetBits(eventos_teclado, EVENTO_CANCELAR);
        }
        vTaskDelayUntil(&xUltimoDespertar, xFrecuencia);
    }
}

void ManejadorAlarma(bool estado) {
    if (estado) {
        xEventGroupSetBits(eventos_teclado, EVENTO_ALARMA);
    }
}

void TareaFSM(void * pvParameters) {
    rtos_context_t * contexto = (rtos_context_t *)pvParameters;
    board_t placa = contexto->placa;
    clock_t reloj = contexto->reloj;

    estado_t estado_actual = RELOJ_SIN_CONFIGURAR;
    hora_t hora_temporal = {0};
    uint8_t display_bcd[4] = {0, 0, 0, 0};
    bool titilar_punto = false;

    const TickType_t TIMEOUT_CONFIG = pdMS_TO_TICKS(30000);
    const TickType_t TIMEOUT_TICK_1S = pdMS_TO_TICKS(1000);
    const TickType_t TIMEOUT_ALARMA = pdMS_TO_TICKS(60000);

    TickType_t tiempo_espera = portMAX_DELAY;

    DisplayFlashDigits(placa->display, 0, 3, 250);
    SetPunto(placa, 0, true);
    SetPunto(placa, 1, true);
    SetPunto(placa, 2, true);
    SetPunto(placa, 3, true);

    while (1) {
        EventBits_t eventos = xEventGroupWaitBits(eventos_teclado, 0x7F, pdTRUE, pdFALSE, tiempo_espera);
        if (eventos == 0) {
            if (estado_actual == MOSTRANDO_HORA) {
                hora_t hora_actual;
                if (ClockGetCurrentTime(reloj, hora_actual)) {
                    display_bcd[0] = hora_actual[0];
                    display_bcd[1] = hora_actual[1];
                    display_bcd[2] = hora_actual[2];
                    display_bcd[3] = hora_actual[3];
                    DisplayWriteBCD(placa->display, display_bcd, 4);

                    titilar_punto = !titilar_punto;
                    SetPunto(placa, 1, titilar_punto);
                }
            } else if (estado_actual == ALARMA_SONANDO) {
                DigitalOutputDeactivate(placa->buzzer);
                estado_actual = MOSTRANDO_HORA;
                DisplayFlashDigits(placa->display, 0, 0, 0);
                tiempo_espera = TIMEOUT_TICK_1S;
            } else if (estado_actual != RELOJ_SIN_CONFIGURAR) {
                hora_t hora_actual;
                if (ClockGetCurrentTime(reloj, hora_actual)) {
                    estado_actual = MOSTRANDO_HORA;
                    DisplayFlashDigits(placa->display, 0, 0, 0);
                    SetPunto(placa, 0, false);
                    SetPunto(placa, 1, false);
                    SetPunto(placa, 2, false);
                    if (ClockGetAlarmEnabled(reloj))
                        SetPunto(placa, 3, true);
                    else
                        SetPunto(placa, 3, false);
                    tiempo_espera = TIMEOUT_TICK_1S;
                } else {
                    estado_actual = RELOJ_SIN_CONFIGURAR;
                    DisplayFlashDigits(placa->display, 0, 3, 250);
                    SetPunto(placa, 0, true);
                    SetPunto(placa, 1, true);
                    SetPunto(placa, 2, true);
                    SetPunto(placa, 3, true);
                    tiempo_espera = portMAX_DELAY;
                }
            }
            continue;
        }

        if ((eventos & EVENTO_ALARMA) && estado_actual != RELOJ_SIN_CONFIGURAR) {
            estado_actual = ALARMA_SONANDO;
            DigitalOutputActivate(placa->buzzer);
            DisplayFlashDigits(placa->display, 0, 3, 100);
            tiempo_espera = TIMEOUT_ALARMA;
            continue;
        }

        switch (estado_actual) {

        case RELOJ_SIN_CONFIGURAR:
            if (eventos & EVENTO_F1_LARGO) {
                estado_actual = AJUSTANDO_MINUTOS_RELOJ;
                memset(hora_temporal, 0, sizeof(hora_temporal));
                DisplayFlashDigits(placa->display, 2, 3, 250);
                SetPunto(placa, 0, false);
                SetPunto(placa, 1, false);
                SetPunto(placa, 2, false);
                SetPunto(placa, 3, false);
                tiempo_espera = TIMEOUT_CONFIG;
            }
            break;

        case MOSTRANDO_HORA:
            if (eventos & EVENTO_F1_LARGO) {
                estado_actual = AJUSTANDO_MINUTOS_RELOJ;
                ClockGetCurrentTime(reloj, hora_temporal);
                DisplayFlashDigits(placa->display, 2, 3, 250);
                tiempo_espera = TIMEOUT_CONFIG;
            } else if (eventos & EVENTO_F2_LARGO) {
                estado_actual = AJUSTANDO_MINUTOS_ALARMA;
                ClockGetAlarm(reloj, hora_temporal);
                DisplayFlashDigits(placa->display, 2, 3, 250);
                SetPunto(placa, 0, true);
                SetPunto(placa, 1, true);
                SetPunto(placa, 2, true);
                SetPunto(placa, 3, true);
                tiempo_espera = TIMEOUT_CONFIG;
            } else if (eventos & EVENTO_ACEPTAR) {
                if (!ClockGetAlarmEnabled(reloj))
                    ClockToggleAlarm(reloj);
                SetPunto(placa, 3, true);
            } else if (eventos & EVENTO_CANCELAR) {
                if (ClockGetAlarmEnabled(reloj))
                    ClockToggleAlarm(reloj);
                SetPunto(placa, 3, false);
            }
            break;

        case AJUSTANDO_MINUTOS_RELOJ:
            tiempo_espera = TIMEOUT_CONFIG;
            if (eventos & EVENTO_F3)
                DecrementarMinutos(hora_temporal);
            if (eventos & EVENTO_F4)
                IncrementarMinutos(hora_temporal);
            if (eventos & EVENTO_ACEPTAR) {
                estado_actual = AJUSTANDO_HORAS_RELOJ;
                DisplayFlashDigits(placa->display, 0, 1, 250);
            } else if (eventos & EVENTO_CANCELAR) {
                tiempo_espera = 0;
            }
            display_bcd[0] = hora_temporal[0];
            display_bcd[1] = hora_temporal[1];
            display_bcd[2] = hora_temporal[2];
            display_bcd[3] = hora_temporal[3];
            DisplayWriteBCD(placa->display, display_bcd, 4);
            break;

        case AJUSTANDO_HORAS_RELOJ:
            tiempo_espera = TIMEOUT_CONFIG;
            if (eventos & EVENTO_F3)
                DecrementarHoras(hora_temporal);
            if (eventos & EVENTO_F4)
                IncrementarHoras(hora_temporal);
            if (eventos & EVENTO_ACEPTAR) {
                ClockSetupCurrentTime(reloj, hora_temporal);
                estado_actual = MOSTRANDO_HORA;
                DisplayFlashDigits(placa->display, 0, 0, 0);
                tiempo_espera = TIMEOUT_TICK_1S;
            } else if (eventos & EVENTO_CANCELAR) {
                tiempo_espera = 0;
            }
            display_bcd[0] = hora_temporal[0];
            display_bcd[1] = hora_temporal[1];
            display_bcd[2] = hora_temporal[2];
            display_bcd[3] = hora_temporal[3];
            DisplayWriteBCD(placa->display, display_bcd, 4);
            break;

        case AJUSTANDO_MINUTOS_ALARMA:
            tiempo_espera = TIMEOUT_CONFIG;
            if (eventos & EVENTO_F3)
                DecrementarMinutos(hora_temporal);
            if (eventos & EVENTO_F4)
                IncrementarMinutos(hora_temporal);
            if (eventos & EVENTO_ACEPTAR) {
                estado_actual = AJUSTANDO_HORAS_ALARMA;
                DisplayFlashDigits(placa->display, 0, 1, 250);
            } else if (eventos & EVENTO_CANCELAR) {
                tiempo_espera = 0;
            }
            display_bcd[0] = hora_temporal[0];
            display_bcd[1] = hora_temporal[1];
            display_bcd[2] = hora_temporal[2];
            display_bcd[3] = hora_temporal[3];
            DisplayWriteBCD(placa->display, display_bcd, 4);
            break;

        case AJUSTANDO_HORAS_ALARMA:
            tiempo_espera = TIMEOUT_CONFIG;
            if (eventos & EVENTO_F3)
                DecrementarHoras(hora_temporal);
            if (eventos & EVENTO_F4)
                IncrementarHoras(hora_temporal);
            if (eventos & EVENTO_ACEPTAR) {
                ClockSetupAlarm(reloj, hora_temporal);
                if (!ClockGetAlarmEnabled(reloj)) {
                    ClockToggleAlarm(reloj);
                }
                estado_actual = MOSTRANDO_HORA;
                DisplayFlashDigits(placa->display, 0, 0, 0);
                SetPunto(placa, 0, false);
                SetPunto(placa, 1, false);
                SetPunto(placa, 2, false);
                SetPunto(placa, 3, true);
                tiempo_espera = TIMEOUT_TICK_1S;
            } else if (eventos & EVENTO_CANCELAR) {
                tiempo_espera = 0;
            }
            display_bcd[0] = hora_temporal[0];
            display_bcd[1] = hora_temporal[1];
            display_bcd[2] = hora_temporal[2];
            display_bcd[3] = hora_temporal[3];
            DisplayWriteBCD(placa->display, display_bcd, 4);
            break;

        case ALARMA_SONANDO:
            if (eventos & EVENTO_ACEPTAR) {
                ClockPostponeAlarm(reloj, 5);
                DigitalOutputDeactivate(placa->buzzer);
                estado_actual = MOSTRANDO_HORA;
                DisplayFlashDigits(placa->display, 0, 0, 0);
                tiempo_espera = TIMEOUT_TICK_1S;
            } else if (eventos & EVENTO_CANCELAR) {
                ClockSkipTodayAlarm(reloj);
                DigitalOutputDeactivate(placa->buzzer);
                estado_actual = MOSTRANDO_HORA;
                DisplayFlashDigits(placa->display, 0, 0, 0);
                tiempo_espera = TIMEOUT_TICK_1S;
            }
            break;
        }
    }
}

/* === End of documentation ==================================================================== */

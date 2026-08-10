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

#ifndef RTOS_TAREAS_H_
#define RTOS_TAREAS_H_

/** @file rtos_tareas.h
 ** @brief Declaración de las tareas de freertos
 **/

/* === Headers files inclusions ==================================================================================== */

/* === Header for C++ compatibility ================================================================================ */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions =================================================================================== */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "event_groups.h"
#include "bsp.h"
#include "reloj.h"
#include <string.h>

#define EVENTO_F1_LARGO       (1 << 0)
#define EVENTO_F2_LARGO       (1 << 1)
#define EVENTO_F3             (1 << 2)
#define EVENTO_F4             (1 << 3)
#define EVENTO_ACEPTAR        (1 << 4)
#define EVENTO_CANCELAR       (1 << 5)
#define EVENTO_ALARMA         (1 << 6)

#define PERIODO_TECLADO_MS    50
#define TICKS_PULSACION_LARGA (3000 / PERIODO_TECLADO_MS)

/* === Public data type declarations =============================================================================== */

typedef struct {
    board_t placa;
    clock_t reloj;
} rtos_context_t;

typedef enum {
    RELOJ_SIN_CONFIGURAR,
    MOSTRANDO_HORA,
    AJUSTANDO_MINUTOS_RELOJ,
    AJUSTANDO_HORAS_RELOJ,
    AJUSTANDO_MINUTOS_ALARMA,
    AJUSTANDO_HORAS_ALARMA,
    ALARMA_SONANDO
} estado_t;

/* === Public variable declarations ================================================================================ */

extern EventGroupHandle_t eventos_teclado;

/* === Public function declarations ================================================================================ */

void TareaDisplay(void * Parametros);

void TareaContarTiempo(void * Parametros);

void TareaTeclado(void * Parametros);

void TareaFSM(void * Parametros);

void ManejadorAlarma(bool estado);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* RTOS_TAREAS_H_ */
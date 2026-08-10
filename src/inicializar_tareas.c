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

/** @file inicializar_tareas.c
 ** @brief Implementación de la inicialización de las tareas
 */
/* === Headers files inclusions ================================================================ */

#include "rtos_tareas.h"
#include "inicializar_tareas.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

/* === Private function definitions ============================================================ */

static rtos_context_t contexto_reloj;

/* === Public variable definition  ============================================================= */

EventGroupHandle_t eventos_teclado;

/* === Private function definitions ============================================================ */

/* === Public function definitions ============================================================ */

/* === Public function implementation ========================================================== */

void InicializarTareasRTOS(void) {
    contexto_reloj.placa = BoardCreate();
    contexto_reloj.reloj = ClockCreate(1, ManejadorAlarma);
    eventos_teclado = xEventGroupCreate();
    xTaskCreate(TareaDisplay, "Display", configMINIMAL_STACK_SIZE, (void *)contexto_reloj.placa,
                (configMAX_PRIORITIES - 1), NULL);
    xTaskCreate(TareaContarTiempo, "Reloj", configMINIMAL_STACK_SIZE, (void *)contexto_reloj.reloj,
                (configMAX_PRIORITIES - 2), NULL);
    xTaskCreate(TareaTeclado, "Teclado", configMINIMAL_STACK_SIZE, (void *)contexto_reloj.placa,
                (configMAX_PRIORITIES - 3), NULL);
    xTaskCreate(TareaFSM, "FSM", configMINIMAL_STACK_SIZE * 2, (void *)&contexto_reloj, (configMAX_PRIORITIES - 4),
                NULL);
}

/* === End of documentation ==================================================================== */

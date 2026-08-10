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

/**
 * @brief Definición de bits para los Eventos del sistema gestionados por FreeRTOS
 *
 */
#define EVENTO_F1_LARGO    (1 << 0) /**< Bit 0: Evento de pulsación larga de la tecla F1 */
#define EVENTO_F2_LARGO    (1 << 1) /**< Bit 1: Evento de pulsación larga de la tecla F2 */
#define EVENTO_F3          (1 << 2) /**< Bit 2: Evento de pulsación de la tecla F3 */
#define EVENTO_F4          (1 << 3) /**< Bit 3: Evento de pulsación de la tecla F4 */
#define EVENTO_ACEPTAR     (1 << 4) /**< Bit 4: Evento de pulsación de la tecla ACEPTAR */
#define EVENTO_CANCELAR    (1 << 5) /**< Bit 5: Evento de pulsación de la tecla CANCELAR */
#define EVENTO_ALARMA      (1 << 6) /**< Bit 6: Evento asíncrono disparado cuando el reloj alcanza la hora de alarma */

/**
 * @brief Tiempos de configuración para el filtrado de botones
 *
 */
#define PERIODO_TECLADO_MS 50 /**< Tiempo de refresco y antirrebote para la lectura del teclado en ms */
#define TICKS_PULSACION_LARGA                                                                                          \
    (3000 / PERIODO_TECLADO_MS) /**< Cantidad de ciclos equivalentes a 3 segundos de pulsación */

/* === Public data type declarations =============================================================================== */

/**
 * @brief Estructura de contexto para pasar múltiples instancias a las tareas
 *
 * Agrupa punteros de los objetos principales del sistema evitando el uso de varaibles globales
 *
 */
typedef struct {
    board_t placa;
    clock_t reloj;
} rtos_context_t;

/**
 * @brief Estados posibles de la Máquina de Estados Finitos del reloj
 */
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

/**
 * @brief Manejador global del grupo de eventos para comunicar el teclado y la alarma con la FSM
 *
 */
extern EventGroupHandle_t eventos_teclado;

/**
 * @brief Semáforo Mutex para proteger el acceso a los datos internos del reloj
 *
 */
extern SemaphoreHandle_t mutex_reloj;

/**
 * @brief Semáforo Mutex para proteger la escritura de buffers del display
 *
 */
extern SemaphoreHandle_t mutex_display;

/* === Public function declarations ================================================================================ */

/**
 * @brief Tarea de alta prioridad encargada del barrido multiplexado del display
 *
 * @param Parametros Puntero al contexto o a la placa, casteado a (void *)
 */
void TareaDisplay(void * Parametros);

/**
 * @brief Tarea encargada de incrementar la base de tiempo del reloj
 * Se ejecuta periódicamente cada 1 segundo exacto.
 *
 * @param Parametros Puntero a la instancia del reloj, casteado a (void *)
 */
void TareaContarTiempo(void * Parametros);

/**
 * @brief Tarea encargada de leer el estado físico de los pulsadores
 * Realiza el debouncing y levanta las banderas de eventos
 *
 * @param Parametros Puntero al objeto de la placa, casteado a (void *)
 */
void TareaTeclado(void * Parametros);

/**
 * @brief Tarea de la máquina de estados (FSM) principal de la interfaz de usuario
 * Espera bloqueada a que ocurran eventos del teclado o la alarma para cambiar la lógica visual.
 *
 * @param Parametros Puntero al objeto de contexto (rtos_context_t), casteado a (void *)
 */
void TareaFSM(void * Parametros);

/**
 * @brief Función de callback ejecutada por el módulo del reloj
 * Es llamada automáticamente cuando la hora actual coincide con la hora de alarma seteada.
 *
 * @param estado true si la alarma debe activarse, false en caso contrario
 */
void ManejadorAlarma(bool estado);

/* === End of conditional blocks =================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* RTOS_TAREAS_H_ */
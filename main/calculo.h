#ifndef CALCULO_H
#define CALCULO_H

/*
 * Calculos fisicos del experimento.
 * Este modulo no usa hardware: se puede probar en la PC (ver test/test_calculo.c).
 */

#include <stdbool.h>
#include <stdint.h>

#define CARGA_ELECTRON_C    1.602176634e-19     // [C]
#define VELOCIDAD_LUZ_M_S   299792458.0         // [m/s]
#define PLANCK_REFERENCIA   6.62607015e-34      // [J s]

/* Un punto de la curva I-V */
typedef struct {
    float frenado_v;        // Potencial de frenado medido [V]
    float corriente_na;     // Fotocorriente medida [nA]
} punto_t;

typedef enum {
    CORTE_OK = 0,           // Se encontro el potencial de corte
    CORTE_SIN_FOTOEMISION,  // La corriente sin frenado es casi nula (por ejemplo, LED infrarrojo)
    CORTE_FUERA_DE_RANGO,   // La corriente nunca bajo del umbral dentro del barrido
} resultado_corte_t;

/* Resultado del ajuste lineal U0 = (h/e) f - W/e */
typedef struct {
    int cantidad;               // Cantidad de LED usados en el ajuste
    double h;                   // Constante de Planck estimada [J s]
    double u_h;                 // Incertidumbre estandar de h (solo con 3 o mas puntos) [J s]
    double funcion_trabajo_ev;  // Funcion trabajo W [eV]
    double r2;                  // Coeficiente de determinacion del ajuste
} ajuste_planck_t;

/**
 * @brief                       Convierte longitud de onda en frecuencia
 * @param[in] longitud_onda_nm  Longitud de onda [nm]
 * @return double               Frecuencia [Hz]
 */
double calculo_frecuencia_hz(uint16_t longitud_onda_nm);

/**
 * @brief                       Busca el potencial de corte de una curva I-V
 * @note                        Criterio: primera tension donde la corriente (menos la oscura)
 *                              cae por debajo de umbral_fraccion x corriente sin frenado.
 *                              Entre los dos puntos que cruzan el umbral se interpola linealmente.
 * @param[in] puntos            Curva ordenada de menor a mayor frenado. puntos[0] debe ser 0 V
 * @param[in] cantidad          Cantidad de puntos
 * @param[in] corriente_oscura_na  Corriente con el LED apagado [nA]
 * @param[in] umbral_fraccion   Fraccion de la corriente inicial que define el corte (ej. 0.02)
 * @param[in] corriente_minima_na  Debajo de esta corriente inicial no hay fotoemision
 * @param[out] potencial_corte_v   Potencial de corte encontrado [V]
 * @return resultado_corte_t    CORTE_OK si se encontro
 */
resultado_corte_t calculo_potencial_corte(const punto_t *puntos, int cantidad,
                                          float corriente_oscura_na, float umbral_fraccion,
                                          float corriente_minima_na, float *potencial_corte_v);

/**
 * @brief                       Ajuste por cuadrados minimos de U0 en funcion de f
 * @param[in] frecuencia_hz     Frecuencias de los LED con corte valido [Hz]
 * @param[in] potencial_v       Potenciales de corte [V]
 * @param[in] cantidad          Cantidad de pares (minimo 2)
 * @param[out] ajuste           Resultado del ajuste
 * @return bool                 true si el ajuste se pudo calcular
 */
bool calculo_constante_planck(const double *frecuencia_hz, const double *potencial_v,
                              int cantidad, ajuste_planck_t *ajuste);

#endif /* CALCULO_H */

#include "calculo.h"

#include <math.h>

double calculo_frecuencia_hz(uint16_t longitud_onda_nm)
{
    return VELOCIDAD_LUZ_M_S / (longitud_onda_nm * 1e-9);
}

resultado_corte_t calculo_potencial_corte(const punto_t *puntos, int cantidad,
                                          float corriente_oscura_na, float umbral_fraccion,
                                          float corriente_minima_na, float *potencial_corte_v)
{
    if(cantidad < 2) {
        return CORTE_FUERA_DE_RANGO;
    }

    // Corriente sin frenado, descontando la corriente oscura
    float corriente_inicial = puntos[0].corriente_na - corriente_oscura_na;
    if(corriente_inicial < corriente_minima_na) {
        return CORTE_SIN_FOTOEMISION;
    }

    float umbral = umbral_fraccion * corriente_inicial;

    for(int i = 1; i < cantidad; i++) {
        float corriente = puntos[i].corriente_na - corriente_oscura_na;
        if(corriente <= umbral) {
            // Interpolacion lineal entre el punto anterior (arriba del umbral) y este (abajo)
            float v1 = puntos[i - 1].frenado_v;
            float v2 = puntos[i].frenado_v;
            float i1 = puntos[i - 1].corriente_na - corriente_oscura_na;
            float i2 = corriente;

            if(i1 == i2) {
                *potencial_corte_v = v2;
            }
            else {
                *potencial_corte_v = v1 + (umbral - i1) * (v2 - v1) / (i2 - i1);
            }
            return CORTE_OK;
        }
    }

    return CORTE_FUERA_DE_RANGO;
}

bool calculo_constante_planck(const double *frecuencia_hz, const double *potencial_v,
                              int cantidad, ajuste_planck_t *ajuste)
{
    if(cantidad < 2) {
        return false;
    }

    // Promedios
    double x_medio = 0;
    double y_medio = 0;
    for(int i = 0; i < cantidad; i++) {
        x_medio += frecuencia_hz[i];
        y_medio += potencial_v[i];
    }
    x_medio /= cantidad;
    y_medio /= cantidad;

    // Sumas de cuadrados
    double sxx = 0;
    double sxy = 0;
    double syy = 0;
    for(int i = 0; i < cantidad; i++) {
        double dx = frecuencia_hz[i] - x_medio;
        double dy = potencial_v[i] - y_medio;
        sxx += dx * dx;
        sxy += dx * dy;
        syy += dy * dy;
    }
    if(sxx == 0) {
        return false;   // Todos los LED con la misma frecuencia
    }

    // Recta U0 = pendiente * f + ordenada
    double pendiente = sxy / sxx;
    double ordenada = y_medio - pendiente * x_medio;

    // Suma de los residuos al cuadrado
    double residuos = 0;
    for(int i = 0; i < cantidad; i++) {
        double error = potencial_v[i] - (pendiente * frecuencia_hz[i] + ordenada);
        residuos += error * error;
    }

    // Incertidumbre estandar de la pendiente (necesita al menos 3 puntos)
    double u_pendiente = 0;
    if(cantidad >= 3) {
        u_pendiente = sqrt(residuos / (cantidad - 2) / sxx);
    }

    ajuste->cantidad = cantidad;
    ajuste->h = pendiente * CARGA_ELECTRON_C;           // pendiente = h / e
    ajuste->u_h = u_pendiente * CARGA_ELECTRON_C;
    ajuste->funcion_trabajo_ev = -ordenada;             // ordenada = -W / e
    ajuste->r2 = (syy > 0) ? 1.0 - residuos / syy : 1.0;
    return true;
}

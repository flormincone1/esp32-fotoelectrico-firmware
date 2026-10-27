/*
 * Prueba del modulo de calculo en la PC, sin ESP32.
 *
 *   gcc -I../main test_calculo.c ../main/calculo.c -lm -o test_calculo && ./test_calculo
 *
 * Genera curvas I-V sinteticas con un valor conocido de h y verifica que el
 * firmware lo recupere.
 */

#include <math.h>
#include <stdio.h>

#include "calculo.h"

#define PUNTOS      101
#define PASO_V      0.025f

static int fallas = 0;

static void verificar(const char *nombre, double obtenido, double esperado, double tolerancia)
{
    int ok = fabs(obtenido - esperado) <= tolerancia;
    printf("[%s] %-34s obtenido = %.4g, esperado = %.4g\n", ok ? " OK " : "FALLA", nombre, obtenido, esperado);
    if(!ok) {
        fallas++;
    }
}

/* Curva I-V simplificada: corriente que baja lineal hasta U0 y despues queda en la oscura */
static void generar_curva(punto_t *curva, float u0, float i0, float oscura)
{
    for(int i = 0; i < PUNTOS; i++) {
        float v = i * PASO_V;
        float i_foto = (v < u0) ? i0 * (1.0f - v / u0) : 0.0f;
        curva[i].frenado_v = v;
        curva[i].corriente_na = i_foto + oscura;
    }
}

int main(void)
{
    const uint16_t longitudes[] = { 395, 470, 525, 625 };
    const double w_ev = 1.6;    // Funcion trabajo simulada [eV]
    double frecuencias[4];
    double cortes[4];
    punto_t curva[PUNTOS];

    // 1) Potencial de corte de cada curva sintetica
    for(int k = 0; k < 4; k++) {
        double f = calculo_frecuencia_hz(longitudes[k]);
        double u0 = PLANCK_REFERENCIA * f / CARGA_ELECTRON_C - w_ev;
        generar_curva(curva, (float)u0, 2.0f, 0.03f);

        float corte = 0;
        resultado_corte_t r = calculo_potencial_corte(curva, PUNTOS, 0.03f, 0.02f, 0.05f, &corte);
        verificar("resultado CORTE_OK", r, CORTE_OK, 0);
        // Con umbral del 2 % el corte queda un 2 % antes del U0 ideal
        verificar("potencial de corte [V]", corte, u0 * 0.98, 0.01);

        frecuencias[k] = f;
        cortes[k] = u0;
    }

    // 2) LED sin fotoemision (control negativo)
    float corte = 0;
    generar_curva(curva, 0.5f, 0.0f, 0.03f);
    verificar("sin fotoemision", calculo_potencial_corte(curva, PUNTOS, 0.03f, 0.02f, 0.05f, &corte),
              CORTE_SIN_FOTOEMISION, 0);

    // 3) Corte fuera del barrido
    generar_curva(curva, 5.0f, 2.0f, 0.0f);
    verificar("corte fuera de rango", calculo_potencial_corte(curva, PUNTOS, 0.0f, 0.02f, 0.05f, &corte),
              CORTE_FUERA_DE_RANGO, 0);

    // 4) Ajuste de Planck con datos exactos
    ajuste_planck_t ajuste;
    verificar("ajuste calculado", calculo_constante_planck(frecuencias, cortes, 4, &ajuste), 1, 0);
    verificar("h [J s]", ajuste.h, PLANCK_REFERENCIA, 1e-38);
    verificar("funcion trabajo [eV]", ajuste.funcion_trabajo_ev, w_ev, 1e-6);
    verificar("R2", ajuste.r2, 1.0, 1e-9);

    // 5) Ajuste con ruido: la incertidumbre tiene que ser distinta de cero
    cortes[1] += 0.02;
    cortes[2] -= 0.02;
    calculo_constante_planck(frecuencias, cortes, 4, &ajuste);
    printf("Con ruido: h = %.3e +- %.1e J s, W = %.3f eV, R2 = %.4f\n",
           ajuste.h, ajuste.u_h, ajuste.funcion_trabajo_ev, ajuste.r2);
    verificar("u(h) > 0", ajuste.u_h > 0, 1, 0);

    printf("\n%s\n", fallas == 0 ? "TODAS LAS PRUEBAS PASARON" : "HAY PRUEBAS QUE FALLARON");
    return fallas != 0;
}

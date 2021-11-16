#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "lista.h"
#include "hash.h"
#define CAP_INICIAL 50ul
#define FACTOR_NVA_CAP 2ul
#define FACTOR_CARGA_MAX 2.5
#define FACTOR_CARGA_MIN 0.2
#define LARGO_MAX_CLAVES 150ul
// #define FUN_HASHING djb2 // Recuperar

// Estructuras
typedef struct hash{
    void** arreglo;
    size_t capacidad;
    size_t carga;
    hash_destruir_dato_t funcion_destruir_dato;
} hash_t;

typedef struct hash_iter{
    const hash_t* hash;
    int nro_elemento; // Eliminar si no se usa
    size_t pos_en_arreglo;
    lista_iter_t* iterador_lista_actual;
} hash_iter_t;

typedef struct campo{
    char* clave;
    void* dato;
} campo_t;

typedef bool (*visitar)(void* dato, void* extra);

// Funcion de hashing DJB2
// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long djb2(const char* str){
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// Primitivas y funciones
campo_t* campo_crear(const char* clave, void* dato){
    campo_t* campo = malloc(sizeof(campo_t));
    if (campo == NULL) return NULL;

    campo->clave = malloc(sizeof(char) * LARGO_MAX_CLAVES); 
    strcpy(campo->clave, clave);
    campo->dato = dato;

    return campo;
}

hash_t* hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if (hash == NULL) return NULL;

    void** arreglo_dinamico = malloc(sizeof(lista_t*) * CAP_INICIAL);
    if (arreglo_dinamico == NULL) return NULL;

    hash->arreglo = arreglo_dinamico;
    hash->capacidad = CAP_INICIAL;
    hash->carga = 0;
    hash->funcion_destruir_dato = destruir_dato;

    for (int i = 0; i < hash->capacidad; i++){
        hash->arreglo[i] = lista_crear();
    }

    return hash;
}

// Funcion auxiliar
// Retorna un iterador posicionado en la posicion de la lista de la clave buscada
// El iterador queda al final si no se encuentra la clave
lista_iter_t* aux_posicionar_iterador(const hash_t* hash, const char* clave){
    size_t posicion = djb2(clave) % hash->capacidad;
    lista_iter_t* iterador = lista_iter_crear(hash->arreglo[posicion]);
    campo_t* campo;
    while (!lista_iter_al_final(iterador)){
        campo = lista_iter_ver_actual(iterador);
        if (strcmp(campo->clave, clave) == 0)return iterador;
        lista_iter_avanzar(iterador);
    }
    return iterador;
}

bool hash_pertenece(const hash_t* hash, const char* clave){
    lista_iter_t* iterador = aux_posicionar_iterador(hash, clave); // No me convence este función. Revisar
    bool resultado = !lista_iter_al_final(iterador);
    lista_iter_destruir(iterador);
    return resultado;
}

void* hash_obtener(const hash_t* hash, const char* clave){
    if (!hash_pertenece(hash, clave))
        return NULL;

    lista_iter_t* iterador = aux_posicionar_iterador(hash, clave); // No me convence este función. Revisar
    campo_t* campo = lista_iter_ver_actual(iterador);
    void* dato = campo->dato;
    lista_iter_destruir(iterador);
    return dato;
}

size_t hash_cantidad(const hash_t* hash){
    return hash->carga;
}

void des_campo(void* campo){
    if (campo == NULL) return;
    campo_t* dato = campo;
    free(dato->clave);
    free(dato);
}

bool destruir_dato(void* dato, void* extra){
    hash_t* hash = extra;
    if (!dato) return false;
    campo_t* campo = dato;
    hash->funcion_destruir_dato(campo->dato);
    return true;
}

void destruir_listas(hash_t* hash){
    for (size_t i = 0; i < hash->capacidad; i++){
        lista_t* lista = hash->arreglo[i];
        if (!lista_esta_vacia(lista)){
            if (hash->funcion_destruir_dato != NULL) lista_iterar(hash->arreglo[i], destruir_dato, hash);
            lista_destruir(hash->arreglo[i], des_campo);
        } else lista_destruir(hash->arreglo[i], NULL);
    }
}

void borrar_campo(hash_t* hash, const char* clave){
    size_t posicion = djb2(clave) % hash->capacidad;
    campo_t* campo;
    lista_t* lista = hash->arreglo[posicion];
    lista_iter_t* iterador = lista_iter_crear(lista);
    while (!lista_iter_al_final(iterador)){
        campo = lista_iter_ver_actual(iterador);
        if (strcmp(campo->clave, clave) == 0){
            des_campo(campo);
            lista_iter_borrar(iterador);
            lista_iter_destruir(iterador);
            return;
        }
        lista_iter_avanzar(iterador);
    }
    lista_iter_destruir(iterador);
    return;
}

void hash_destruir(hash_t* hash){
    destruir_listas(hash);
    free(hash->arreglo);
    free(hash);
}

bool guardar_campo(void* dato, void* extra){
    campo_t* campo = dato;
    if (campo == NULL) return false;
    hash_t* hash = extra;
    size_t posicion = djb2(campo->clave) % hash->capacidad;
    lista_t* lista = hash->arreglo[posicion];
    lista_insertar_ultimo(lista, campo);
    return true;
}

bool hash_redimensionar(hash_t* hash, size_t nueva_capacidad){
    if (nueva_capacidad < CAP_INICIAL) nueva_capacidad = CAP_INICIAL;
    void** nuevo_arreglo = malloc(sizeof(lista_t*) * nueva_capacidad);
    if (nuevo_arreglo == NULL) return false;
    
    void** viejo_arreglo = hash->arreglo;
    size_t vieja_cap = hash->capacidad;
    lista_t* lista_actual;
    hash->capacidad = nueva_capacidad;
    hash->arreglo = nuevo_arreglo;
    for (int i = 0; i < hash->capacidad; i++){
        hash->arreglo[i] = lista_crear();
    }
    for (int i = 0; i < vieja_cap; i++){
        lista_actual = viejo_arreglo[i];
        if (!lista_esta_vacia(lista_actual)) lista_iterar(lista_actual, guardar_campo, hash);
        lista_destruir(lista_actual, NULL);
    }
    free(viejo_arreglo);
    return true;
}

void* hash_borrar(hash_t* hash, const char* clave){
    if (!hash_pertenece(hash, clave))return NULL;
    void* dato = hash_obtener(hash, clave);
    borrar_campo(hash, clave);
    hash->carga--;
    if (hash->carga / hash->capacidad < FACTOR_CARGA_MIN && hash->capacidad > CAP_INICIAL) {
        //hash_redimensionar(hash, hash->capacidad / FACTOR_NVA_CAP); <----- SOLUCIONAR. No funciona redimensionar hacia abajo, hay un doble free
    }
    return dato;
}

bool hash_guardar(hash_t* hash, const char* clave, void* dato){
    if (hash_pertenece(hash, clave)){
        void* dato_reemplazado = hash_borrar(hash, clave);
        if (hash->funcion_destruir_dato != NULL){
            hash_destruir_dato_t funcion_dest = hash->funcion_destruir_dato;
            funcion_dest(dato_reemplazado);
            //hash->funcion_destruir_dato(dato_reemplazado); // ACORTAR A UN LINEA. ALGO ASÍ
        }
    }
    size_t posicion = djb2(clave) % hash->capacidad;
    campo_t* campo_agregado = campo_crear(clave, dato);
    lista_insertar_ultimo(hash->arreglo[posicion], campo_agregado);
    hash->carga++;

    if (hash->carga / hash->capacidad > FACTOR_CARGA_MAX) hash_redimensionar(hash, hash->capacidad * FACTOR_NVA_CAP);
    return true;
}

/* Iterador del hash */

// Crea iterador
hash_iter_t* hash_iter_crear(const hash_t* hash)
{
    hash_iter_t* iterador = malloc(sizeof(hash_iter_t));
    if (iterador == NULL)
        return NULL;

    iterador->hash = hash;
    iterador->pos_en_arreglo = 0;
    if (hash->carga == 0) {
        iterador->iterador_lista_actual = NULL;
        iterador->pos_en_arreglo = -1;
        return iterador;
    }
    lista_t* lista_pos_arreglo = iterador->hash->arreglo[iterador->pos_en_arreglo];
    while(lista_esta_vacia(lista_pos_arreglo)){
        iterador->pos_en_arreglo++;
        lista_pos_arreglo = iterador->hash->arreglo[iterador->pos_en_arreglo];
    }
    iterador->iterador_lista_actual = lista_iter_crear(iterador->hash->arreglo[iterador->pos_en_arreglo]);
    iterador->nro_elemento = 0;
    return iterador;
}

// Avanza iterador
bool hash_iter_avanzar(hash_iter_t* iter){
    if (hash_iter_al_final(iter)) return false;
    lista_iter_avanzar(iter->iterador_lista_actual);
    if (lista_iter_al_final(iter->iterador_lista_actual)){
        iter->pos_en_arreglo++;
        while(lista_esta_vacia(iter->hash->arreglo[iter->pos_en_arreglo])){
            iter->pos_en_arreglo++;
            if (iter->pos_en_arreglo == iter->hash->capacidad){
                iter->pos_en_arreglo = -1;
                return true;
            }
        }
        lista_iter_destruir(iter->iterador_lista_actual);
        iter->iterador_lista_actual = lista_iter_crear(iter->hash->arreglo[iter->pos_en_arreglo]);
    }
    return true;
}

// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char* hash_iter_ver_actual(const hash_iter_t* iter)
{
    if (hash_iter_al_final(iter)) return NULL;
    campo_t* campo_actual = lista_iter_ver_actual(iter->iterador_lista_actual);
    return campo_actual->clave;
}

// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t* iter){
    return iter->pos_en_arreglo == -1;
}

// Destruye iterador
void hash_iter_destruir(hash_iter_t* iter){
    if(iter->iterador_lista_actual != NULL) lista_iter_destruir(iter->iterador_lista_actual);
    free(iter);
}

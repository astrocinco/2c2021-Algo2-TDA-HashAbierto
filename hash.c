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
#define LARGO_MAX_CLAVES 40ul

//#define FUN_HASHING djb2 // Recuperar



// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long djb2(const char *str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// Los structs deben llamarse "hash" y "hash_iter".
typedef struct hash{
    void** arreglo;
    long unsigned int capacidad;
    long unsigned int carga;
    hash_destruir_dato_t funcion_destruir_dato; // Tal vez error en esta funcion por estar pasando directamente y no pasar &
    // O por no hacer * para asignarlo como tipo de dato hash_destruir_dato_t en un primitiva
} hash_t;

typedef struct hash_iter{
    const hash_t* hash;
    int pos_en_arreglo;
    lista_iter_t* iterador_pos_arreglo;
    bool al_final;
} hash_iter_t;

typedef struct campo{
    char* clave;
    void* dato;
} campo_t;

typedef bool (*visitar)(void *dato, void*extra);

campo_t* campo_crear(const char* clave, void* dato){
    campo_t* campo = malloc(sizeof(campo_t));
    if (campo == NULL) return NULL;

    campo->clave = malloc(sizeof(char) * LARGO_MAX_CLAVES);
    strcpy(campo->clave, clave);
    campo->dato = dato;

    return campo;
}
/*
void campo_destruir_clave(campo_t* campo){
    printf("Campo siendo destruido: %s\n", campo->clave);
    free(campo->clave);
}
*/
hash_t *hash_crear(hash_destruir_dato_t destruir_dato){
    hash_t* hash = malloc(sizeof(hash_t));
    if (hash == NULL) return NULL;

    void** arreglo_dinamico = malloc(sizeof(lista_t*) * CAP_INICIAL);
    if (arreglo_dinamico == NULL) return NULL;

    hash->arreglo = arreglo_dinamico;
    hash->capacidad = CAP_INICIAL;
    hash->carga = 0;
    hash->funcion_destruir_dato = destruir_dato;

    for (int i = 0; i < CAP_INICIAL; i++){
        hash->arreglo[i] = lista_crear(); 
    }
    return hash;
}

// Funcion auxiliar
// Retorna un iterador posicionado en la posicion de la lista de la clave buscada
// El iterador queda al final si no se encuentra la clave
lista_iter_t* aux_posicionar_iterador(const hash_t* hash, const char* clave){
    //printf("83\n");
    long unsigned int posicion = djb2(clave) % hash->capacidad;
    //printf("86: %d\n", posicion);
    lista_iter_t* iterador = lista_iter_crear(hash->arreglo[posicion]);
    campo_t* campo;
    while(!lista_iter_al_final(iterador)){
        campo = lista_iter_ver_actual(iterador);
        if (strcmp(campo->clave, clave) == 0){
            return iterador;
        }
        lista_iter_avanzar(iterador);
    }
    return iterador;
}

//Funcion auxiliar
// Iterador interno. Recibe una función y un parametro para esta
void hash_iterador_interno(hash_t* hash, visitar funcion, void* extra){
    if (hash->carga == 0) return;

    long unsigned int posicion_arreglo = 0;
    lista_t* lista;
    lista_iter_t* iterador_lista;
    campo_t* campo;

    while(posicion_arreglo < hash->capacidad){
        lista = hash->arreglo[posicion_arreglo];
        if (lista_largo(lista) == 0) {
            posicion_arreglo++;
            continue;
        }
        iterador_lista = lista_iter_crear(lista);
        while(!lista_iter_al_final(iterador_lista)){
            campo = lista_iter_ver_actual(iterador_lista);
            if (extra == NULL){ // Potencialmente redundante
                funcion(campo->dato, NULL);
            }
            else funcion(campo->dato, extra);
            lista_iter_avanzar(iterador_lista);
        }
    }
}

bool hash_redimensionar(hash_t* hash, long unsigned int nueva_capacidad){ 
    printf("Linea 132\n");
    printf("133 Carga: %ld - Capacidad: %ld", hash->carga, hash->capacidad);
    void** nuevo_arreglo_dinamico = malloc(sizeof(lista_t*) * CAP_INICIAL);
    if (nuevo_arreglo_dinamico == NULL) return false;
    // Crear nuevo arreglo ^

    long unsigned int posicion_arreglo = 0;
    lista_t* lista;
    lista_iter_t* iterador_lista;
    campo_t* campo;

    while(posicion_arreglo < hash->capacidad){
        lista = hash->arreglo[posicion_arreglo];
        if (lista_largo(lista) == 0) {
            posicion_arreglo++;
            continue;
        }
        iterador_lista = lista_iter_crear(lista);
        while(!lista_iter_al_final(iterador_lista)){
            campo = lista_iter_ver_actual(iterador_lista);
            long unsigned int posicion_nuevo = djb2(campo->clave) % hash->capacidad;
            printf("%zu\n", posicion_nuevo);
            lista_insertar_primero(nuevo_arreglo_dinamico[posicion_nuevo], campo); // Es lo mismo insertar_primero o insertar_ultimo?
            lista_iter_avanzar(iterador_lista);
        }
    }
    // Pasar todos elementos a nueva pos ^(considerando fun hashing % nueva_cap)
    free(hash->arreglo);
    // Eliminar viejo arreglo
    hash->arreglo = nuevo_arreglo_dinamico;
    // Asignar nuevo arreglo como actual ^
    return true;
}

bool hash_pertenece(const hash_t *hash, const char *clave){
    lista_iter_t* iterador = aux_posicionar_iterador(hash, clave);
    bool resultado = !lista_iter_al_final(iterador);
    lista_iter_destruir(iterador);
    return resultado;
}

bool hash_guardar(hash_t *hash, const char *clave, void *dato){ 
    if (hash_pertenece(hash, clave)) {

        void* dato_reemplazado = hash_borrar(hash, clave);

        if (hash->funcion_destruir_dato != NULL) {
            hash_destruir_dato_t funcion_dest = hash->funcion_destruir_dato;

            funcion_dest(dato_reemplazado);
            //hash->funcion_destruir_dato(dato_reemplazado); // ACORTAR A UN LINEA. ALGO ASÍ    
        }
        //printf("189\n");
    } 

    long unsigned int posicion = djb2(clave) % hash->capacidad;
    campo_t* campo_agregado = campo_crear(clave, dato);
    lista_insertar_ultimo(hash->arreglo[posicion], campo_agregado);
    printf("Dato de campo nuevo: %s\n", (char*)campo_agregado->dato);
    hash->carga++;
    //printf("193 Carga: %ld - Capacidad: %ld\n", hash->carga, hash->capacidad);
    //printf("%d %d %d", FACTOR_CARGA_MAX, FACTOR_NVA_CAP, FACTOR_CARGA_MIN);
    if (hash->carga / hash->capacidad > FACTOR_CARGA_MAX) hash_redimensionar(hash, hash->capacidad * FACTOR_NVA_CAP);
    printf("--191-- Debugging: %s Parametro: %s\n", (char*)hash_obtener(hash, clave), (char*)dato);
    return true;
}

void *hash_obtener(const hash_t *hash, const char *clave){
    if (!hash_pertenece(hash, clave)) return NULL;

    lista_iter_t* iterador = aux_posicionar_iterador(hash, clave);
    campo_t* campo = lista_iter_ver_actual(iterador);
    void* dato = campo->dato;
    lista_iter_destruir(iterador);
    return dato;
}

size_t hash_cantidad(const hash_t *hash){
    return hash->carga;
}

void des_campo(void* campo){
    campo_t* dato = campo;
    free(dato->clave);
    free(dato);
}

void *hash_borrar(hash_t *hash, const char *clave){
    if (!hash_pertenece(hash, clave)) return NULL;

    void* dato = hash_obtener(hash, clave);
    lista_iter_t* iterador = aux_posicionar_iterador(hash, clave);
    campo_t* campo = lista_iter_ver_actual(iterador);
    //campo_destruir(campo);
    des_campo(campo);
    lista_iter_destruir(iterador);
    hash->carga--;

    if (hash->carga / hash->capacidad < FACTOR_CARGA_MIN && hash->capacidad > CAP_INICIAL) hash_redimensionar(hash, hash->capacidad / FACTOR_NVA_CAP);
    return dato;
}

void destruir_datos(hash_t* hash){
    //printf("221\n");
    if (hash->carga == 0) return;
    long unsigned int posicion_arreglo = 0;
    lista_t* lista;
    lista_iter_t* iterador_lista;
    campo_t* campo;
    printf("227\n");
    while(posicion_arreglo < hash->capacidad){
        //printf("    229 %d\n", posicion_arreglo);
        lista = hash->arreglo[posicion_arreglo];
        if (lista_largo(lista) == 0) {
            posicion_arreglo++;
            continue;
        }
        iterador_lista = lista_iter_crear(lista);
        while(!lista_iter_al_final(iterador_lista)){
            printf("        237\n");
            campo = lista_iter_ver_actual(iterador_lista);
            printf("        239 %s\n", (char*)campo->dato);
            hash->funcion_destruir_dato(campo->dato);
            printf("        241\n");
            printf("%d", lista_iter_avanzar(iterador_lista));
            printf("        243\n");
        }
        printf("    245\n");
        lista_iter_destruir(iterador_lista);
    }
}
/*
void destruir_campos(hash_t* hash){
    if (hash->carga == 0) return;
    long unsigned int posicion_arreglo = 0;
    lista_t* lista;
    lista_iter_t* iterador_lista;
    campo_t* campo;

    while(posicion_arreglo < hash->capacidad){
        lista = hash->arreglo[posicion_arreglo];
        if (lista_largo(lista) == 0) {
            posicion_arreglo++;
            continue;
        }
        iterador_lista = lista_iter_crear(lista);
        while(!lista_iter_al_final(iterador_lista)){
            campo = lista_iter_ver_actual(iterador_lista);

            campo_destruir_clave(campo);
            lista_iter_avanzar(iterador_lista);
        }
        lista_iter_destruir(iterador_lista);
        posicion_arreglo++;
    }

}
*/


void destruir_listas(hash_t* hash){
    for (size_t i = 0; i < hash->capacidad; i++){
        lista_t* lista= hash->arreglo[i];
        if (!lista_esta_vacia(lista)){
            lista_destruir(hash->arreglo[i], des_campo);
        }else{
            lista_destruir(hash->arreglo[i],NULL);
        }
    }
}

void hash_destruir(hash_t *hash){
    /*
    destruir_campos(hash);
    */
    //destruir(campo->dato); // Hacer funcion para destruir datos con funcion hash->destruir_dato

    destruir_listas(hash);
    free(hash->arreglo);
    free(hash);
}

/* Iterador del hash */

// Crea iterador
hash_iter_t *hash_iter_crear(const hash_t *hash){
    hash_iter_t* iterador = malloc(sizeof(hash_iter_t));
    if (iterador == NULL) return NULL;

    iterador->hash = hash;
    iterador->pos_en_arreglo = 0;
    iterador->iterador_pos_arreglo = lista_iter_crear(iterador->hash->arreglo[iterador->pos_en_arreglo]);
    if (iterador->hash->carga == 0) iterador->al_final = true;
    return iterador;
}

// Avanza iterador
bool hash_iter_avanzar(hash_iter_t *iter){ // REVISAR
    if (iter->al_final) return false;

    if (lista_iter_al_final(iter->iterador_pos_arreglo)){
        iter->pos_en_arreglo++;
        while(lista_esta_vacia(iter->hash->arreglo[iter->pos_en_arreglo])){
            iter->pos_en_arreglo++;
        }
        free(iter->iterador_pos_arreglo);
        iter->iterador_pos_arreglo = lista_iter_crear(iter->hash->arreglo[iter->pos_en_arreglo]);
    } else {
        lista_iter_avanzar(iter->iterador_pos_arreglo);
    }
    return true;
}

// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char *hash_iter_ver_actual(const hash_iter_t *iter){
    if (iter->al_final) return NULL;
    campo_t* campo_actual = lista_iter_ver_actual(iter->iterador_pos_arreglo);
    return campo_actual->clave;
}

// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t *iter){
    return iter->al_final;
}

// Destruye iterador
void hash_iter_destruir(hash_iter_t *iter){
    free(iter->iterador_pos_arreglo);
    free(iter);
}

// 13/11
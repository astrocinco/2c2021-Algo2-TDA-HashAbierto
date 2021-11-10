#include <stdbool.h>
#include <stddef.h>
#include "lista.h"
#include "hash.h"
#define CAP_INICIAL 50
#define FUN_HASHING djb2

// https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned long djb2(unsigned char *str) {
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

// Los structs deben llamarse "hash" y "hash_iter".
typedef struct hash{
    void** arreglo;
    int capacidad;
    int carga;
    hash_destruir_dato_t funcion_destruir_dato;
} hash_t;

typedef struct hash_iter{
    hash_t* hash;
    int pos_en_arreglo;
} hash_iter_t;

typedef struct campo{
    char* clave;
    void* dato;
} campo_t;

// tipo de función para destruir dato
typedef void (*hash_destruir_dato_t)(void *); // Sigo sin entender bien esto. Escribo algo acá?
// O simplemente lo estamos declarando? Porq qué en cola no lo declaramos y en este tda si

campo_t* campo_crear(char* clave, void* dato){
    campo_t* campo = malloc(sizeof(campo_t));
    if (campo == NULL) return NULL;

    campo->clave = clave;
    campo->dato = dato;

    return campo;
}

void campo_destruir(campo_t* campo){ // PUEDE QUE ACÁ FALTE ALGO DE DESTRUIR EL CONTENIDO DEL CAMPO
    free(campo);
}

/* Crea el hash
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

    for (int i = 0; i > CAP_INICIAL; i++){
        hash->arreglo[i] = lista_crear(); 
    }
    return hash;
}

/* Guarda un elemento en el hash, si la clave ya se encuentra en la
 * estructura, la reemplaza. De no poder guardarlo devuelve false.
 * Pre: La estructura hash fue inicializada
 * Post: Se almacenó el par (clave, dato)
 */
bool hash_guardar(hash_t *hash, const char *clave, void *dato){
    int pos_elem_guardado = FUN_HASHING(clave) % hash->capacidad;
    campo_t* campo_agregado = campo_crear(clave, dato);

    lista_insertar_ultimo(hash->arreglo[pos_elem_guardado], campo_agregado);
}

/* Borra un elemento del hash y devuelve el dato asociado.  Devuelve
 * NULL si el dato no estaba.
 * Pre: La estructura hash fue inicializada
 * Post: El elemento fue borrado de la estructura y se lo devolvió,
 * en el caso de que estuviera guardado.
 */
void *hash_borrar(hash_t *hash, const char *clave){
    int posicion = FUN_HASHING(clave) % hash->capacidad;
    lista_iter_t* iterador = lista_iter_crear(hash->arreglo[posicion]);
    campo_t* campo;
    while(!lista_iter_al_final(iterador)) {
        campo = lista_iter_ver_actual(iterador);
        //if () // SEGUIR
    }
}

/* Obtiene el valor de un elemento del hash, si la clave no se encuentra
 * devuelve NULL.
 * Pre: La estructura hash fue inicializada
 */
void *hash_obtener(const hash_t *hash, const char *clave);

/* Determina si clave pertenece o no al hash.
 * Pre: La estructura hash fue inicializada
 */
bool hash_pertenece(const hash_t *hash, const char *clave);

/* Devuelve la cantidad de elementos del hash.
 * Pre: La estructura hash fue inicializada
 */
size_t hash_cantidad(const hash_t *hash){
    return hash->carga;
}
/* Destruye la estructura liberando la memoria pedida y llamando a la función
 * destruir para cada par (clave, dato).
 * Pre: La estructura hash fue inicializada
 * Post: La estructura hash fue destruida
 */
void hash_destruir(hash_t *hash);

/* Iterador del hash */

// Crea iterador
hash_iter_t *hash_iter_crear(const hash_t *hash);

// Avanza iterador
bool hash_iter_avanzar(hash_iter_t *iter);

// Devuelve clave actual, esa clave no se puede modificar ni liberar.
const char *hash_iter_ver_actual(const hash_iter_t *iter);

// Comprueba si terminó la iteración
bool hash_iter_al_final(const hash_iter_t *iter);

// Destruye iterador
void hash_iter_destruir(hash_iter_t *iter);
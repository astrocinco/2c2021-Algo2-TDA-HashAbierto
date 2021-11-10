#ifndef _LISTA_H
#define _LISTA_H

#include <stdbool.h>
#include <stdlib.h>

//definicion de tipos de dato
typedef struct lista lista_t;
typedef struct lista_iter lista_iter_t;


// Crea la lista. 
// Retorna una lista vacía.
lista_t *lista_crear(void);

// Devuelve true o false si la lista está vacía o no.
// Pre: La lista debe haber sido creada anteriormente.
bool lista_esta_vacia(const lista_t *lista);

// Retorna true si se logra insertar el puntero del dato al principio de la lista, o false si falla la primitiva.
// Pre: La lista debe haber sido creada anteriormente.
// Post: Se insertó un dato al principio de la lista.
bool lista_insertar_primero(lista_t *lista, void *dato);

// Retorna true si se logra insertar el puntero del dato al final de la lista, o false si falla la primitiva.
// Pre: La lista debe haber sido creada anteriormente.
// Post: Se insertó un dato al final de la lista.
bool lista_insertar_ultimo(lista_t *lista, void *dato);

// Retorna el puntero del primer elemento y lo elimina de la lista. Si la lista está vacía, retorna NULL.
// Pre: La lista debe haber sido creada anteriormente.
// Post: Se eliminó el primer elemento de la lista, si existía.
void *lista_borrar_primero(lista_t *lista);

// Retorna el puntero del primer elemento de la lista. Si la lista está vacía, retorna NULL.
// Pre: La lista debe haber sido creada anteriormente.
void *lista_ver_primero(const lista_t *lista);

// Retorna el puntero del último elemento de la lista. Si la lista está vacía, retorna NULL.
// Pre: La lista debe haber sido creada anteriormente.
void *lista_ver_ultimo(const lista_t* lista);

// Retorna la cantidad de elementos que hay en la lista.
// Pre: La lista debe haber sido creada anteriormente.
size_t lista_largo(const lista_t *lista);

// Destruye la lista y libera la memoria que ocupaba. 
// Si se recibe la función destruir_dato por parámetro, por cada uno de los elementos de la lista llama a destruir_dato.
// En caso que no se utilice destruir_dato, debe ingresarse "NULL" en su lugar.
// Pre: La lista debe hacer sido creada anteriormente, y destruir_dato debe ser la función necesario para destruir cada elemento de la lista.
// Post: Se destruyó la lista. En caso de ingresarse destruir_dato también se destruyeron sus elementos.
void lista_destruir(lista_t *lista, void (*destruir_dato)(void *)); 

// Aplica la funcion visitar a cada uno de los elementos de la lista (completar).
void lista_iterar(lista_t *lista, bool visitar(void *dato, void *extra), void *extra);

// Crea un iterador apuntando al primer elemento de la lista.
// Pre: la lista fue creada.
lista_iter_t *lista_iter_crear(lista_t *lista);

// Avanza al próximo elemento de la lista. Si está al final y no puede avanzar, retorna false.
// Pre: la lista y el iterador fueron creados.
// Post el iterador está posicionado sobre la próxima posición.
bool lista_iter_avanzar(lista_iter_t *iter);

// Retorna un puntero de dato del elemento actual del iterador. Si está al final retorna NULL.
// Pre: la lista y el iterador fueron creados.
void *lista_iter_ver_actual(const lista_iter_t *iter);

// Retorna true si el iterador está posicionado sobre el final de la lista, false si no.
// Pre: la lista y el iterador fueron creados.
bool lista_iter_al_final(const lista_iter_t *iter);

// Destruye el iterador. No retorna nada.
// Pre: la lista y el iterador fueron creados.
void lista_iter_destruir(lista_iter_t *iter);

// Inserta en la lista el elemento ingresado por parametro. El elemento que antes estaba en esa posición ahora es el siguiente al recién insertado.
// Pre: la lista y el iterador fueron creados.
// Post: Se inserta el elemento y se suma al largo de la lista
bool lista_iter_insertar(lista_iter_t *iter, void *dato);

// Borra el elemento sobre el que estaba el iterador. Si no hay elementos, retorna NULL
// Pre: la lista y el iterador fueron creados.
// Post: El elemento es borrado y se resta al largo de la lista.
void *lista_iter_borrar(lista_iter_t *iter);

// Realiza pruebas automatizadas para verificar la implementación.
void pruebas_lista_estudiante(void);

#endif  // _LISTA_H

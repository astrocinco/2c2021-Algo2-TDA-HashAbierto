#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include "lista.h"

// ESTRUCTURAS
typedef struct nodo{
	void* dato;
	struct nodo* siguiente;
} nodo_t;

struct lista{
	nodo_t* primero;
	nodo_t* ultimo;
	size_t largo;
};

struct lista_iter{
	nodo_t* actual;
	nodo_t* anterior;
	lista_t* lista_iterada; 
};

typedef bool (*visitar)(void *dato, void*extra);
typedef void (*destruir_dato)(void *);

nodo_t *crear_nodo(void* dato){
	nodo_t *nodo = malloc(sizeof(nodo_t));
	if(nodo == NULL){
		return NULL;
	}
	nodo->dato = dato;
	nodo->siguiente = NULL;
	return nodo;
}

// PRIMITIVAS LISTA
lista_t *lista_crear(void){
	lista_t *lista = malloc(sizeof(lista_t));
	if (!lista){
		return NULL;
	}
	lista->primero = NULL;
	lista->ultimo = NULL;
	lista->largo =0;
	return lista;
}

bool lista_esta_vacia(const lista_t *lista){
	return lista->largo == 0;
}

size_t lista_largo(const lista_t *lista){
	return lista->largo;
}

void *lista_ver_ultimo(const lista_t* lista){
	if(lista_esta_vacia(lista)){
		return NULL;
	}
	void* ultimo = lista->ultimo->dato;
	return ultimo;
}

void *lista_ver_primero(const lista_t *lista){
	if(lista_esta_vacia(lista)){
		return NULL;
	}
	void* primero = lista->primero->dato;
	return primero;
}

bool lista_insertar_primero(lista_t *lista, void *dato){
	nodo_t* nodo = crear_nodo(dato);
	if (nodo == NULL){
		return false;
	}
	nodo->siguiente = lista->primero;
	lista->primero = nodo;
	if (lista_esta_vacia(lista)){
		lista->ultimo = nodo;
	}
	(lista->largo)++;
	return true;
}

bool lista_insertar_ultimo(lista_t *lista, void *dato){
	nodo_t* nodo = crear_nodo(dato);
	if (nodo == NULL){
		return false;
	}
	if (lista_esta_vacia(lista)){
		lista->primero = nodo;
	}else{
		lista->ultimo->siguiente = nodo;
	}
	lista->ultimo = nodo;
	(lista->largo)++; 
	return true;
}

void *lista_borrar_primero(lista_t *lista){
	if (lista_esta_vacia(lista)){
		return NULL;
	}
	void* valor = lista->primero->dato;
	nodo_t* actual = lista->primero;
	lista->primero = lista->primero->siguiente;
	(lista->largo)--; 

	free(actual);
	return valor;
}

void lista_destruir(lista_t *lista,destruir_dato destruir){
	while(!lista_esta_vacia(lista)){
		if (destruir != NULL){
			destruir(lista->primero->dato);
		}
		lista_borrar_primero(lista);
	}
	free(lista);
}

// PRIMITIVAS ITERADOR
lista_iter_t *lista_iter_crear(lista_t *lista){
	lista_iter_t *iter = malloc(sizeof(lista_iter_t));
	if (iter == NULL){
		return NULL;
	}
	iter->lista_iterada = lista;
	iter->actual = lista->primero;
	iter->anterior = NULL;

	return iter;
}

bool lista_iter_avanzar(lista_iter_t *iter){
	if (lista_iter_al_final(iter)){
		return false;
	}
	iter->anterior = iter->actual;
	iter->actual = iter->actual->siguiente;
	return true;
}

bool lista_iter_al_final(const lista_iter_t *iter){
	return (iter->anterior == iter->lista_iterada->ultimo);
}

void *lista_iter_ver_actual(const lista_iter_t *iter){
	if (lista_iter_al_final(iter)){
		return NULL;
	}
	void* dato = iter->actual->dato;
	if (lista_iter_al_final(iter)){
		return NULL;
	}
	return dato;
}

void lista_iter_destruir(lista_iter_t *iter){
	free(iter);
}

bool lista_iter_insertar(lista_iter_t *iter, void *dato){
	nodo_t *nuevo = crear_nodo(dato);
	if (nuevo == NULL){
		return false;
	}
	if (lista_esta_vacia(iter->lista_iterada)){
		iter->lista_iterada->primero = nuevo;
		iter->lista_iterada->ultimo = nuevo;
		iter->actual = nuevo;
		(iter->lista_iterada->largo)++;
		return true;
	}
	if (lista_iter_al_final(iter) && !lista_esta_vacia(iter->lista_iterada)){
		iter->lista_iterada->ultimo->siguiente = nuevo;
		iter->lista_iterada->ultimo = nuevo; 
	}
	if (iter->actual == iter->lista_iterada->primero){
		iter->lista_iterada->primero =nuevo;
	}else if (lista_iter_al_final(iter)){
		iter->lista_iterada->ultimo =nuevo;
		iter->anterior->siguiente = nuevo;
	}else{
		iter->anterior->siguiente = nuevo;
	}
	nuevo->siguiente = iter->actual;
	iter->actual = nuevo;
	(iter->lista_iterada->largo)++;
	return true;
}

void *lista_iter_borrar(lista_iter_t *iter){
	if (lista_esta_vacia(iter->lista_iterada) || lista_iter_al_final(iter)) {
		return NULL;
	}
	void* puntero = iter->actual->dato;

	if (iter->actual == iter->lista_iterada->primero && iter->actual == iter->lista_iterada->ultimo){
		iter->lista_iterada->primero = NULL;
		iter->lista_iterada->ultimo = NULL;
		free(iter->actual);
		iter->actual = iter->lista_iterada->ultimo;
	}else if (iter->actual == iter->lista_iterada->primero){
		iter->lista_iterada->primero = iter->actual->siguiente;
		free(iter->actual);
		iter->actual = iter->lista_iterada->primero;
	}else if (iter->actual == iter->lista_iterada->ultimo){
		iter->lista_iterada->ultimo = iter->anterior;
		free(iter->actual);
		iter->actual = NULL;
	} else{
		nodo_t* siguiente = iter->actual->siguiente;
		iter->anterior->siguiente = iter->actual->siguiente;
		free(iter->actual);
		iter->actual = siguiente;
	}

	(iter->lista_iterada->largo)--;
	return puntero;
}

void lista_iterar(lista_t *lista,visitar visita, void *extra){
	if (lista_esta_vacia(lista)){
		return;
	}
	nodo_t* nodo = lista->primero;
	bool visitado = true;

	while(nodo != NULL && visitado==true){
		if (extra == NULL){
			visitado = visita(nodo->dato, NULL);
			nodo = nodo->siguiente;
		}else{
			visitado = visita(nodo->dato,extra);
			nodo = nodo->siguiente;
		}
	}
}

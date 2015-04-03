/*
Tipo lista doblemente enlazada.

Marcos Vázquez Rey
ESAT 2015
*/

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <malloc.h>

struct tNodo {
	int info;
	struct tNodo *next;
	struct tNodo *prev;
};

typedef struct tNodo tNodo;


void list_create(int **ls) {
	*ls = NULL;
}

void list_insert(tNodo **ls, int data) {
	tNodo *p, *q;

	q = *ls;

	p = (tNodo*) malloc(sizeof(tNodo));
	p->next = q;
	p->info = data;
	p->prev = NULL;

	if (q != NULL)
		q->prev = p;

	*ls = p;
}

void list_show(tNodo *ls) {
	tNodo *p;

	p = ls;
	while (p != NULL) {
		printf("%d ", p->info);
		p = p->next;
	}
}

int list_length(tNodo *ls) {
	int lenght = 0;
	tNodo *p;

	p = ls;
	while (p != NULL) {
		p = p->next;
		lenght++;
	}

	return lenght;
}

//Devuelve un puntero apuntando al elemento buscado, o a NULL si no existe
tNodo * list_search(tNodo **ls, int data) {
	tNodo *p;
	int found = 0;

	p = *ls;
	while (p != NULL && found == 0) {
		if (p->info == data)
			found = 1;
		else
			p = p->next;
	}

	return p;
}

//Elimina el elemento de la lista
int list_delete(tNodo **ls, int data) {
	tNodo *p;
	int value = 0;

	p = list_search(&(*ls), data);

	if (p != NULL)
		value = p->info;

	if (p->next != NULL)
		p->next->prev = p->prev;
	if (p->prev != NULL)
		p->prev->next = p->next;

	free(p);
	return value;
}

//Devuelve y extrae el último elemento insertado
int list_extract(tNodo **ls) {
	tNodo *p, *q;
	p = q = *ls;

	if (p != NULL) {
		q = p->next;
	}

	q->prev = NULL;
	*ls = q;

	return p->info;
}

/*
int main(void) {
	tNodo *lista1, *lista2;
	int numero;

	crear_lista(&lista1);
	crear_lista(&lista2);

	insertar_lista(&lista1, 4);
	insertar_lista(&lista1, 15);
	insertar_lista(&lista1, 20);
	insertar_lista(&lista1, 35);

	insertar_lista(&lista2, 14);
	insertar_lista(&lista2, 25);
	insertar_lista(&lista2, 30);
	insertar_lista(&lista2, 45);

	printf("\n");
	mostrar_lista(lista1); printf("Long: %d\n", longitud_lista(lista1));
	mostrar_lista(lista2); printf("Long: %d\n", longitud_lista(lista2));
	printf("\n");

	numero = extraer_lista(&lista1);
	printf("Numero Extraido en lista1 (primero): %d\n ", numero);
	eliminar_lista(&lista1, 15);
	printf("\n");
	mostrar_lista(lista1); printf("Long: %d\n", longitud_lista(lista1));
	mostrar_lista(lista2); printf("Long: %d\n", longitud_lista(lista2));

	getch();
	return 0;
}
*/
struct tNodo {
	int info;
	struct tNodo *next;
	struct tNodo *prev;
};

typedef struct tNodo tNodo;


void list_create(int **ls);

void list_insert(tNodo **ls, int data);

void list_show(tNodo *ls);

int list_length(tNodo *ls);

//Devuelve un puntero apuntando al elemento buscado, o a NULL si no existe
tNodo * list_search(tNodo **ls, int data);

//Elimina el elemento de la lista
int list_delete(tNodo **ls, int data);

//Devuelve y extrae el último elemento insertado
int list_extract(tNodo **ls);
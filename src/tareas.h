/* date = March 11th 2021 10:28 pm */

#ifndef PRUEBA_H
#define PRUEBA_H

#include <stdio.h>
#include <windows.h>

#define DEFAULT_TITLE_CHARS 128
#define DEFAULT_TEXT_CHARS  1024

#define DATA_DIR "../data"



typedef struct vector_2
{
    int x, y;
} v2;


typedef struct _text_buffer
{
    int length = 0;      // Cantidad de caracteres
    int size = 0;        // Tamano del buffer en bytes incluyendo el '\0'
    char *buffer = NULL;
    
} Text_buffer;

// Estructura de una tarea

typedef struct _task
{
    int id;
    bool edited;
    bool done;
    
    Text_buffer title;
} Task;

// Estructura de una lista de tareas

typedef struct _task_list
{
    int id = 0;
    bool all_done = false;
    
    bool parsed = false;
    bool edited = false;
    bool show = true;
    
    bool inside = false;
    bool clicked = false;
    
    // TODO: Temporal (no deberia estar aqui!!)
    int item_pressed = -1;
    
    SYSTEMTIME CreationTime = {0};
    SYSTEMTIME LastWriteTime = {0};
    
    Text_buffer title = {0};
    Text_buffer content = {0};
    
    int scroll = 0;
    int cantidad_tareas = 0;     // Cantidad de elementos en la lista de tareas
    Task *tasks = NULL;          // Puntero a la lista de tareas
    
} Task_List;

// Representacion de la lista que contiene las listas de Tareas
typedef struct _main_list
{
    bool inside = false;
    int item_pressed = -1;
    
    int scroll = 0;
    
    int cantidad_listas = 0;
    Task_List *listas = NULL;
    
} Main_List;


// Inicializa SDL y crea la ventana
bool init();

bool task_parser(Text_buffer *, Task_List *);

bool create_new_list(Main_List *main_list);
bool delete_list(Task_List *list);
bool check_list_title_available(Main_List *main_list, char* title);
bool create_new_list(Main_List *main_list, Text_buffer* new_list_name);
bool create_new_task(Task_List *list, Text_buffer *task_title);

bool save_list_to_file(Task_List *list);

void search_list(Main_List *list, char *search_text);
void reset_search(Main_List *list);

int string_copy (char *source, char *dest, int number);
int text_length(char *string);
int text_length(char *string, char term_char);

int create_text_buffer (Text_buffer *buf, int size);
int resize_text_buffer (Text_buffer *buf, int new_size);
int reset_text_buffer(Text_buffer *buf);
int delete_text_buffer (Text_buffer *buf);

// Depende del SO
void win32_file_parser (char *file_name, Task_List *lista);
void win32_read_dir (char *notes_dir, Main_List* main_list);

void linux_read_dir (char *notes_dir);


#endif //PRUEBA_H

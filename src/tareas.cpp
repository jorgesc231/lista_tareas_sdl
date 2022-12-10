#include <SDL.h>    // Solo por SDL_Log()

#include "tareas.h"

#define ASSERT(x) if(!(x)) __debugbreak();

extern char LISTS_DIR[];


// TODO: Por ahora busca solo en el titulo, tambien deberia buscar en el contenido
void search_list(Main_List *list, char *search_text)
{
    if (list && search_text) {
        for (int i = 0; i < list->cantidad_listas; i++)
        {
            if (strstr(list->listas[i].title.buffer, search_text)) {
                list->listas[i].show = true;
            } else {
                list->listas[i].show = false;
            }
        }
    }
}

void reset_search(Main_List *list)
{
    if (list) {
        for (int i = 0; i < list->cantidad_listas; i++)
        {
            list->listas[i].show = true ? list->listas[i].id != -1 : false;
        }
    }
}

// NOTE: 
bool create_new_list(Main_List *main_list, Text_buffer* new_list_name)
{
    char file_path_to_open[MAX_PATH];
    
    static int nuevo_id = main_list->cantidad_listas - 1;
    nuevo_id++;
    
    snprintf(file_path_to_open, MAX_PATH, "%s\\%s.txt", LISTS_DIR, new_list_name->buffer );
    
    
    HANDLE file_handle = CreateFileA(file_path_to_open, GENERIC_WRITE, 0, 0, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        SDL_Log ("\n\nERROR: Error al crear el archivo %s\n\n", file_path_to_open);
        exit (EXIT_FAILURE);
    }
    
    bool write_success = WriteFile(
        file_handle,
        new_list_name->buffer,
        new_list_name->length,
        NULL,
        NULL);
    
    if (!write_success)
    {
        SDL_Log ("\n\nERROR: Error al Escribir en el archivo %s\n\n", file_path_to_open);
        exit (EXIT_FAILURE);
    }
    
    CloseHandle(file_handle);
    
    
#if 1
    int tamano_listas = sizeof(Task_List) * main_list->cantidad_listas;
    main_list->listas = (Task_List *) realloc(main_list->listas, tamano_listas + sizeof(Task_List));
    
    
    Task_List *nueva_lista = &main_list->listas[main_list->cantidad_listas];
    
    create_text_buffer(&nueva_lista->title, new_list_name->length);
    
    string_copy(new_list_name->buffer, nueva_lista->title.buffer, new_list_name->length);
    
    
    nueva_lista->id = nuevo_id;
    
    nueva_lista->all_done = 0;
    nueva_lista->edited = false;
    nueva_lista->show = true;
    
    nueva_lista->inside = false;
    nueva_lista->clicked = false;
    nueva_lista->item_pressed = -1;
    
    nueva_lista->cantidad_tareas = 0;
    Task *tasks = (Task *) malloc (sizeof(Task));
    
    nueva_lista->scroll = 0;
    
    
    //nueva_lista->parsed = true;
    win32_file_parser(file_path_to_open, nueva_lista);
    
    
    
    main_list->cantidad_listas++;
    
    //main_list->item_pressed = nueva_lista->id;
#endif
    
    // TODO: Temporal
    return true;
}

// NOTE: Al cerrar la app se elimina la memoria
bool delete_list(Task_List *list)
{
    bool result = false;
    char file_to_delete[MAX_PATH];
    
    if (list) {
        // Cambiar show a false
        list->show = false;
        list->id = -1;
        
        // Eliminar el archivo
        snprintf(file_to_delete, MAX_PATH, "%s\\%s.txt", LISTS_DIR, list->title.buffer );
        
        // If the function succeeds, the return value is nonzero.
        // If the function fails, the return value is zero (0). To get extended error information, call GetLastError.
        result = DeleteFileA(file_to_delete);
    }
    
    return result;
}

// NOTE: Debe haber una mucho mejor forma de hacer esto...
bool save_list_to_file(Task_List *list)
{
    bool write_success = false;
    char file_path_to_open[MAX_PATH];
    char done_text[7] = "[ ] - ";
    
    
    snprintf(file_path_to_open, MAX_PATH, "%s\\%s.txt", LISTS_DIR, list->title.buffer );
    
    
    HANDLE file_handle = CreateFileA(file_path_to_open, GENERIC_WRITE, 0, 0, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        SDL_Log ("\n\nERROR: Error al crear el archivo %s\n\n", file_path_to_open);
        exit (EXIT_FAILURE);
    }
    
    for (int i = 0; i < list->cantidad_tareas; i++)
    {
        
        done_text[1] = list->tasks[i].done ? 'x' : ' ';
        
        write_success = WriteFile(
            file_handle,
            done_text,
            6,
            NULL,
            NULL
            );
        
        if (list->tasks[i].title.length > 0) {
            write_success = WriteFile(
                file_handle,
                list->tasks[i].title.buffer,
                //list->tasks[i].title.length, // Aqui deberia ser en length pero esta dando 1 menos...
                list->tasks[i].title.size,
                NULL,
                NULL
                );
        }
        
        write_success = WriteFile(
            file_handle,
            "\n",
            1,
            NULL,
            NULL
            );
        
        if (!write_success)
        {
            SDL_Log ("\n\nERROR: Error al Escribir en el archivo %s\n\n", file_path_to_open);
            break;
        }
        
    }
    
    CloseHandle(file_handle);
    
    return write_success;
}


bool create_new_task(Task_List *list, Text_buffer *task_title)
{
    if (list && task_title)
    {
        list->cantidad_tareas++;
        
        
        // NOTE: Esto es bastante meh
        if (list->cantidad_tareas == 1)
        {
            list->tasks = (Task*) malloc(sizeof(Task) * list->cantidad_tareas);
        } else {
            list->tasks = (Task*) realloc(list->tasks, sizeof(Task) * list->cantidad_tareas);
        }
        
        Task *new_task = &list->tasks[list->cantidad_tareas - 1];
        
        new_task->edited = false;
        new_task->id = list->cantidad_tareas;
        
        new_task->done = false;
        
        create_text_buffer(&new_task->title, task_title->size);
        
        string_copy (task_title->buffer, new_task->title.buffer, task_title->size);
        
        new_task->title.length = task_title->length;
        
        return 1;
        
    } else return 0;
}


bool check_list_title_available(Main_List *main_list, char* title) {
    if (main_list && title) {
        for (int i = 0; i < main_list->cantidad_listas; i++) {
            if (strcmp(main_list->listas[i].title.buffer, title) == 0) {
                return 0;
            }
        }
        
        return 1;
    }
    
    return 0;
}


// TODO: Terminar
bool task_parser(Text_buffer *text, Task_List *list_state)
{
    /* Ejemplo del formato del texto a parsear
    Este va a ser el formate de las listas de tareas
    
[ ] - Agregar el campo de notas
[ ] - Arreglar el campo de titulo
[ ] - Arreglar el scroll de la lista
[ ] - Hacer que solo abra archivos de texto (.txt)
[ ] - Version para Linux
*/
    
    
    int i = 0;
    for (char *c = text->buffer; i < text->length; c = text->buffer++, i++ )
    {
        if (*c == '[' && (i + 2) < text->length)
        {
            bool windows_eol = false;
            
            bool completed = *(c + 1) == 'x' ? true : false;
            
            // NOTE: Para saltarme el [ ] - 
            c = c + 6;
            
            // NOTE: para no aceptar tareas vacias ([] - )
            if (*c == '\n') continue;
            
            int eol_index = text_length(c, '\n');
            
            if (c[eol_index - 1] == '\r')
            {
                //OutputDebugStringA("Formato de Windows");
                windows_eol = true;
                
                c[eol_index - 1] = '\0';
            }
            else {
                //OutputDebugStringA("Formato de Unix");
                c[eol_index] = '\0';
            }
            
            list_state->cantidad_tareas++;
            list_state->tasks = (Task*) realloc(list_state->tasks, sizeof(Task) * list_state->cantidad_tareas);
            
            Task *new_task = &list_state->tasks[list_state->cantidad_tareas - 1];
            
            new_task->edited = false;
            new_task->id = list_state->cantidad_tareas;
            
            new_task->done = completed;
            
            // Inicializa el texto de la tarea (Usa solo el title)
            new_task->title.length = windows_eol ? eol_index - 2 : eol_index - 1;
            new_task->title.size = new_task->title.length + 1;
            
            new_task->title.buffer = c;
        }
    }
    
    // TODO: Esto funciona cuando el texto esta vacio, pero si tiene caracteres no
    // @FIX
    if (i == 0)
    {
        SDL_Log ("No tiene un formato valido\n");
        return false;
    } else {
        return true;
    }
}

// Usar esto en Windows:
// https://docs.microsoft.com/en-us/windows/win32/fileio/listing-the-files-in-a-directory
// TODO: Sacar las rutas absolutas
void win32_read_dir (char *lists_dir, Main_List *main_list)
{
    char file_path_to_open[MAX_PATH];
    char search_path[MAX_PATH];
    
    int file_count = 0;
    
    // https://docs.microsoft.com/en-us/windows/win32/api/minwinbase/ns-minwinbase-win32_find_dataa
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    
    DWORD dwError = 0;
    
    // Prepara el search path
    // TODO: Debe haber una mejor forma de hacer esto...
    strncpy(search_path, lists_dir, MAX_PATH);
    strncat(search_path, "\\*", 3);
    
    // Find the first file in the directory.
    hFind = FindFirstFile (search_path, &ffd);
    
    
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        SDL_Log ("Error: Find handler\n");
        //return dwError;
        exit (EXIT_FAILURE);
    } 
    
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        else file_count++;
        
    } while (FindNextFile(hFind, &ffd) != 0);
    
    FindClose(hFind);
    
    
    main_list->listas = (Task_List *) malloc (sizeof (Task_List) * file_count);
    
    
    // Find the first file in the directory.
    hFind = FindFirstFile (search_path, &ffd);
    
    
    if (hFind == INVALID_HANDLE_VALUE) 
    {
        SDL_Log ("Error: Find handler\n");
        //return dwError;
        exit (EXIT_FAILURE);
    } 
    
    
    // List all the files in the directory with some info about them.
    int i = 0;
    
    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            SDL_Log ("  %s   <DIR>\n", ffd.cFileName);
        } else {
            
            int title_length = text_length (ffd.cFileName, '.');
            
            // NOTE: Saltarse los archivos con titulo en blanco o que no son .txt
            if (title_length <= 0 || strcmp(".txt", &ffd.cFileName[title_length]) != 0) 
            {
                file_count--;
                continue; 
            }
            
            
            // NOTE: 1 dia buscando un error y era porque esto no estaba inicializado.
            main_list->listas[i].id = file_count;
            main_list->listas[i].edited = false;
            main_list->listas[i].all_done = false;
            main_list->listas[i].parsed = false;
            main_list->listas[i].show = true;
            main_list->listas[i].inside = false;
            main_list->listas[i].clicked = false;
            main_list->listas[i].item_pressed = -1;
            
            main_list->listas[i].title = {0};
            main_list->listas[i].content = {0};
            
            main_list->listas[i].scroll = 0;
            main_list->listas[i].cantidad_tareas = 0;     
            main_list->listas[i].tasks = NULL;
            
            // Lectura del titulo
            main_list->listas[i].title.length = title_length;
            
            main_list->listas[i].title.size = main_list->listas[i].title.length + 1;
            main_list->listas[i].title.buffer = (char *) malloc (sizeof (char) * main_list->listas[i].title.size);
            
            if (!main_list->listas[i].title.buffer) {
                SDL_Log ("\nERROR: No se pudo asignar memoria para el titulo\n");
                exit (EXIT_FAILURE);
            }
            
            // Copia el titulo sin la extension
            string_copy (ffd.cFileName, main_list->listas[i].title.buffer, main_list->listas[i].title.length);
            
            snprintf(file_path_to_open, MAX_PATH, "%s\\%s", lists_dir, ffd.cFileName );
            
            //Parsear el texto
            win32_file_parser (file_path_to_open, &main_list->listas[i]);
            
            
            i++;
            
            //filesize.LowPart = ffd.nFileSizeLow;
            //filesize.HighPart = ffd.nFileSizeHigh;
            //SDL_Log ("  %s   %lld bytes\n", ffd.cFileName, filesize.QuadPart);
        }
        
    } while (FindNextFile(hFind, &ffd) != 0 && i <= file_count);
    
    FindClose(hFind);
    
    //list_state.cantidad = file_count;
    main_list->cantidad_listas = file_count;
    
    SDL_Log ("Cantidad de archivos: %d\n", file_count);
}


// Talvez usar https://docs.microsoft.com/es-es/windows/win32/api/fileapi/nf-fileapi-getfileattributesexa para obtener los datos del archivo
// Para los años bisiestos: https://techcommunity.microsoft.com/t5/azure-developer-community-blog/it-s-2020-is-your-code-ready-for-leap-day/ba-p/1157279
void win32_file_parser (char *file_name, Task_List *lista)
{
    TIME_ZONE_INFORMATION tmz;
    FILETIME CreationTime;
    FILETIME LastWriteTime;
    
    HANDLE file_handle;
    char *file_buffer = NULL;
    LARGE_INTEGER file_size;
    
    DWORD bytes_read = 0;
    
    file_handle = CreateFileA(file_name, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    
    if (file_handle == INVALID_HANDLE_VALUE)
    {
        SDL_Log ("\n\nERROR: Error al abrir el archivo %s\n\n", file_name);
        exit (EXIT_FAILURE);
    }
    
    if (!GetFileSizeEx(file_handle, &file_size)) 
    {
        SDL_Log ("\nERROR: Error al obtener el tamaño del archivo\n");
        exit (EXIT_FAILURE);
    }
    
    file_buffer = (char *) malloc (file_size.QuadPart + 1);
    if (!file_buffer)
    {
        SDL_Log ("\nERROR: no se pudo reservar memoria para el buffer del archivo\n");
        exit (EXIT_FAILURE);
    }
    
    // Read from the file
    if (!ReadFile(file_handle, file_buffer, file_size.QuadPart, &bytes_read, 0))
    {
        SDL_Log ("\nERROR: error al leer el archivo\n");
        exit (EXIT_FAILURE);
    }
    
    // Obtiene las fechas y horas de creacion y ultima escritura usando: https://docs.microsoft.com/es-es/windows/win32/sysinfo/file-times
    // y https://docs.microsoft.com/es-es/windows/win32/sysinfo/time-functions
    
    GetTimeZoneInformation(&tmz); //Obtiene la informacion de la zona horaria 
    
    // Obtiene las fechas y horas de creacion y modificacion en UTC
    if (!GetFileTime(file_handle, &CreationTime, 0, &LastWriteTime))
    {
        SDL_Log ("\nERROR: No se pudo obtener los metadatos del archivos\n");
        exit (EXIT_FAILURE);
    }
    
    if (!FileTimeToSystemTime(&CreationTime, &lista->CreationTime)) // Tranforma del tipo FILETIME a SYSTEMTIME
    {
        SDL_Log ("\nERROR: No se pudo obtener la fecha de creacion del archivo\n");
        exit(EXIT_FAILURE);
    }
    
    if (!FileTimeToSystemTime(&LastWriteTime, &lista->LastWriteTime))
    {
        SDL_Log ( "\nERROR: No se pudo obtener la fecha de ultima escritura del archivo\n");
        exit(EXIT_FAILURE);
    }
    
    if (!SystemTimeToTzSpecificLocalTime(&tmz, &lista->CreationTime,  &lista->CreationTime)) // Tranforma de UTC a la zona horaria indicada
    {
        SDL_Log ("\nERROR: No se pudo convertir la fecha de creacion del archivo\n");
        exit(EXIT_FAILURE);
    }
    
    if (!SystemTimeToTzSpecificLocalTime(&tmz, &lista->LastWriteTime, &lista->LastWriteTime))
    {
        SDL_Log("\nERROR: No se pudo convertir la fecha de ultima escritura del archivo\n");
        exit(EXIT_FAILURE);
    }
    
    lista->content.size = file_size.QuadPart;
    lista->content.length = lista->content.size ? lista->content.size - 1 : 0;
    lista->content.buffer = file_buffer;
    lista->content.buffer[file_size.QuadPart] = '\0';
    
    task_parser(&lista->content, lista);
    
    
    //free (file_buffer);
    CloseHandle(file_handle);
}


#if 0
void linux_read_dir (char *notes_dir)
{
    DIR *dir = opendir(notes_dir);
    struct dirent *entity;
    
    // Contar la cantidad de elementos en el directorio
    if (dir != NULL)
    {
        for (list_state.cantidad = 0; (entity = readdir(dir)) != NULL; list_state.cantidad++)
        {
            if (entity->d_name[0] == '.') {
                list_state.cantidad--;
            }
        }
        
        rewinddir(dir);
        
        // Asignacion de memoria para la lista de notas
        list_state.note_list = (Nota *) malloc (sizeof (Nota) * list_state.cantidad);
        if (!note_list) {
            SDL_Log ("\nERROR: No se pudo asignar memoria para la lista de notas\n");
            exit (EXIT_FAILURE);
        }
        
        // Cambia al directorio de las notas
        if (!SetCurrentDirectory(notes_dir)) {
            SDL_Log ("Error cambiando el directorio\n");
            exit(EXIT_FAILURE);
        }
        
        // Inicializa el array de notas
        for (int i = 0; (entity = readdir(dir)) != NULL; i++)
        {
            if (entity->d_name[0] == '.') {
                i--;
                continue;
            }
            
            list_state.note_list[i].id = i;
            list_state.note_list[i].completed = false;
            list_state.note_list[i].edited = false;
            
            // Lectura del titulo
            list_state.note_list[i].title.length = text_length (entity->d_name, '.'); // Cuenta los caracteres sin la extension
            
            // Posicion inicial del cursor
            note_list[i].title.cursor_pos = note_list[i].title.length;
            
            note_list[i].title.size = note_list[i].title.length + 1;
            note_list[i].title.buffer = (char *) malloc (sizeof (char) * note_list[i].title.size);
            
            if (!list_state.note_list[i].title.buffer) {
                SDL_Log ("\nERROR: No se pudo asignar memoria para el titulo\n");
                exit (EXIT_FAILURE);
            }
            
            //Parsear el texto
            win32_file_parser (entity->d_name, &list_state.note_list[i]); 
            
            // Copia el titulo sin la extension
            string_copy (entity->d_name, list_state.note_list[i].title.buffer, list_state.note_list[i].title.length);
        }
        
        // Vuelve al directorio del ejecutable
        if (!SetCurrentDirectory("..")) {
            SDL_Log ("Error cambiando el directorio\n");
            exit(EXIT_FAILURE);
        }
        
        closedir (dir);
        
    } else {
        SDL_Log ("Error al abrir el directorio de notas\n");
        exit (EXIT_FAILURE);
    }
    
}
#endif


void verrmsg (int errnum, const char *fmt, va_list ap)
{
    if (fmt)
        vfprintf(stderr, fmt, ap);
    if (errnum != 0)
        fprintf(stderr, ": %s", strerror(errnum));
    putc('\n', stderr);
}



//
// Funciones de utileria
//


// deben ser del mismo largo
// el entero tiene que ser el largo del string (length)
int string_copy (char *source, char *dest, int length)
{
    int counter = 0;
    // TODO: Verificar los tamanos de los string y si son validos los punteros
    while (*source != '\0' && length > 0 && counter < length)
    {
        *dest++ = *source++;
        counter++;
    }
    
    *dest = '\0';
    
    return counter;
}

// Funcion para contar el largo de una cadena, 
// cuenta hasta el caracter nulo o encuentra el caracter de terminacion que se le pase, no  
// incluyendolo, por lo que hay que sumar 1 al reservar memoria.
int text_length(char *string, char term_char)
{
    int counter = 0;
    
    while (string != NULL && *string != '\0' && *string != term_char)
    {    
        counter++;
        string++;
    }
    return counter;
}

// Mas facil de usar
int text_length(char *string)
{
    return text_length (string, '\0');
}



int create_text_buffer(Text_buffer *buf, int size)
{
    if (buf != NULL && size > 0) {
        
        char *new_buffer = (char *) malloc (sizeof(char) * size);
        
        if (new_buffer) {
            
            buf->buffer = new_buffer;
            buf->size = size;
            buf->length = 0;
            
            buf->buffer[0] = '\0';
            
            return 1;
            
        } else return 0;
        
    } else {
        return 0;
    }
}

int resize_text_buffer (Text_buffer *buf, int new_size)
{
    if (buf != NULL && new_size > 0) {
        
        char *new_buffer = (char *) realloc (buf->buffer, sizeof(char) * new_size);
        
        if (new_buffer) {
            buf->buffer = new_buffer;
            buf->size = new_size;
            return 1;
            
        } else return 0;
        
    } else return 0;
}


// NOTE: mantiene el size y el buffer para permitir que se pueda volver a usar
int reset_text_buffer(Text_buffer *buf)
{
    if (buf) {
        buf->buffer[0] = '\0';
        buf->length = 0;
        
        return 1;
    } 
    
    return 0;
}


int delete_text_buffer(Text_buffer *buf)
{
    if (buf != NULL && buf->buffer != NULL) {
        free(buf->buffer);
        buf->length = 0;
        buf->size = 0;
        
        return 1;
    } else return 0;
}
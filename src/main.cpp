/*
* Lista de Tareas usando SDL
*/

#define ASSERT(x) if(!(x)) __debugbreak();

// windows
#include <windows.h>
#include <timezoneapi.h>
#include <sysinfoapi.h>

// i/o
#include <fileapi.h>
#include <handleapi.h>
#include <fcntl.h>
#include <io.h>

//Using SDL and standard IO
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef mingw
#include <dirent.h>   // Para manejar directorios en linux
#endif


#define IMGUI_SRC_ID 1

#define RECTCUT_SDL
#include "GUI.h"

#include "tareas.h"   // Estructuras de datos y defines

#define FPS 60
#define FRAME_TARGET_TIME (1000.0f / FPS)

//----------------------------------------------
// Global variables declaration
//---------------------------------------------

char* base_path = NULL;

// Necesario para fix timestep
float last_frame_time = 0;

bool entering_text = false;

//Screen dimension constants
const int SCREEN_WIDTH_MIN = 600;
const int SCREEN_HEIGHT_MIN = 600;

int actual_window_width = SCREEN_WIDTH_MIN;
int actual_window_height = SCREEN_HEIGHT_MIN;


// TODO: No se si es necesario...
char MY_DATA_DIR[MAX_PATH];
char LISTS_DIR[MAX_PATH];


// Initialize current screen
GUI::screens current_screen = GUI::LISTAS;

SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;

GUI::Texture text_texture                    = {NULL, {0, 0, 0, 0}}; 
GUI::Texture menu_button_texture             = {NULL, {0, 0, 0, 0}};
GUI::Texture add_button_texture              = {NULL, {0, 0, 0, 0}};
GUI::Texture search_texture                  = {NULL, {0, 0, 0, 0}};
GUI::Texture back_button_texture             = {NULL, {0, 0, 0, 0}};
GUI::Texture delete_button_texture           = {NULL, {0, 0, 0, 0}};
GUI::Texture close_button_texture            = {NULL, {0, 0, 0, 0}};
GUI::Texture check_box_texture               = {NULL, {0, 0, 0, 0}};
GUI::Texture check_box_outline_blank_texture = {NULL, {0, 0, 0, 0}};


GUI::UIstate uistate = {0};

GUI::General_Style style;

// Esta es la representacion de la lista del menu princiapal,
// donde envez de una lista de punteros a tareas apunta a otras tasks_list.
// Esto esta mal pero es lo unico que se me ocurre ahora para no tener
// que reescribir toda la app.
Main_List main_list = { 0 };

GUI::List_Theme  default_list_theme = { 15, 1, 64, 0, .5f, .5f, 0 };

// Tema de lista para una lista de tareas
GUI::List_Theme check_list_theme = { 15, 10, 50, 0, 1.0f, 0, 0, 0 };

SDL_Rect layout_notas[GUI::NUM_ELEMENTOS];

Task_List *list_pressed = NULL;
Task_List new_list;


struct animated_menu_state {
    bool show = false;
    bool animando = false;
    SDL_Rect rect = {0};
} left_menu;


//Event handler
SDL_Event event;

bool is_running = false;

// Cosas del campo de busqueda (text_field)
char hint_search_text[] = "Buscar..."; 
Text_buffer search_input_text; 

// Cosas del campo de agregar tareas
char hint_add_text[] = "Agregar tarea..."; 
Text_buffer add_input_text; 

// Cosas del campo de agregar lista
char hint_add_list[] = "Nombre de la nueva Lista..."; 
Text_buffer add_list_input_text; 


// TODO: Para la optimizacion del texto, pendiente...
bool renderText = false;
bool resize = false;        // Es necesario recalcular el layout
bool should_reinit = false;

float delta_time = 0.0f;

bool show_menu = false;
bool menu_pressed = false;
bool back_pressed = false;


//Loads media
bool load_media();
bool load_texture( char *path, GUI::Texture *texture);

// Renderiza toda la aplicacion
void render_one_frame(int screen_width, int screen_height);

void update();
bool proccess_main_input();
bool my_init_fonts();   // Inicializa las fuente y los iconos
bool ui_init();         // Inicializa el layout de la interfaz
void close_sdl();


int main( int argc, char* argv[] )
{
    //Start up SDL and create window
    if( !init() )
    {
        SDL_Log("Failed to initialize!\n" );
    } else {
        
        if( !load_media() )
        {
            SDL_Log("Failed to load media!\n" );
        } else {
            
            is_running = true;
            
            // Creacion de los buffers de texto
            create_text_buffer(&search_input_text, 40);
            create_text_buffer(&add_input_text, 40);
            create_text_buffer(&add_list_input_text, 40);
            
            
            //Enable text input
            //SDL_StartTextInput();
            
            while (is_running)
            {
                proccess_main_input();
                
                update();
                
                // if (entering_text) GUI::proccess_search_input_text_events(&event);
                
                // Comienza a dibujar
                
                render_one_frame(actual_window_width, actual_window_height);
                
                // Termina de dibujar
            }
            
        }
    }
    
    //Disable text input
    //SDL_StopTextInput();
    
    //Free resources and close SDL
    close_sdl();
    
    return 0;
}

bool proccess_main_input()
{ 
    // TODO:
    //while (SDL_WaitEvent(&event))
    while (SDL_PollEvent(&event))
    {
        
        //User requests quit
        if( event.type == SDL_QUIT )
        {
            return (is_running = false);
        }
        
        if (event.type == SDL_KEYDOWN)
        {
            if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                return (is_running = false);
            
            if (event.key.keysym.scancode == SDL_SCANCODE_F1)
                current_screen = GUI::MENU_PRINCIPAL;
            
            if (event.key.keysym.scancode == SDL_SCANCODE_F2)
                show_menu = !show_menu;
            
            if (event.key.keysym.scancode == SDL_SCANCODE_F3)
                current_screen = GUI::TAREAS;
            
            if (event.key.keysym.scancode == SDL_SCANCODE_TAB)
                uistate.key_entered = event.key.keysym.scancode;
            
            uistate.keymod = event.key.keysym.mod;
            
            if (event.key.keysym.scancode == SDL_SCANCODE_RETURN)
                uistate.key_entered = event.key.keysym.scancode;
            
            if (event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE)
                uistate.key_entered = event.key.keysym.scancode;
        }
        
        // Eventos de la ventana
        if (event.type == SDL_WINDOWEVENT)
        {
            switch (event.window.event)
            {
                // El primer evento
                SDL_WINDOWEVENT_SHOWN:
                {
                    SDL_Log("SDL_WINDOWEVENT_SHOWN");
                    
                } break;
                
                case SDL_WINDOWEVENT_RESIZED:
                {
                    SDL_Log("Window %d resize to %dx%d\n", event.window.windowID,
                            event.window.data1, event.window.data2);
                    
                    resize = true;
                    
                } break;
                
                //Get new dimensions and repaint on window size change
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                {
                    // TODO: Recalcular el layout
                    resize = true;
                    
                    SDL_Log("Window %d resize to %dx%d\n", event.window.windowID, event.window.data1, event.window.data2);
                    
                    
                    should_reinit = (event.window.data1 != actual_window_width) || (event.window.data2 != actual_window_height);
                    
                    // Guarda la nueva resolucion
                    actual_window_width = event.window.data1;
                    actual_window_height = event.window.data2;
                    
                    if (should_reinit) 
                    {
                        ui_init();          // Recalcula el layout de la pantalla
                        my_init_fonts();    // Resize the font for the new window size
                    }
                    
                    render_one_frame(actual_window_width, actual_window_height);
                    
                } break;
                
                //Repaint on exposure: just means that window was obscured in some way and now is not obscured so we want to 
				// repaint the window. 
                case SDL_WINDOWEVENT_EXPOSED:
                {
                    SDL_Log("SDL_WINDOWEVENT_EXPOSED");
                } break;
                
				// Window minimized
                // we make sure to only render when the window is not minimized because this can cause some bugs when we try to 
				// render to a minimized window.
                case SDL_WINDOWEVENT_MINIMIZED:
                {
                    SDL_Log("SDL_WINDOWEVENT_MINIMIZED");//Minimized = true;
                } break;
                
                //Window maxized
                case SDL_WINDOWEVENT_MAXIMIZED:
                {
                    SDL_Log("SDL_WINDOWEVENT_MAXIMIZED");//Minimized = false;
                } break;
                
                //Window restored  (deja de estar minimizada)
                case SDL_WINDOWEVENT_RESTORED:
                {
                    SDL_Log("SDL_WINDOWEVENT_RESTORED"); //Minimized = false;
                } break;
            }
        }
        
        if (event.type == SDL_MOUSEBUTTONDOWN)
        {
            if (event.button.button & SDL_BUTTON_LEFT) uistate.mouse_down = 1;
        }
        
        if (event.type == SDL_MOUSEBUTTONUP)
        {
            if (event.button.button & SDL_BUTTON_LEFT) uistate.mouse_down = 0;
        }
        
        // List Scrolling
        if(event.type == SDL_MOUSEWHEEL)
        {
            if(event.wheel.y > 0) // scroll up
            {
                // handling "scroll up"
                uistate.mouse_wheel = event.wheel.y;
            }
            else if(event.wheel.y < 0) // scroll down
            {
                // handling "scroll down"
                uistate.mouse_wheel = event.wheel.y;
            }
        }
        
    }
    
    return true;
    
}

void update()
{
    // timestep fijo a 60 fps
    
    int current_time = SDL_GetTicks();
    
    int time_to_wait = FRAME_TARGET_TIME - (current_time - last_frame_time);
    
#if 1
    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) 
        SDL_Delay(time_to_wait);
#endif
    
    delta_time = (current_time - last_frame_time) / 1000.0f;
    
    last_frame_time = current_time;
}


void render_one_frame(int screen_width, int screen_height)
{
    //Clear screen
    SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
    SDL_RenderClear( gRenderer );
    
    GUI::imgui_prepare(&uistate);
    
    switch (current_screen) {
        
        case GUI::LISTAS:
        {                
            
            // Dibuja la lista en el panel
            if (main_list.cantidad_listas > 0) {
                GUI::draw_main_list (&layout_notas[GUI::LIST_PANEL], &event, &default_list_theme, &main_list);
            } else {
                GUI::draw_label(&layout_notas[GUI::LIST_PANEL], "No hay listas de tareas", style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            }
            
            // Dibuja el fondo de la tittle_bar
            SDL_SetRenderDrawColor( gRenderer, style.background_color.r, style.background_color.g, style.background_color.b, 0xFF );
            SDL_RenderFillRect (gRenderer, &layout_notas[GUI::TITLE_BAR]);
            
            // Dibuja el texto de la tittle bar
            GUI::draw_label (&layout_notas[GUI::TITLE_BAR], "Listas de Tareas", style.font_color, GUI::BOLD, GUI::CENTER, GUI::CENTER, 0);
            
            // Fondo de la barra de busqueda
            SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
            SDL_RenderFillRect (gRenderer, &layout_notas[GUI::BUSQUEDA]);
            
            
            // Boton de agregar lista
            if (GUI::draw_boton(GEN_ID, &layout_notas[GUI::RIGHT_BUTTON], 
                                NULL, &add_button_texture, &event))
            {
                list_pressed = &new_list;
                current_screen = GUI::LIST_EDITOR;
            }
            
            
            // Icono de busqueda
            if (GUI::draw_boton(GEN_ID, &layout_notas[GUI::SEARCH_BUTTON], NULL, &search_texture, &event));
            
            
            // TODO: buscar
            // ingreso de texto (text_input)
            GUI::text_field(GEN_ID, &layout_notas[GUI::BUSQUEDA], &search_input_text, hint_search_text);
            
            //GUI::draw_boton(GEN_ID, &layout_notas[GUI::BUSQUEDA], text_buffer, NULL, &event);
            
            
            // Icono de cancelar la busqueda
            if (search_input_text.length > 0) {
                
                if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::CANCEL_BUTTON], NULL, &close_button_texture, &event))
                {
                    reset_text_buffer(&search_input_text);
                    
                    reset_search(&main_list);
                }
                
                search_list(&main_list, search_input_text.buffer);
                
            } else if (search_input_text.length == 0) {
                reset_search(&main_list);
            }
            
            
            
            //TODO: Dibuja el campo de busqueda
            
            //Rerender text if needed
            if ( renderText )
            {
                //Text is not empty
                if( search_input_text.buffer != "" )
                {
                    //Render new text
                    //gInputTextTexture.loadFromRenderedText( inputText.c_str(), textColor );
                }
                //Text is empty
                else
                {
                    //Render space texture
                    //gInputTextTexture.loadFromRenderedText( " ", textColor );
                }
            }
            
            // Dibuja el separador de la tittle_bar
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            
            SDL_RenderDrawLine( gRenderer, 0, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h, actual_window_width, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h);
            
            // Dibuja el separador de la barra de busqueda
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            
            // Dibuja el fondo de la barra inferior
            SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
            SDL_RenderFillRect (gRenderer, &layout_notas[GUI::BOTTOM_BAR]);
            
            //Draw blue horizontal line
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            SDL_RenderDrawLine( gRenderer, layout_notas[GUI::BOTTOM_BAR].x, layout_notas[GUI::BOTTOM_BAR].y, layout_notas[GUI::BOTTOM_BAR].x + layout_notas[GUI::BOTTOM_BAR].w, layout_notas[GUI::BOTTOM_BAR].y);
            
            
            
            // Muestra la resolucion actual de la ventana (DEBUG)
            if (resize)
            {
                char buffer[64];
                sprintf (buffer, "%d x %d", actual_window_width, actual_window_height);                      
                
                GUI::draw_label (&layout_notas[GUI::BOTTOM_BAR], buffer, style.font_color, GUI::BOLD, GUI::CENTER, GUI::CENTER, 0);
            }
            
			
            
            // Dibuja el boton de menu
            if (menu_pressed)
            {
                show_menu = true;
            }
            
            if (back_pressed)
            {
                show_menu = false;
                back_pressed = false;
            }
            
            if (!show_menu)
            {
                if (left_menu.rect.w > 0)
                    GUI::contract_toward(&left_menu.rect, 0, delta_time, .15);
                
				SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
				SDL_RenderFillRect (gRenderer, &left_menu.rect);
                
                
                menu_pressed = GUI::draw_boton (GEN_ID, &layout_notas[GUI::LEFT_BUTTON], NULL, &menu_button_texture, &event);
            }
            else {
                SDL_SetRenderDrawColor(gRenderer, 0x21, 0x25, 0x29, 0xFF);
				
                GUI::expand_toward(&left_menu.rect, 
                                   layout_notas[GUI::LAYOUT_PRINCIPAL].w * .5, 
                                   delta_time, .15);
                
                SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
				SDL_RenderFillRect (gRenderer, &left_menu.rect);
                
                back_pressed = GUI::draw_boton (GEN_ID, &layout_notas[GUI::LEFT_BUTTON], NULL, &back_button_texture, &event);
            }
            
        } break;
        
        
        // TODO: Hacer editor de lista y editor de task
        case GUI::LIST_EDITOR:
        {
            //Task_List *list_pressed = &main_list.listas[main_list.item_pressed];
            
            if (list_pressed == NULL)
            {
                current_screen = GUI::LISTAS;
                return;
            }
            
            GUI::draw_label (&layout_notas[GUI::CRE_TEXT], "Crear una nueva lista", style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            
            // Separador
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            
            SDL_RenderDrawLine( gRenderer, 0, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h, actual_window_width, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h);
            
            
            // ingreso de texto (text_input)
            GUI::text_field(GEN_ID, &layout_notas[GUI::NOTE_TITLE], &add_list_input_text, hint_add_list);
            
            // boton volver
            if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::LEFT_BUTTON], NULL, &back_button_texture, &event))
            {
                // TODO: Incompleto
                if (add_list_input_text.length > 0 &&
                    check_list_title_available(&main_list, add_list_input_text.buffer)) {
                    
                    // TODO: 
                    
                    create_new_list(&main_list, &add_list_input_text);
                    reset_text_buffer(&add_list_input_text);
                    
                    current_screen = GUI::LISTAS;
                } else {
                    // TODO: Deberia indicar porque no se puede guardar...
                    ;
                }
                
            }
            
            // boton de borrar lista
            if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::RIGHT_BUTTON], NULL,
                                 &delete_button_texture, &event))
            {
                // TODO: Solicitar confirmacion
                reset_text_buffer(&add_list_input_text);
                current_screen = GUI::LISTAS;
            }
            
        } break;
        
        
        // Muestra las tareas que componene la lista
        case GUI::TAREAS:
        {
            using namespace GUI;
            
            Task_List* list_pressed = &main_list.listas[main_list.item_pressed];
            
            // Separador 
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            
            SDL_RenderDrawLine( gRenderer, 0, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h, actual_window_width, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h);
            
            // Dibuja la lista de tareas
            if (list_pressed->cantidad_tareas > 0) {
                if (GUI::draw_list (&layout_notas[GUI::TASKS_LIST], &event, &check_list_theme, list_pressed)) {
                    save_list_to_file(list_pressed);
                }
            } else {
                GUI::draw_label (&layout_notas[GUI::TASKS_LIST], "Aun no hay tareas en esta lista", style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            }
            
            // Dibuja el separador
            SDL_SetRenderDrawColor( gRenderer, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a );
            SDL_RenderFillRect(gRenderer, &layout_notas[GUI::SEPARADOR_INPUT_LIST]);
            
            
            // Dibuja el campo de ingreso de nueva tarea
            if (GUI::text_field(GEN_ID, &layout_notas[GUI::ADD_TASK_INPUT], &add_input_text, hint_add_text))
            {
                create_new_task(list_pressed, &add_input_text);
                reset_text_buffer(&add_input_text);
                
                // TODO:
                save_list_to_file(list_pressed);
                
            }
            
            // boton volver
            if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::LEFT_BUTTON], NULL, &back_button_texture, &event))
            {
                reset_text_buffer(&add_input_text);
                current_screen = GUI::LISTAS;
            }
            
            // boton de borrar la lista de tareas
            if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::RIGHT_BUTTON], NULL,
                                 &delete_button_texture, &event)) {
                
                //SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Eliminar", "Eliminar", gWindow);
                
                
                // Ventana de confirmacion de eliminar lista
                
                SDL_MessageBoxButtonData botones[2] = {
                    {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 1, "Eliminar"}, 
                    {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancelar"}
                };
                
                SDL_MessageBoxData msg_data = {SDL_MESSAGEBOX_WARNING, NULL, 
                    "Eliminar lista", "Seguro que quieres eliminar la lista?", 2, botones,
                    NULL};
                
                int button_id = 0;
                
                SDL_ShowMessageBox(&msg_data, &button_id);
                
                // Presiono Eliminar
                if (button_id == 1 && delete_list(list_pressed)) {
                    current_screen = GUI::LISTAS;
                }
                
                // Canceler (Hacer Nada)
            }
            
            // Muestra la fecha de creacion y edicion del archivo
            int w;
            char buffer[100];
            sprintf (buffer, "Creada: %d:%d:%d %d/%d/%d",
                     list_pressed->CreationTime.wHour, list_pressed->CreationTime.wMinute,
                     list_pressed->CreationTime.wSecond, list_pressed->CreationTime.wDay,
                     list_pressed->CreationTime.wMonth, list_pressed->CreationTime.wYear);
            
            TTF_SizeText(style.bold_font, buffer, &w, NULL);
            
            // Draw creation text
            GUI::draw_label (&layout_notas[GUI::CREATED_TEXT], buffer, style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            
            sprintf (buffer, "Modificada: %d:%d:%d %d/%d/%d",
                     list_pressed->LastWriteTime.wHour, list_pressed->LastWriteTime.wMinute,
                     list_pressed->LastWriteTime.wSecond, list_pressed->LastWriteTime.wDay,
                     list_pressed->LastWriteTime.wMonth, list_pressed->LastWriteTime.wYear);
            
            // Draw modification text
            GUI::draw_label (&layout_notas[GUI::MODIF_TEXT], buffer, style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            
            GUI::draw_label (&layout_notas[GUI::LISTA_NAME], list_pressed->title.buffer, style.font_color, GUI::NORMAL_14, GUI::CENTER, GUI::CENTER, 0);
            
            
        } break;
        
        default:
        case GUI::MENU_PRINCIPAL:
        {
            // Menu desde donde acceder a todas las demas pantallas
            
            // Separador
            SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
            
            SDL_RenderDrawLine( gRenderer, 0, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h, actual_window_width, layout_notas[GUI::TITLE_BAR].y + layout_notas[GUI::TITLE_BAR].h);
            
            // boton volver
            if (GUI::draw_boton (GEN_ID, &layout_notas[GUI::LEFT_BUTTON], NULL,
                                 &back_button_texture, &event))
                current_screen = GUI::LISTAS;
            
            
            GUI::draw_menu_principal(layout_notas[GUI::OPCIONES_PANEL], &event);
            
        } break;
        
    }
    
    GUI::imgui_finish(&uistate);
    
    SDL_RenderPresent( gRenderer ); //Update screen
}


bool init ()
{
    // initialization flag
    bool success = true;
    
    
    //initialize SDL
    if (SDL_Init( SDL_INIT_VIDEO ) < 0)
    {
        SDL_Log ("SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    } else {
        //Create window
        gWindow = SDL_CreateWindow( "Listas de Tareas (SDL)", 
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                   SCREEN_WIDTH_MIN, SCREEN_HEIGHT_MIN, SDL_WINDOW_SHOWN );
        
        if( gWindow == NULL )
        {
            SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
            
        } else {
            
            // Permite redimensionar la ventana
            SDL_SetWindowResizable(gWindow, SDL_TRUE);
            
            SDL_SetWindowMinimumSize(gWindow, SCREEN_WIDTH_MIN, SCREEN_HEIGHT_MIN);
            
            //Get window surface
            gRenderer = SDL_CreateRenderer (gWindow, -1, SDL_RENDERER_ACCELERATED);
            
            if (gRenderer == NULL)
            {
                SDL_Log("Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            } else {
                //Initialize renderer color
                //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
                SDL_RenderClear( gRenderer );
                
                
                //Update screen
                SDL_RenderPresent( gRenderer );
                
                //Initialize PNG loading
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    SDL_Log ("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                
                //Initialize SDL_ttf
                if( TTF_Init() == -1 )
                {
                    SDL_Log ("SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
            }
        }
    }
    
    
    // Inicializa la aplicacion
    base_path = SDL_GetBasePath();
    
    if (base_path)
    {
        // Crea el path del DATA
        strncpy(MY_DATA_DIR, base_path, MAX_PATH);
        strncat(MY_DATA_DIR, "data", 4);
        
        
        // Crea el path hacia las listas de tareas
        strncpy(LISTS_DIR, MY_DATA_DIR, MAX_PATH);
        strncat(LISTS_DIR, "\\listas", 8);
    }
    else {
        strncpy(MY_DATA_DIR, "data", 4);
        strncpy(MY_DATA_DIR, "data/listas", 11);
    }
    
    OutputDebugStringA(MY_DATA_DIR);
    OutputDebugStringA("\n");
    OutputDebugStringA(LISTS_DIR);
    OutputDebugStringA("\n");
    
    SDL_Log(MY_DATA_DIR);
    SDL_Log(LISTS_DIR);
    
    
    //linux_read_dir(default_notes_dir);
    
    // NOTE: Solo para Windows
    win32_read_dir (LISTS_DIR, &main_list);
    
    
    ui_init();
    
    
    SDL_Log("Se inicio correctamente\n");
    
    return success;
}

// Carga todas la imagenes (texturas) que vamos a usar.
bool load_media ()
{
    //Loading success flag
    bool success = true;
    
    success = my_init_fonts();
    
    //Load SVG texture
    if (!load_texture("data/assets/menu-white-24dp.svg", &menu_button_texture)) 
        success = false;
    
    if (!load_texture("data/assets/add-white-24dp.svg", &add_button_texture)) 
        success = false;
    
    if (!load_texture("data/assets/search_white_24dp.svg", &search_texture)) 
        success = false;
    
    if (!load_texture("data/assets/arrow_back-white-24dp.svg", &back_button_texture)) 
        success = false;
    
    if (!load_texture("data/assets/delete_white_24dp.svg", &delete_button_texture)) 
        success = false;
    
    if (!load_texture("data/assets/close-white-24dp.svg", &close_button_texture)) 
        success = false;
    
    if (!load_texture("data/assets/check_box_white_24dp.svg", &check_box_texture))
        success = false;
    
    if (!load_texture("data/assets/check_box_outline_blank_white_24dp.svg", &check_box_outline_blank_texture)) 
        success = false;
    
    return success;
}

bool load_texture( char *path, GUI::Texture *texture )
{
    bool success  = true;
    
    // Carga la textura desde el path
    SDL_Surface *loadedSurface = IMG_Load( path );
    
    if( loadedSurface == NULL )
    {
        SDL_Log ("Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
        
        success = false;
        
    } else {
        texture->texture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( texture->texture == NULL )
        {
            SDL_Log ( "Unable to create texture from %s! SDL Error: %s\n", path, SDL_GetError() );
            
            success = false;
        }
        
        texture->rect.w = loadedSurface->w;
        texture->rect.h = loadedSurface->h;
        
        SDL_FreeSurface( loadedSurface );
    }
    
    return success;
}


// Inicializa la interfaz de usuario y la actualiza cuando cambia la resolucion
bool ui_init()
{
    init_general_theme(&style);
    
    // Crea el layout de la aplicacion
    
    layout_notas[GUI::LAYOUT_PRINCIPAL] = {0, 0, 
        actual_window_width, actual_window_height};
    
    
    using namespace GUI;
    
    // Barra principal
    layout_notas[TITLE_BAR]   = cut_top (&layout_notas[LAYOUT_PRINCIPAL], 42);
    layout_notas[LEFT_BUTTON] = cut_left (&layout_notas[TITLE_BAR], 64);
    layout_notas[RIGHT_BUTTON]  = cut_right(&layout_notas[TITLE_BAR], 64);
    
    
    // Lista
    layout_notas[LAYOUT_LISTA] = layout_notas[LAYOUT_PRINCIPAL];
    // Barra de busqueda
    layout_notas[BUSQUEDA]       = cut_top   (&layout_notas[LAYOUT_LISTA], 42);
    layout_notas[SEARCH_BUTTON]  = cut_left  (&layout_notas[BUSQUEDA], 64);
    layout_notas[CANCEL_BUTTON]  = cut_right (&layout_notas[BUSQUEDA], 64);
    
    // Barra inferior
    layout_notas[BOTTOM_BAR] = cut_bottom (&layout_notas[LAYOUT_LISTA], 42);
    
    // Panel de la lista
    layout_notas[LIST_PANEL] = layout_notas[LAYOUT_LISTA];
    
    
    //
    // Layout de la ventana de tareas
    //
    layout_notas[LAYOUT_TAREAS] = layout_notas[LAYOUT_PRINCIPAL];
    layout_notas[LISTA_NAME] = layout_notas[TITLE_BAR];
    layout_notas[CREATED_TEXT] = cut_top(&layout_notas[LAYOUT_TAREAS], 20);
    layout_notas[MODIF_TEXT] = cut_top(&layout_notas[LAYOUT_TAREAS], 20);
    
    // Margen izquierdo
    cut_left (&layout_notas[LAYOUT_TAREAS], 32);
    
    // Margen derecho
    cut_right (&layout_notas[LAYOUT_TAREAS], 32);
    
    // Separacion del texto de creacion y modificacion
    cut_top(&layout_notas[LAYOUT_TAREAS], 20);
    
    // Entrada de texto para agregar mas tareas
    layout_notas[ADD_TASK_INPUT] = cut_top(&layout_notas[LAYOUT_TAREAS], 42);
    
    // Separacion del input
    layout_notas[SEPARADOR_INPUT_LIST] = cut_top(&layout_notas[LAYOUT_TAREAS], 20);
    
    layout_notas[TASKS_LIST] = layout_notas[LAYOUT_TAREAS];
    
    
    //
    // Layout del editor
    //
    layout_notas[LAYOUT_EDITOR] = layout_notas[LAYOUT_PRINCIPAL];
    layout_notas[CRE_TEXT] = cut_top(&layout_notas[LAYOUT_EDITOR], 20);
    layout_notas[MOD_TEXT] = cut_top(&layout_notas[LAYOUT_EDITOR], 20);
    
    
    // Margen izquierdo
    cut_left (&layout_notas[LAYOUT_EDITOR], 54);
    
    // Margen derecho
    cut_right (&layout_notas[LAYOUT_EDITOR], 54);
    
    // Separacion del titulo
    cut_top(&layout_notas[LAYOUT_EDITOR], 20);
    // Titulo de la nota
    layout_notas[NOTE_TITLE] = cut_top(&layout_notas[LAYOUT_EDITOR], 42);
    // Separacion del titulo
    cut_top(&layout_notas[LAYOUT_EDITOR], 10);
    
    
    //
    // Layout del Menu Principal
    //
    
    layout_notas[LAYOUT_MENU] = layout_notas[LAYOUT_PRINCIPAL];
    // Margen izquierdo
    cut_left (&layout_notas[LAYOUT_MENU], 54);
    
    // Margen derecho
    cut_right (&layout_notas[LAYOUT_MENU], 54);
    
    cut_top(&layout_notas[LAYOUT_MENU], 40);
    
    layout_notas[OPCIONES_PANEL] = layout_notas[LAYOUT_MENU];
    
    
    
    
    // TODO: Temporal
    // Layout del menu izquierdo
    left_menu.rect.x = 0;
    left_menu.rect.y = 0;
    left_menu.rect.w = actual_window_width * .5;
    left_menu.rect.h = actual_window_height;
    
    return true;
}


// Inicializa las fuentes, se llama al principio o si cambia la resolucion de la pantalla
// (TODO: para adaptar la fuente a los dpi)
// TODO: Deberia hacer esto tambien para los iconos
bool my_init_fonts()
{
    bool success = true;
    
    // Una forma de adaptar el tamano de la fuenta dependiendo de la resolucion de la pantalla
    // FIX: Cuando se usa la actual, no se ajusta bien cuando la ventana crece y
    // da algunos problemas de flickering en alguos textos
    int altura_fuente_pequena = SCREEN_HEIGHT_MIN / 40;
    int altura_fuente_grande = SCREEN_HEIGHT_MIN / 30;
    //int altura_fuente_pequena = actual_window_height / 40;
    //int altura_fuente_grande = actual_window_height / 30;
    
    // Si se cargaron las fuentes anteriormente, libera la memoria
    if (style.normal_font != NULL || style.normal_font_2 != NULL || style.bold_font != NULL)
    {
        TTF_CloseFont( style.normal_font );
        TTF_CloseFont( style.normal_font_2 );
        TTF_CloseFont( style.bold_font );
        style.normal_font = NULL;
        style.normal_font_2 = NULL;
        style.bold_font = NULL;
    }
    
    //Open the fonts
    // 16
    style.normal_font = TTF_OpenFont( "data/assets/SourceCodePro-Regular.ttf", altura_fuente_grande );
    
    // 14
    style.normal_font_2 = TTF_OpenFont( "data/assets/SourceCodePro-Regular.ttf", altura_fuente_pequena );
    
    // 18
    style.bold_font = TTF_OpenFont( "data/assets/SourceCodePro-Semibold.ttf", altura_fuente_grande );
    
    
    if( style.normal_font == NULL || style.bold_font == NULL || style.normal_font_2 == NULL)
    {
        SDL_Log ("Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
        
    } else {
        SDL_Log ("Fonts loaded successfuly\n");
    }
    
    
    return success;
}


// TODO: Falta agregar varias cosas aqui...
void close_sdl()
{
    //Free global font
    TTF_CloseFont( style.normal_font );
    TTF_CloseFont( style.normal_font_2 );
    TTF_CloseFont( style.bold_font );
    style.normal_font = NULL;
    style.normal_font_2 = NULL;
    style.bold_font = NULL;
    
    SDL_DestroyRenderer (gRenderer);
    
    //Destroy window
    SDL_DestroyWindow( gWindow );
    
    gWindow = NULL;
    gRenderer = NULL;
    
    TTF_Quit();
    IMG_Quit();
    
    //Quit SDL subsystems
    SDL_Quit();
}
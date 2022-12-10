// Funciones que utilizan todos los demas elementos de la GUI

#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>

#include "GUI.h"

#define IMGUI_SRC_ID 4

extern const int SCREEN_WIDTH_MIN;
extern const int SCREEN_HEIGHT_MIN;
extern GUI::Texture text_texture;
extern GUI::screens current_screen;
extern GUI::Texture check_box_texture;
extern GUI::Texture check_box_outline_blank_texture;

extern GUI::UIstate uistate;

extern GUI::General_Style style;

extern SDL_Renderer *gRenderer;

extern char hint_search_text[];


namespace GUI {
    
    static bool interacting = false;
    
    //
    // Estilos 
    // TODO: Terminar los estilos
    
#if 0
    // Colores
    SDL_Color BACKGROUND_COLOR    = {0x21, 0x25, 0x29, 0xFF};
    SDL_Color FONT_COLOR          = {0xF1, 0xF3, 0xF5, 0xFF};             // Text color GRAY 1
    SDL_Color FONT_COLOR_2        = {0x8C, 0x8F, 0x94, 0xFF};
    SDL_Color FONT_COLOR_WHITE    = {0xFF, 0xFF, 0xFF, 0xFF};             // WHITE
    SDL_Color BUTTON_COLOR        = {0x49, 0x50, 0x57, 0xFF};             // GRAY 7
    SDL_Color OVER_BUTTON_COLOR   = {0x86, 0x8E, 0x96, 0xFF};             // GRAY 6
    SDL_Color TEXT_BOX_COLOR      = {0x34, 0x3A, 0x40, 0xFF};             // GRAY 8
    SDL_Color ICON_COLOR          = {0x8C, 0x8F, 0x94, 0xFF};
    SDL_Color RED_COLOR           = {0xFF, 0x00, 0x00, 0xFF};
#endif
    
    // Tener una funcion GetColor() que retorne el color que se le pida
    // Revisar esto: https://github.com/ocornut/imgui/blob/fa2b318dd6190852a6fe7ebc952b6551e93899e0/imgui.cpp#L2726
    void init_general_theme(General_Style* style)
    {
        style->background_color  = {0x21, 0x25, 0x29, 0xFF};
        
        style->font_color        = {0xF1, 0xF3, 0xF5, 0xFF};
        style->font_color_2      = {0x8C, 0x8F, 0x94, 0xFF};
        style->font_color_white  = {0xFF, 0xFF, 0xFF, 0xFF};
        
        style->button_color      = {0x49, 0x50, 0x57, 0xFF};
        style->over_button_color = {0x86, 0x8E, 0x96, 0xFF};
        
        style->text_box_color    = {0x34, 0x3A, 0x40, 0xFF};
        style->icon_color        = {0x8C, 0x8F, 0x94, 0xFF};
        style->red_color         = {0xFF, 0x00, 0x00, 0xFF};
        
    }
    
    
    void imgui_prepare(struct UIstate *state)
    {
        state->hot_item = 0;
    }
    
    void imgui_finish(struct UIstate *state)
    {
        // Si el mouse no esta en click, tenemos que limpiar el item activo para que los
        // widgets no se confundan en el estado activo
        // (Y permitir que el siguiente widget al que se le haga click pase a estar activo)
        
        if (state->mouse_down == 0)
        {
            state->active_item = 0;
        } else {
            if (state->active_item == 0)
                state->active_item = -1;
        }
        
        //if no widget grabbed tab, clear focus
        if (state->key_entered == SDLK_TAB) state->keyboard_focus = 0;
        
        // Clear the entered key (TAB)
        state->key_entered = 0;
        
        state->mouse_wheel = 0;
    }
    
    
    // TODO: El corte del texto no es muy bonito
    // FIX: Comprobar si funciona con cuando el texto esta vacio...
    bool draw_label (SDL_Rect *rect, char *text, SDL_Color textColor, FONT_TYPE tipo_fuente,
                     ALIGN horizontal_align, ALIGN vertical_align, int desplazamiento)
    {
        SDL_Surface *text_surface = NULL;
        SDL_Rect tex_sector = {0};
        
        int cantidad_caracteres = 0;
        
        // Selecciona la fuente (por defecto es la BOLD)
        TTF_Font *label_font = style.bold_font;
        
        if (tipo_fuente == NORMAL) label_font = style.normal_font;
        else if (tipo_fuente == NORMAL_14) label_font = style.normal_font_2;
        
        //Get rid of preexisting texture
        if (text_texture.texture != NULL) {
            SDL_DestroyTexture( text_texture.texture );
            text_texture.texture = NULL;
            text_texture.rect.w = 0;
            text_texture.rect.h = 0;
            text_texture.rect.x = 0;
            text_texture.rect.y = 0;
        }
        
        // cuantos caracteres se pueden renderizar en el espacio
        TTF_MeasureText(label_font, text, rect->w, NULL, &cantidad_caracteres);
        
        // Si no hay texto (para evitar acceso a memoria no asignada)
        if (!text) return false;
        
        if (*text == '\0')
            text = " ";
        
        text_surface = TTF_RenderText_Blended(label_font, text, textColor);
        
        
        if( text_surface == NULL ) {
            SDL_Log ("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
            
        } else {
            //Create texture from surface pixels
            text_texture.texture = SDL_CreateTextureFromSurface( gRenderer, text_surface );
            
            if( text_texture.texture == NULL )
            {
                SDL_Log ("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
                
                return false;
                
            } else {
                //Get image dimensions
                
                text_texture.rect.w = text_surface->w;
                text_texture.rect.h = text_surface->h;
                
                text_texture.rect.x = rect->x + desplazamiento;
                text_texture.rect.y = rect->y;
                
                //
                // Alineacion del texto
                //
                
                // Centrado verticalmente
                if (vertical_align == CENTER)
                    text_texture.rect.y = rect->y + rect->h * .5 - text_texture.rect.h * .5;
                
                // Centrado horizontalmente
                if (horizontal_align == CENTER) {
                    
                    text_texture.rect.x = (rect->x + desplazamiento) + (rect->w - desplazamiento) / 2 - text_texture.rect.w / 2;
                    
                } else if (horizontal_align == RIGHT) {
                    text_texture.rect.x = (rect->x + desplazamiento) + (rect->w - desplazamiento) - text_texture.rect.w;
                }
            }
            
            // DEBUG
            //SDL_SetRenderDrawColor( gRenderer, 0x86, 0x8E, 0x96, 0xFF );
            //SDL_RenderDrawRect (gRenderer, &length);
            
            //Get rid of old surface
            SDL_FreeSurface( text_surface );
        }
        
        //Render font texture to screen
        
        // Si la textura del texto cabe completamente en el rect
        
        if (text_texture.rect.w <= rect->w)
            SDL_RenderCopy( gRenderer, text_texture.texture, NULL, &text_texture.rect );
        
        else if (text_surface->w > rect->w)
        {
            text_texture.rect.w = rect->w;
            tex_sector.w = rect->w;
            tex_sector.h = text_texture.rect.h;
            
            SDL_RenderCopy( gRenderer, text_texture.texture, &tex_sector, &text_texture.rect );
        }
        
        
        // Si la textura del texto es mas grande que el rect
        
        
        //Return success
        return text_texture.texture != NULL;
    }
    
    
    
    // we're going to be loading our image from text rendered by SDL_ttf instead of a file.
    // 0 en max_width para no limitar el texto
    bool draw_text (char *text, SDL_Color textColor, FONT_TYPE tipo_fuente, int x, int y)
    {
        SDL_Surface *textSurface = NULL;
        
        //Get rid of preexisting texture
        if (text_texture.texture != NULL) {
            SDL_DestroyTexture( text_texture.texture );
            text_texture.texture = NULL;
            text_texture.rect.w = 0;
            text_texture.rect.h = 0;
            text_texture.rect.x = 0;
            text_texture.rect.y = 0;
        }
        
        // Si no hay texto
        int w = 0, h = 0;
        TTF_SizeText(style.normal_font, text, &w, &h);
        if (w == 0) 
            text = " ";
        
        // Definir que tipo de fuente se va a usar
        if (tipo_fuente == BOLD)
            textSurface = TTF_RenderText_Blended(style.bold_font, text, textColor);
        else textSurface = TTF_RenderText_Blended(style.normal_font, text, textColor);
        
        if( textSurface == NULL ) {
            SDL_Log ("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
            
        } else {
            //Create texture from surface pixels
            text_texture.texture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
            if( text_texture.texture == NULL )
            {
                SDL_Log ("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else {
                //Get image dimensions
                text_texture.rect.w = textSurface->w;
                text_texture.rect.h = textSurface->h;
                
                text_texture.rect.x = x;
                text_texture.rect.y = y;
            }
            
            //Get rid of old surface
            SDL_FreeSurface( textSurface );
        }
        
        //Render font texture to screen
        SDL_RenderCopy( gRenderer, text_texture.texture, NULL, &text_texture.rect );
        
        //Return success
        return text_texture.texture != NULL;
    }
    
    // TODO: Esto deberia estar fucionado con lo de arriba y ademas le falta algunas cosas
    // 0 en max_width para no limitar el texto
    bool draw_text_wrapped (char *text, SDL_Color textColor, FONT_TYPE tipo_fuente, int x, int y, int max_width)
    {
        SDL_Surface *textSurface = NULL;
        
        //Get rid of preexisting texture
        if (text_texture.texture != NULL) {
            SDL_DestroyTexture( text_texture.texture );
            text_texture.texture = NULL;
            text_texture.rect.w = 0;
            text_texture.rect.h = 0;
            text_texture.rect.x = 0;
            text_texture.rect.y = 0;
        }
        
        // Si no hay texto
        int w = 0, h = 0;
        TTF_SizeText(style.normal_font, text, &w, &h);
        if (w == 0) 
            text = " ";
        /*
     Text is wrapped to multiple lines on line endings and on word boundaries
     if it extends beyond wrapLength in pixels.
     If wrapLength is 0, only wrap on new lines.
     This function returns the new surface, or NULL if there was an error.
     */
        
        // Definir que tipo de fuente se va a usar
        if (tipo_fuente == BOLD)
            textSurface = TTF_RenderText_Blended_Wrapped(style.bold_font, text, textColor, max_width);
        else textSurface = TTF_RenderText_Blended_Wrapped(style.normal_font, text, textColor, max_width);
        
        if( textSurface == NULL ) {
            SDL_Log ("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
            
        } else {
            //Create texture from surface pixels
            text_texture.texture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
            if( text_texture.texture == NULL )
            {
                SDL_Log ("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
            }
            else {
                //Get image dimensions
                text_texture.rect.w = textSurface->w;
                text_texture.rect.h = textSurface->h;
                
                text_texture.rect.x = x;
                text_texture.rect.y = y;
            }
            
            //Get rid of old surface
            SDL_FreeSurface( textSurface );
        }
        
        //Render font texture to screen
        SDL_RenderCopy( gRenderer, text_texture.texture, NULL, &text_texture.rect );
        
        //Return success
        return text_texture.texture != NULL;
    }
    
    
    // TODO: Optimizar y borrar codigo duplicado
    // TODO: Deberia usar parametros por defecto de C++
    // Dibuja el boton
    static void draw_boton_core (int button_id, SDL_Rect *rect, char* text, SDL_Event* eventos, GUI::Texture *texture, bool over)
    {
        int w, h;
        
        // TODO: Esto deberia estar en un tema que se pase por parametro
        v2 pad = {5, 5};
        bool center = false;
        int button_size = 32;
        int icon_size = 24;
        
        if (text)
        {
            TTF_SizeText(style.bold_font, text, &w, &h);
            
            if (rect->w == 0 && rect->h == 0) {
                rect->w = w + pad.x;
                rect->h = h + pad.y;
                
            }
            
            // Corrige la posicion del boton para centrarlo
            if (center) {
                rect->x = rect->x - rect->w / 2;
                rect->y = rect->y - rect->h / 2;
            }
            
            //Render rec
            if (over) 
            { 
                if (uistate.active_item == button_id)
                {
                    // El boton esta "hot" y activo
                    SDL_Log ("Activo y hot\n");
                } else {
                    SDL_SetRenderDrawColor( gRenderer, 0x86, 0x8E, 0x96, 0xFF ); // Sobre
                }
            } else {
                // El boton no esta hot, pero podria estar activo
                SDL_SetRenderDrawColor( gRenderer, 0x49, 0x50, 0x57, 0xFF ); // Fuera
            }
            
            
            SDL_RenderFillRect( gRenderer, rect );
            
            //Render label
            draw_label( rect, text, style.font_color, BOLD, CENTER, CENTER, 0);
        }
        
        // Dibujar botones con imagenes (iconos)
        // El tamano del icono no puede ser mayor al del boton.
        // Si icon_size es mayor al tamano original del icono se escalara
        if (texture)
        {
            // Dibuja el contorno del boton
            SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );        
            SDL_RenderDrawRect( gRenderer, rect );
            
            int cortes_laterales = (rect->w - button_size) / 2;
            int cortes_arriba_abajo = (rect->h - button_size) / 2;
            
            SDL_Rect button_rect = *rect;
            
            //FIX: Alguna de estas funciones esta fallando
            cut_left (&button_rect, cortes_laterales);
            cut_right (&button_rect, cortes_laterales);
            cut_top (&button_rect, cortes_arriba_abajo);
            cut_bottom (&button_rect, cortes_arriba_abajo);
            
            // Dibuja el contorno del boton
            SDL_RenderDrawRect( gRenderer, &button_rect );
            
            // Dibuja el fondo de los botones
            SDL_SetRenderDrawColor( gRenderer, 0x21, 0x25, 0x29, 0xFF );
            SDL_RenderFillRect (gRenderer, rect);
            
            int offset = (button_size - icon_size) / 2;
            
            SDL_Rect texture_rect = {button_rect.x + offset, button_rect.y + offset, icon_size, icon_size
            };
            
            
            
            // Dibuja el contorno del icono
            SDL_RenderDrawRect( gRenderer, &texture_rect );
            
            // Dibuja la textura del icono
            SDL_SetTextureColorMod( texture->texture, 140, 143, 148); 
            SDL_RenderCopy( gRenderer, texture->texture, NULL, &texture_rect );
        }
    }
    
    bool draw_boton (int button_id, SDL_Rect* rect, char* text, GUI::Texture *texture, SDL_Event *event)
    {
        bool pressed = false;
        
        // Check if mouse is in button
        bool over = is_over(rect);
        
        // Si el mouse esta sobre el boton, estamos en hot
        if (over)
        {
            uistate.hot_item = button_id;
            if (uistate.active_item == 0 && uistate.mouse_down)
                uistate.active_item = button_id;
            
        }
        
        // Si ningun widget tiene el foco del teclado... lo tomamos
        if (uistate.keyboard_focus == 0) uistate.keyboard_focus = button_id;
        
        // Si tenemos el foco del teclado, lo indicamos
        if (uistate.keyboard_focus == button_id)
        {
            // Si tenemos el foco, tenemos que procesar las teclas
            if (event->type == SDL_KEYDOWN)
            {
                if (uistate.key_entered == SDL_SCANCODE_TAB)
                {
                    // Si presionamos tab, perdemos el foco del teclado
                    // para que lo tome el siguiente widget
                    uistate.keyboard_focus = 0;
                    
                    // Tambie resetea key_entered para que los demas widget no lo procesen
                    uistate.key_entered = 0;
                }
                
                if (uistate.key_entered == SDL_SCANCODE_RETURN)
                {
                    // Si teniamos el foco y se presiono return, entonces actuamos como
                    // si se le hubiese hecho click
                    return 1;
                }
            }
        }
        
        uistate.prev_host_item = button_id;
        
        draw_boton_core (button_id, rect, text, event, texture, over);
        
        if (uistate.keyboard_focus == button_id)
        {
            SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF ); // Focus
            SDL_Rect prueba = {rect->x, rect->y, rect->w, rect->h};
            SDL_RenderDrawRect( gRenderer, &prueba );
        }
        
        
        // Si el boton esta "hot" y activo, pero el boton del mouse no esta apretado,
        // El usuario tiene que haber hecho click en el boton
        if (uistate.mouse_down == 0 && uistate.hot_item == button_id && uistate.active_item == button_id)
            pressed = true;
        
        
        return pressed; 
    }
    
    
    int draw_list (SDL_Rect *rect, SDL_Event *evento, List_Theme *tema, Task_List *lista)
    {
        int item_pressed = 0;
        int modified = 0;
        bool over = false;
        
        SDL_Rect barra;
        SDL_Rect indicator;
        
        int largo_total = lista->cantidad_tareas * (tema->alto_elemento + tema->espacio_entre_elementos);
        int limite_scroll = -(largo_total) + rect->h - tema->espacio_final;
        
        
        tema->ancho_elemento = rect->w - tema->ancho_scroll_bar;
        
        SDL_Rect item = { rect->x, rect->y, tema->ancho_elemento, tema->alto_elemento };
        
        // inicializa la scroll bar
        barra.x = rect->x + rect->w - tema->ancho_scroll_bar;
        barra.y = rect->y;
        barra.w = tema->ancho_scroll_bar;
        barra.h = rect->h;
        
        
        // Define el indicador
        
        indicator.x = barra.x + 2;
        indicator.w = barra.w - 4;
        
        // TODO: Esto falla cuando el largo total es menor que rect->h
        if (rect->h <= largo_total)
            indicator.h = (barra.h) * (rect->h / (float)largo_total);
        else
            indicator.h = barra.h;
        
        
        // Insteracciones del mouse con el marcador de scroll
        
        // FIX: Tiene bastante codigo duplicado
#if 1
        if (evento->type == SDL_MOUSEBUTTONDOWN && (evento->button.button & SDL_BUTTON_LEFT) && is_over (&indicator))
        {
            interacting = true;
        } else if (evento->type == SDL_MOUSEBUTTONUP && (evento->button.button & SDL_BUTTON_LEFT) && interacting)
        {
            interacting = false;
        }
        
        if (evento->type == SDL_MOUSEMOTION && (evento->motion.state & SDL_BUTTON_LMASK) && 
            interacting)
        {
            // Update widget value
            int mouse_pos = evento->motion.y - (barra.y + indicator.h / 2);
            if (mouse_pos < 0) mouse_pos = 0;
            if (mouse_pos > barra.h) mouse_pos = barra.h;
            
            int v = (mouse_pos * largo_total) / barra.h;
            
            if (v != lista->scroll)
            {
                lista->scroll = -v;
            }
        }
        
        if (evento->type == SDL_MOUSEBUTTONDOWN && (evento->button.button & SDL_BUTTON_LEFT) && is_over (&barra))
        {
            // Update widget value
            int mouse_pos = evento->button.y - (barra.y + indicator.h / 2);
            if (mouse_pos < 0) mouse_pos = 0;
            if (mouse_pos > barra.h) mouse_pos = barra.h;
            
            int v = (mouse_pos * largo_total) / barra.h;
            
            if (v != lista->scroll)
            {
                lista->scroll = -v;
            }
        }
        
#endif
        
#if 1
        // TODO: Los pixeles de scroll deberian estar en el tema
        lista->scroll += uistate.mouse_wheel * 20;
        
        
        // Limites del scroll
        // NOTE: El orden de estos dos ifs es importante!!! (para evitar bugs)
        if (lista->scroll < limite_scroll)
        {
            lista->scroll = limite_scroll;
        }
        
        
        if (lista->scroll > 0 || largo_total <= rect->h)
        {
            lista->scroll = 0;
        }
        
        
#endif
        
        // Calculate mouse cursor's relative y offset
        int ypos = 0;
        
        if (largo_total > rect->h)
            ypos = (barra.h * ((float)lista->scroll / (float)largo_total));
        
        // En SDL y positivo es hacia arriba (creo...)
        indicator.y = barra.y - ypos;
        
        
        // Corrige para que el indicador este separado 1 pixel del margen de la barra
        if (indicator.y == barra.y)
            indicator.y += 2;
        else if ((indicator.y + indicator.h + 2) >= barra.y + barra.h)
            indicator.y -= 1;
        
        //
        // Dibujado
        //
        
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );        
        SDL_RenderDrawRect( gRenderer, &barra );
        SDL_RenderFillRect( gRenderer, &indicator );
        
        
        for (int i = 0; i < lista->cantidad_tareas; i++)
        {
            // Define la posicion del elemento de la lista
            item.y = rect->y + (i * (tema->alto_elemento + tema->espacio_entre_elementos)) + lista->scroll;
            
            
            // Dibuja un elemento de la lista si esta dentro del rect
            if (item.y >= rect->y - item.h && item.y <= rect->h + rect->y) {
                
                SDL_Rect elemento = get_right (&item, tema->ancho_elemento - tema->left_offset - 48);
                
                // Codigo de la marca de completado
                SDL_Rect check = get_left(&item, 48);
                check.x += tema->left_offset;
                
                if (lista->tasks[i].done)
                {
                    if (draw_boton (GEN_ID + i, &check, NULL, &check_box_texture, evento))
                    {
                        lista->tasks[i].done = false;
                        modified = true;
                    }
                }
                else {
                    if (draw_boton (GEN_ID + i, &check, NULL, &check_box_outline_blank_texture, evento))
                    {
                        lista->tasks[i].done = true;
                        modified = true;
                    }
                }
                
                
                // Check if mouse is over the list
                if (is_over (rect)) over = is_over (&item);
                
                if (over) 
                {
                    SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a ); // Sobre
                    
                    if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) && is_over (&elemento) && interacting == false)
                    {
                        lista->item_pressed = i;
                        current_screen = TAREAS;
                    }
                    
                } else { 
                    SDL_SetRenderDrawColor( gRenderer, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a ); // Fuera
                    
                }
                
                //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );        
                SDL_RenderFillRect( gRenderer, &elemento);
                //SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );  
                //SDL_RenderDrawRect( gRenderer, &check );
                
                if (tema->title_rect_height)
                {
                    SDL_Rect tittle_rect = get_top (&elemento, elemento.h * tema->title_rect_height);
                    
                    // Dibuja el titulo
                    tittle_rect.w -= 15;
                    
                    draw_label( &tittle_rect, lista->tasks[i].title.buffer, style.font_color, BOLD, LEFT, CENTER, 0);
                }
                
                // Dibuja la previsualizacion del texto
                if (tema->text_rect_height)
                {
                    SDL_Rect text_rect = get_bottom (&elemento, elemento.h * tema->text_rect_height);
                    
                    //SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0xFF, 0xFF );
                    //SDL_RenderDrawRect( gRenderer, &elemento );
                    
                    //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );
                    //SDL_RenderDrawRect( gRenderer, &tittle_rect );
                    
                    //SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );
                    //SDL_RenderDrawRect( gRenderer, &text_rect );
                    
                    // Dibuja el texto debajo del titulo
                    text_rect.w -= 15;
                    draw_label( &text_rect, lista->content.buffer, style.font_color_2, NORMAL_14, LEFT, TOP, 0);
                }
                // Separador 
                SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
                SDL_RenderDrawLine( gRenderer, elemento.x, elemento.y + elemento.h - 1, elemento.x + elemento.w - 1, elemento.y + elemento.h - 1);
                
            }
        }
        
        return modified;
    }
    
    
    // NOTE: Retorna la cantidad de elementos visibles en la lista
    void draw_main_list (SDL_Rect *rect, SDL_Event *evento, List_Theme *tema, Main_List *lista)
    {
        int item_pressed = 0;
        bool over = false;
        
        SDL_Rect barra;
        SDL_Rect indicator;
        
        int largo_total = lista->cantidad_listas * (tema->alto_elemento + tema->espacio_entre_elementos);
        int limite_scroll = -(largo_total) + rect->h - tema->espacio_final;
        
        
        
        tema->ancho_elemento = rect->w - tema->ancho_scroll_bar;
        
        SDL_Rect item = { rect->x, rect->y, tema->ancho_elemento, tema->alto_elemento };
        
        // inicializa la scroll bar
        barra.x = rect->x + rect->w - tema->ancho_scroll_bar;
        barra.y = rect->y;
        barra.w = tema->ancho_scroll_bar;
        barra.h = rect->h;
        
        
        // Inicializa el indicador
        indicator.x = barra.x + 2;
        indicator.w = barra.w - 4;
        
        // TODO: Esto falla cuando el largo total es menor que rect->h
        if (rect->h <= largo_total)
            indicator.h = (barra.h) * (rect->h / (float)largo_total);
        else
            indicator.h = barra.h;
        
        
        // Insteracciones del mouse con el marcador de scroll
        
        // FIX: Tiene bastante codigo duplicado
#if 1
        if (evento->type == SDL_MOUSEBUTTONDOWN && (evento->button.button & SDL_BUTTON_LEFT) && is_over (&indicator))
        {
            interacting = true;
        } else if (evento->type == SDL_MOUSEBUTTONUP && (evento->button.button & SDL_BUTTON_LEFT) && interacting)
        {
            interacting = false;
        }
        
        if (evento->type == SDL_MOUSEMOTION && (evento->motion.state & SDL_BUTTON_LMASK) && 
            interacting)
        {
            // Update widget value
            int mouse_pos = evento->motion.y - (barra.y + indicator.h / 2);
            if (mouse_pos < 0) mouse_pos = 0;
            if (mouse_pos > barra.h) mouse_pos = barra.h;
            
            int v = (mouse_pos * largo_total) / barra.h;
            
            if (v != lista->scroll)
            {
                lista->scroll = -v;
            }
        }
        
        if (evento->type == SDL_MOUSEBUTTONDOWN && (evento->button.button & SDL_BUTTON_LEFT) && is_over (&barra))
        {
            // Update widget value
            int mouse_pos = evento->button.y - (barra.y + indicator.h / 2);
            if (mouse_pos < 0) mouse_pos = 0;
            if (mouse_pos > barra.h) mouse_pos = barra.h;
            
            int v = (mouse_pos * largo_total) / barra.h;
            
            if (v != lista->scroll)
            {
                lista->scroll = -v;
            }
        }
        
#endif
        
        // Scroll
#if 1
        // TODO: Los pixeles de scroll deberian estar en el tema
        lista->scroll += uistate.mouse_wheel * 20;
        
        
        // Limites del scroll
        // NOTE: El orden de estos dos ifs es importante!!! (para evitar bugs)
        if (lista->scroll < limite_scroll)
        {
            lista->scroll = limite_scroll;
        }
        
        if (lista->scroll > 0 || largo_total <= rect->h)
        {
            lista->scroll = 0;
        }
#endif
        
        // Calculate mouse cursor's relative y offset
        int ypos = 0;
        
        if (largo_total > rect->h)
            ypos = (barra.h * ((float)lista->scroll / (float)largo_total));
        
        
        // En SDL y positivo es hacia arriba (creo...)
        indicator.y = barra.y - ypos;
        
        
        // Corrige para que el indicador este separado 1 pixel del margen de la barra
        if (indicator.y == barra.y)
            indicator.y += 2;
        else if ((indicator.y + indicator.h + 2) >= barra.y + barra.h)
            indicator.y -= 1;
        
        //
        // Dibujado
        //
        
        SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );        
        SDL_RenderDrawRect( gRenderer, &barra );
        SDL_RenderFillRect( gRenderer, &indicator );
        
        
        for (int i = 0, show_counter = 0; i < lista->cantidad_listas; i++, show_counter++)
        {
            if (lista->listas[i].show == false) { 
                show_counter--; 
                continue;
            }
            
            // Define la posicion del elemento de la lista
            item.y = rect->y + (show_counter * (tema->alto_elemento + tema->espacio_entre_elementos)) + lista->scroll;
            
            
            // Dibuja un elemento de la lista si esta dentro del rect
            if (item.y >= rect->y - item.h && item.y <= rect->h + rect->y) {
                
                SDL_Rect elemento = get_right (&item, tema->ancho_elemento - tema->left_offset - 48);
                
                // Check if mouse is over the list
                if (is_over (rect)) over = is_over (&item);
                
                if (over) 
                {
                    SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a ); // Sobre
                    
                    if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) && is_over (&elemento) && interacting == false)
                    {
                        lista->item_pressed = i;
                        current_screen = TAREAS;
                    }
                    
                } else { 
                    SDL_SetRenderDrawColor( gRenderer, style.background_color.r, style.background_color.g, style.background_color.b, style.background_color.a ); // Fuera
                    
                }
                
                //SDL_SetRenderDrawColor( gRenderer, 0xFF, 0x00, 0x00, 0xFF );        
                SDL_RenderFillRect( gRenderer, &elemento);
                //SDL_SetRenderDrawColor( gRenderer, 0x00, 0xFF, 0x00, 0xFF );  
                //SDL_RenderDrawRect( gRenderer, &check );
                
                if (tema->title_rect_height)
                {
                    SDL_Rect tittle_rect = get_top (&elemento, elemento.h * tema->title_rect_height);
                    
                    // Dibuja el titulo
                    tittle_rect.w -= 15;
                    
                    draw_label( &tittle_rect, lista->listas[i].title.buffer, style.font_color, BOLD, LEFT, CENTER, 0);
                    
                    // NOTE: Dibuja la cantidad sin completar y el total de tareas
                    //draw_label(&elemento, "2/5", style.font_color, NORMAL_14, RIGHT, TOP, 0);
                }
                
                // Separador 
                SDL_SetRenderDrawColor( gRenderer, style.text_box_color.r, style.text_box_color.g, style.text_box_color.b, style.text_box_color.a );
                SDL_RenderDrawLine( gRenderer, elemento.x, elemento.y + elemento.h - 1, elemento.x + elemento.w - 1, elemento.y + elemento.h - 1);
                
            }
        }
        
    }
    
    
    
    
    // Comprueba si el mouse esta sobre un rect
    bool is_over(SDL_Rect *rect)
    {
        int x, y;
        bool over = true;
        
        // Get mouse position
        SDL_GetMouseState (&x, &y);
        
        // Posicion del mouse con respecto al boton
        if (x < rect->x ||                // el mouse esta a la izquierda
            x > rect->x + rect->w ||      // a la derecha
            y < rect->y ||                // arriba
            y > rect->y + rect->h)        // abajo
            over = false;
        
        // Si no esta en ninguna de esas posiciones, solo puede estar sobre el boton.
        return over;
    }
    
    
    int text_field(int id, SDL_Rect* rect, Text_buffer *text_buffer, char* hint_text)
    {
        int len = text_buffer->length;
        int changed = 0;
        
        bool over = GUI::is_over(rect);
        
        // Check for hotness
        if (over)
        {
            uistate.hot_item = id;
            
            if (uistate.active_item == 0 && uistate.mouse_down)
            {
                uistate.active_item = id;
            }
        }
        
        // If no widget has keyboard focus, take it
        if (uistate.keyboard_focus == 0) uistate.keyboard_focus = id;
        
        
        // Render the text field
        if (uistate.active_item == id || uistate.hot_item == id)
        {
            SDL_SetRenderDrawColor( gRenderer, 0x86, 0x8E, 0x96, 0xFF ); // Sobre
        } else 
        {
            SDL_SetRenderDrawColor( gRenderer, 0x49, 0x50, 0x57, 0xFF ); // Fuera
        }
        
        
        SDL_RenderFillRect( gRenderer, rect );
        
        if (uistate.keyboard_focus != id && text_buffer->length == 0) {
            draw_label (rect, hint_text, style.font_color_white, GUI::NORMAL, GUI::CENTER, GUI::CENTER, 0);
        } else {
            draw_label (rect, text_buffer->buffer, style.font_color_white, GUI::NORMAL, GUI::CENTER, GUI::CENTER, 0);
        }
        
        
        // Renderiza el cursor (TODO: Aun funciona mal...)
        if (uistate.keyboard_focus == id && (SDL_GetTicks() >> 8) & 1)
        {
            if (text_texture.rect.x > 0)
                draw_text ("|", style.font_color_white, GUI::NORMAL, text_texture.rect.x + text_texture.rect.w - 6, text_texture.rect.y);
            else {
                draw_label (rect, "|", style.font_color_white, GUI::NORMAL, GUI::CENTER, GUI::CENTER, 0);
            }
        } 
        
        // If we have keyboard focus, show it
        if (uistate.keyboard_focus == id)
        {
            SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF ); // Focus
            SDL_Rect prueba = {rect->x, rect->y, rect->w + 2, rect->h + 2};
            SDL_RenderDrawRect( gRenderer, &prueba );
        }
        
        
        // If button is hot and active, but mouse button is not dow, the user must have clicked
        // the widget; give it keyboard focus
        if (uistate.mouse_down == 0 && uistate.hot_item == id && uistate.active_item == id)
            uistate.keyboard_focus = id;
        
        
        // if we have keyboard focus, we'll need to process the key
        if (uistate.keyboard_focus == id)
        {
#if 0
            switch (uistate.key_entered)
            {
                //case SDLK_TAB:
                case SDL_SCANCODE_TAB:
                {
                    // if tab pressed, lose keyboard focus
                    // Next widget will grab the focus
                    uistate.keyboard_focus = 0;
                    // If shift was also pressed, we want to move focus
                    // to the previous widget instead
                    
                    //if (uistate.keymod & KMOD_SHIFT) uistate.keyboard_focus = //uistate.prev_host_item;
                    
                    // Also clear the key so that next widget won't process it
                    uistate.key_entered = 0;
                    
                } break;
                
                case SDL_SCANCODE_BACKSPACE:
                {
                    if (len > 0)
                    {
                        len--;
                        buffer[len] = 0;
                        changed = 1;
                    }
                    
                } break;
            }
#endif
            
#if 1
            // TODO: Pasar esto al sistema de arriba
            // Procesar los eventos de texto en el ciclo principal
            // Y pasar el texto y CTRL + C y CTRL + V como flags en el estado de la ui?
            // O encontrar la razon de porque registra mas de 1 evento
            
            // NOTE: Esto es para recibir texto UNICODE...
            // Para hacerlo simple y rapido deberia solo recibir ASCII como arriba...
            // Con lo de arriba es casi imposible recibir texto escrito...
            // Este metodo es mejor pero no se por que los multiples eventos
            // Deberia revizar todo el codigo con el depurador y ordenar bien la entrada
            // del teclado.
            
            // No deberia recibir el evento en la funcion...
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                // TODO: No funcion como deberia
                if (event.type == SDL_MOUSEBUTTONDOWN)
                {
                    if ((event.button.button & SDL_BUTTON_LEFT) && !over)
                    {
                        uistate.keyboard_focus = 0;
                    }
                }
                
                if (event.type == SDL_TEXTINPUT && text_buffer->length < text_buffer->size) // Special text input event
                {
                    //text_buffer->buffer[text_buffer->length] = event.text.text[0];
                    //strncpy(text_buffer->buffer, event.text.text, text_buffer->size);
                    strncat(text_buffer->buffer, event.text.text, text_buffer->size);
                    //text_buffer->buffer = event.text.text;
                    //text_buffer->length++;
                    text_buffer->length = strlen(text_buffer->buffer);
                    text_buffer->buffer[text_buffer->length] = '\0';
                    
                }
                
                if (event.type == SDL_KEYDOWN)
                {
                    if (event.key.keysym.sym == SDLK_TAB)
                    {
                        uistate.keyboard_focus = 0;
                    }
                    
                    // Handle backspace
                    if (event.key.keysym.sym == SDLK_BACKSPACE && text_buffer->length > 0)
                    {
                        text_buffer->length--;
                        text_buffer->buffer[text_buffer->length] = 0;
                        //changed = 1;
                    }
                    
                    // Handle copy
                    
                    // Handle paste
                    
                    
                    // Handle ENTER
                    if (event.key.keysym.sym == SDLK_RETURN && text_buffer->length > 0)
                    {
                        changed = 1;
                    }
                }  
                
                
#endif
                
#if 0
                if (uistate.keychar >= 32 && uistate.keychar < 127 && len < 30)
                {
                    buffer[len] = uistate.keychar;
                    len++;
                    buffer[len] = 0;
                    changed = 1;
                }
                
#endif
            }
        }
        
        uistate.prev_host_item = id;
        
        
        
        return changed;
    }
    
    
    
    
    
    // FIX: Este codigo es bastante malo
    void draw_menu_principal(SDL_Rect rect, SDL_Event *evento)
    {
        // el enum parte en 1, hay que saltarse Menu_principal, y otro no se porque...
        int cantidad_botones = CANTIDAD_VENTANAS - 3; 
        
        // Esto hace que si la ventana no tiene un nombre, deja un boton vacio y no crashea
        char *nombres[CANTIDAD_VENTANAS - 1] = {
            "Notas",
            "Editor",
            "Crono",
            "Config",
            "Lista",
            "Prueba", // Despues viene el menu principal
        };
        
        int boton_width = 100, boton_height = 100;
        
        int botones_por_fila = rect.w / boton_width;
        int botones_por_col = rect.h / boton_height;
        
        v2 padding = {0, 0};
        
        
        // Podria dibujarlos con un solo ciclo, cuando llegue al final de la fila
        // que pase a dibujarlos en la siguiente...
        for (int i = 0; i < botones_por_col; i++)
        {
            
            SDL_Rect fila = GUI::cut_top(&rect, boton_height);
            
            for (int colocados = 0; colocados < botones_por_fila; colocados++)
            {
                SDL_Rect boton = GUI::cut_left(&fila, boton_width);
                
                
                
                if (nombres[cantidad_botones])
                {
                    if (draw_boton(GEN_ID + colocados, &boton, nombres[cantidad_botones], NULL, evento))
                    {
                        // Selecciona la ventana correcta
                        current_screen = (GUI::screens) (cantidad_botones + 1);
                    }
                }
                // Para que no de el error de dibujar un string vacio
                else 
                    draw_boton(GEN_ID, &boton, " ", NULL, evento);
                
                
                if (cantidad_botones == 0) return;
                
                // Esto hace que muestre los botones en orden inverso
                cantidad_botones--;
            }
            
        }
    }
}
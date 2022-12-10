#ifndef UI_H
#define UI_H

#include "tareas.h"


#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif


//namespace GetRect {
namespace GUI {
    
    
    typedef struct _General_App_Style
    {
        // Colores
        SDL_Color background_color;
        SDL_Color font_color;
        SDL_Color font_color_2;
        SDL_Color font_color_white;
        
        // Temporal: hasta que cree el tema exclusivo para los botones
        SDL_Color button_color;
        SDL_Color over_button_color;
        
        SDL_Color text_box_color;
        
        SDL_Color icon_color;  // ?
        
        SDL_Color red_color;  // ?
        
        
        //Globally used font
        TTF_Font *bold_font;
        TTF_Font *normal_font;
        TTF_Font *normal_font_2;
        
    } General_Style;
    
    
    typedef struct _List_Theme
    {
        int ancho_scroll_bar = 15;
        
        int espacio_entre_elementos;
        int alto_elemento;
        int ancho_elemento;
        float title_rect_height = 0.5;
        float text_rect_height = 0.5;
        int left_offset;
        
        int espacio_final;
        
        char font;
        
        int text_color;
        int text_over_color;
        
        int background_color;
        int background_color_active; 
    } List_Theme;
    
    
    // Esto deberia tener unos valores por defecto y
    // cambiarlos para cada caso especial
    typedef struct _Button_Theme
    {
        // using label_theme: Label_Theme
        
        // Rectangle_Shape rectangle_shape;
        // Vector 4 surface_color
        // Vector 4 surface_color_over
        
        // Vector 4 surface_color_flash
        // Vector 4 surface_color_down
        
        // Vector 4 text_color
        // Vector 4 text_color_over
        // Vector 4 text_color_pressed
        
        // text_offset_down Vector2.{.12, -.12} // in ems. (desplazamiento del texto cuando se aprita el boton)
        
        
        // Default frame thickness comes from rectagle_shape
        // frame_thickness_over := .04;
        // frame_thickness_down := .05;
        // frame_thickness_flash := .07;
        
        
        // frame_color := Vector4.{.40, .40, .50, 1.0};
        // frame_color_over := Vector4.{.55, .55, .45, 1.0};
        // frame_color_flash := Vector4.{.99, .99, .65, 1.0};
        // frame_color_down := Vector4.{.75, .75, .55, 1.0};
        
        // press_duration := 0.7; // The flash when you press the button
        
        // texture_scale := 1.0; // Relative to the size of the button, if a texture is being draw
        
    } Button_Theme;
    
    
    // Button.jai
    
    /* typedef struct _Button_State
    {
    using widtget: Active_Widget;
    } Button_State;
    
    */
    
    typedef struct {
        SDL_Texture *texture;
        SDL_Rect rect;
    } Texture;
    
    // Hot item is the one that's below the mouse cursor. If you move the mouse cursor on top of a button, for example, it becomes 'hot'.
    //Active item is the one you're currently interacting with. A button becomes 'active' when you press the mouse button on it. If you keep the button pressed and drag the mouse from the button, the button remains 'active', even though it loses it's 'hot' state.
    
    struct UIstate
    {
        int mouse_x;
        int mouse_y;
        int mouse_down;
        
        int mouse_wheel;
        
        int prev_host_item;
        int hot_item;     // ID del item actualmente hot
        int active_item;  // ID del item actualmente activo
        
        // id del widget con keyboard focus (0 = nadie lo tiene)
        int keyboard_focus;  // ID del widget que tiene el foco del teclado
        int key_entered;     // almacena la tecla que se presiono (si se preiono TAB)
        int keymod;          // almacena las key modifier flags (como shift)
        
        char key_char;       // Para ingreso de texto para los inputs
        
        int key_count;
        uint8_t *last_state;
        const uint8_t *current_state;
    };
    
    
    // Prueba de almacenar layout las ventanas de notas
    typedef enum
    {
        LAYOUT_PRINCIPAL = 0,
        
        // barra principal
        TITLE_BAR,
        LEFT_BUTTON,
        RIGHT_BUTTON,
        
        // Layout ventana de listas
        LAYOUT_LISTA,
        // Barra de busqueda
        BUSQUEDA,
        SEARCH_BUTTON,
        CANCEL_BUTTON,
        
        BOTTOM_BAR,
        
        LIST_PANEL,
        
        // Layout ventana de tareas
        LAYOUT_TAREAS,
        LISTA_NAME,
        CREATED_TEXT,
        MODIF_TEXT,
        ADD_TASK_INPUT,
        SEPARADOR_INPUT_LIST,
        TASKS_LIST,
        
        // Layout ventana del Editor
        LAYOUT_EDITOR,
        CRE_TEXT,
        MOD_TEXT,
        NOTE_TITLE,
        
        // Layout Menu Principal
        LAYOUT_MENU,
        OPCIONES_PANEL,
        
        NUM_ELEMENTOS
        
    } layout_index;
    
    
    typedef struct
    {
        SDL_Rect bar;
        SDL_Rect indicator;  // y es el scroll_indicator_pos;
        
        int scroll;
        int scroll_min;
        int scroll_max;
        int scroll_pos;
        
    } Scroll_bar;
    
    
    //
    // Manejo de la entrada de Mouse y Teclado
    //
    
    enum screens {
        LISTAS= 1,
        LIST_EDITOR,
        CRONO,
        CONFIG,
        TAREAS,
        PRUEBA,
        MENU_PRINCIPAL,
        
        CANTIDAD_VENTANAS
    };
    
    typedef enum {
        LEFT,
        RIGHT,
        CENTER,
        TOP,
        BOTTOM
    } ALIGN;
    
    typedef enum {
        NORMAL,
        NORMAL_14,
        BOLD
    } FONT_TYPE;
    
    typedef enum {
        ACEPTAR = 2,
        SI,
        NO
    } RESPUESTAS;
    
    
    typedef struct
    {
        Text_buffer buffer;
        
        TTF_Font *font;
        GUI::Texture Textura;
        SDL_Rect rect;
        
        bool active;
        bool clicked;
        
    } input_field_state;
    
    
    
    
    int text_field(int id, SDL_Rect* rect, Text_buffer *buffer, char* hint_text);
    
    void imgui_prepare(struct UIstate *state);
    void imgui_finish(struct UIstate *state);
    
    
    void init_general_theme(General_Style* style);
    
    
    bool draw_label (SDL_Rect *rect, char *text, SDL_Color textColor, FONT_TYPE tipo_fuente,
                     ALIGN horizontal_align, ALIGN vertical_align, int desplazamiento);
    
    bool draw_text (char *text, SDL_Color textColor, FONT_TYPE tipo_fuente, int x, int y);
    
    bool draw_text_wrapped (char *text, SDL_Color textColor, FONT_TYPE tipo_fuente, int x, int y, int max_width);
    
    bool draw_boton (int button_id, SDL_Rect* rect, char* text, GUI::Texture *texture, SDL_Event *event);
    
    bool draw_boton (int id, SDL_Rect *pos, char *text, SDL_Event *evento, v2 pad, 
                     bool center, bool limit, bool text_boton);
    
    bool draw_boton_img (SDL_Rect *pos, SDL_Event *evento, Texture *texture, int button_size, int icon_size);
    
    int draw_list (SDL_Rect *rect, SDL_Event *evento, List_Theme *tema, Task_List *estado);
    void draw_main_list(SDL_Rect *rect, SDL_Event *evento, List_Theme *tema, Main_List *lista);
    
    
    void draw_menu_principal(SDL_Rect rect, SDL_Event *evento);
    
    
    
    // Text Input
    void draw_editor (SDL_Rect *layout, TTF_Font *font);
    void init_editor (SDL_Rect *rect);
    void editor_open (Text_buffer *buffer);
    
    
    // RectCut
    
    inline int rect_min (int a, int b);
    inline int rect_max (int a, int b);
    
    SDL_Rect cut_left (SDL_Rect *rect, int a);
    SDL_Rect cut_right (SDL_Rect *rect, int a);
    SDL_Rect cut_top (SDL_Rect *rect, int a);
    SDL_Rect cut_bottom (SDL_Rect *rect, int a);
    
    SDL_Rect get_left (SDL_Rect *rect, int a);
    SDL_Rect get_right (SDL_Rect *rect, int a);
    SDL_Rect get_top (SDL_Rect *rect, int a);
    SDL_Rect get_bottom (SDL_Rect *rect, int a);
    
    void expand_toward (SDL_Rect *rect, int obj_width, float dt, float seconds);
    
    void contract_toward (SDL_Rect *rect, int obj_width, float dt, float seconds);
    
    
    void move_toward(float dt, SDL_Rect *rect, int target_pos, float seconds);
    
    
    
    bool proccess_input_text_events(SDL_Event *evento);
    bool draw_input_field (SDL_Rect *pos, char *text, Text_buffer *buffer, SDL_Event *evento, v2 padding, bool focus);
    
    bool is_over(SDL_Rect *rect);
}




#endif
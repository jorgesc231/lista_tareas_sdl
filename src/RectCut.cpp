/* date = March 11th 2021 1:14 pm */
/*
*    Implementacion de RectCut para un layout de UI independiente de la resolucion.
*/

/*
* TODO:
* Explicar aqui como funciona
*/

#include <SDL.h>


namespace GUI {
    
#ifndef _RECT_CUT_H
#define _RECT_CUT_H
    
    // NOTE: Revisar esto: https://dustri.org/b/min-and-max-macro-considered-harmful.html
    inline int rect_min (int a, int b)
    {
        if (a < b) return a;
        else if (b < a) return b;
        else return a;
    }
    
    inline int rect_max (int a, int b)
    {
        if (a > b) return a;
        else if (b > a) return b;
        else return a;
    }
    
#ifdef RECTCUT
    
    struct Rect {
        int minx, miny, maxx, maxy;
    };
    
    // These functions cut a smaller rectangle of an input rectangle and return it. 
    // The trick is that they also modify the input rectangle.
    
    // Este falla
    Rect cut_left (Rect *rect, int a)
    {
        int minx = rect->minx;
        rect->minx = rect_min (rect->maxx, rect->minx + a);
        
        return (Rect) {minx, rect->miny, rect->minx, rect->maxy};
    }
    
    Rect cut_right (Rect *rect, int a)
    {
        int maxx = rect->maxx;
        rect->maxx = max (rect->minx, rect->maxx - a);
        
        return (Rect) {rect->maxx, rect->miny, maxx, rect->maxy};
    }
    
    Rect cut_top(Rect* rect, int a) {
        int miny = rect->miny;
        rect->miny = rect_min(rect->maxy, rect->miny + a);
        
        return (Rect){ rect->minx, miny, rect->maxx, rect->miny };
    }
    
    Rect cut_bottom(Rect* rect, int a) {
        int maxy = rect->maxy;
        rect->maxy = max(rect->miny, rect->maxy - a);
        
        return (Rect){ rect->minx, rect->maxy, rect->maxx, maxy };
    }
    
#endif
    
    //
    // Versiones para SDL
    //
    
    
    
    // FIX: Fallan cuando el layout esta en 0
    // Deberia comprender bien como funcionan y optimizar esto...
    
    SDL_Rect cut_left (SDL_Rect *rect, int a)
    {
        int x = rect->x;
        rect->x = rect_min (rect->x + rect->w, rect->x + a);
        rect->w -= a;
        
        SDL_Rect result = {x, rect->y, a, rect->h};
        
        return result;
    }
    
    SDL_Rect cut_right (SDL_Rect *rect, int a)
    {
        //rect->w = max (rect->x, rect->w - a);
        rect->w = rect->w - a;
        
        SDL_Rect result = {rect->x + rect->w, rect->y, a, rect->h};
        
        return result;
    }
    
    SDL_Rect cut_top (SDL_Rect *rect, int a)
    {
        int y = rect->y;
        rect->y = rect_min(rect->y + rect->h, rect->y + a);
        rect->h -= a;
        
        SDL_Rect result = { rect->x, y, rect->w, rect->y - y };
        
        return result;
    }
    
    SDL_Rect cut_bottom (SDL_Rect *rect, int a)
    {
        rect->h = rect_min (rect->h, rect->h - a);
        
        SDL_Rect result = { rect->x, rect->y + rect->h, rect->w, a};
        
        return result;
    }
    
    //
    // Obtiene el rectangulo (no modifica el rectangulo de entrada)
    //
    
    SDL_Rect get_left (SDL_Rect *rect, int a)
    {
        SDL_Rect result =  {rect->x, rect->y, a, rect->h};
        
        return result;
    }
    
    SDL_Rect get_right (SDL_Rect *rect, int a)
    {
        SDL_Rect result = {rect->x + rect->w - a, rect->y, a, rect->h};
        return result;
    }
    
    SDL_Rect get_top (SDL_Rect *rect, int a)
    {
        SDL_Rect result = { rect->x, rect->y, rect->w, a };
        return result;
    }
    
    SDL_Rect get_bottom (SDL_Rect *rect, int a)
    {
        SDL_Rect result = { rect->x, rect->y + rect->h - a, rect->w, a};
        return result;
    }
    
    
    SDL_Rect shrink(SDL_Rect *rect, int margin)
    {
        // TODO:
        // Recorta un margen del rect en todos sus lados.
        ;
        
        return *rect;
    }
    
    // TODO: Mejorar este codigo
    /*
if (left_menu.w <= layout_notas[LAYOUT_PRINCIPAL].w / 2)
            left_menu.w += 2000 * delta_time;
*/
    void expand_toward (SDL_Rect *rect, int target_width, float dt, float seconds)
    {
        float rapidez = target_width / seconds;
        
        if (rect->w < target_width)
            rect->w += rapidez * dt;
        
        else if (rect->w >= target_width)
            rect->w = target_width;
        
    }
    
    /*
if (left_menu.w > 0)
    {
     left_menu.w -= 2000 * delta_time;
    }
    
    if (left_menu.w < 0) left_menu.w = 0;
*/
    void contract_toward (SDL_Rect *rect, int target_width, float dt, float seconds)
    {
        // FIX: Se que esta mal, pero es lo unico que se me ocurrio ahora...
        // static float mal = rect->w;
        
        float rapidez = rect->w / seconds;
        
        if (rect->w > target_width)
            rect->w -= rapidez * dt;
        
        if (rect->w < 0) rect->w = 0;
    }
    
    void move_toward (float dt, float* a_pointer, float target, 
                      float rate_up, float rate_down)
    {
        ;
    }
}

#endif //_RECT_CUT_H

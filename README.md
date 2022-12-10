# Lista de tareas usando C/C++ y SDL2

Programa para gestionar listas de tareas. 

Permite crear listas de tareas y agregarles tareas, ademas de marcarlas como completadas.

Almacena las listas de tareas como archivos de texto plano para permitir la edicion fuera del programa.

Intefaz grafica usando C/C++ con las bibliotecas SDL2, SDL_Image, SDL_TTF.

La entrada de texto tiene el problema que no registra algunas pulsaciones de tecla, hay que volver a presionarla hasta que apareza.
(Lo solucionare pronto)

__Este es solo un proyecto que hice para aprender como hacer interfaces graficas desde cero usando C/C++.__


## Usar

Descargar desde la seccion de Releases o desde el siguiente link:

    https://github.com/jorgesc231/lista_tareas_sdl/releases/download/v0.2.0/lista_tareas_sdl_win64_release.zip


Ejecutar el ejecutable:

    lista_tareas_sdl.exe

(No recomiendo usarlo enserio porque aun tiene bastantes bugs, sobre todo en la entrada de texto y la forma de almacenar los datos)

## Compilar

SDL2, la fuente y los iconos estan incluidos en el repo.

clonar el repositorio:

    git clone https://github.com/jorgesc231/lista_tareas_sdl.git
    
Abrir una terminal con cl e ir al directorio:

    cd lista_tareas_sdl

y ejecutar el archivo:

    build.bat


__El ejecutable se almacena en el directorio build que esta en el directorio raiz del repositorio.__


## Por Hacer

- Arreglar la entrada de texto
- Soporte UTF-8
- Mejorar focus del teclado
- limpiar el codigo
- Mucha optimizacion
- Terminar version de Linux               
- Mejora los botones
- Transiciones entre ventanas
- Animaciones en la Interfaz Grafica


## Bugs

- A veces la entrada de texto no registra algunas teclas
- No funciona el arrastre de la scroll bar
- A veces duplica un elemento de la lista 
- Crashs aleatorios
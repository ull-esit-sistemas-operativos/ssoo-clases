// fifo-server.c - Servidor del ejemplo del uso de tuberías con nombre para comunicar procesos
//
//  El programa servidor utiliza alarm() y las señales del sistema para mostrar periódicamente la hora. Además, crea
//  una tubería FIFO a la que puede conectarse el programa cliente para darle órdenes.
//
//  Compilar:
//
//      gcc -I../ -o fifo-server fifo-server.c ../common/timeserver.c
//

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>     // Cabecera principal de la API POSIX del sistema operativo
#include <fcntl.h>      // Algunas operaciones del estándar POSIX con descriptores de archivo no están en <unistd.h>
                        // sino aquí. Por ejemplo open()
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include <common/timeserver.h>
#include "fifo-server.h"

const long CONTROL_POLLING_TIME = 500000000 /* ns. */;  // 500 ms.

int make_control_shm();
void cleanup_control_shm();

int alloc_control_memory(int* controlfd);
int free_control_memory(int controlfd, char* command);

int main()
{
    int exit_code = 0;
    bool fifo_created = false;
    bool fifo_opened = false;
    int controlfd = -1;

    // Las variables anteriores y las siguientes estructuras de if son para el manejo de errores en C. Así liberamos
    // recursos y dejamos todo en su sitio al terminar, incluso si salimos  es por un error. Por ejemplo, si al salir
    // por un error olvidamos eliminar la tubería, no podríamos volver a ejecutar el servidor hasta borrarla a mano.
    // En C++ es mejor encapsular los recursos en clases y usar su destructor para liberarlos (RAII).

    if (! exit_code)
    {
        exit_code = make_control_shm();
        if (exit_code == 0)
        {
            fifo_created = true;
        }
    }

    if (! exit_code)
    {
        exit_code = alloc_control_memory(&controlfd);
        if (exit_code == 0)
        {
            fifo_opened = true;
        }
    }

    if (! exit_code)
    {
        setup_signals();
        start_alarm();

        printf( "Escuchando en la tubería de control '%s'...\n", CONTROL_FIFO_PATH);

        struct timespec polling_time =
        {
            .tv_sec = 0,
            .tv_nsec = CONTROL_POLLING_TIME
        };
          
        // Leer de la tubería de control los comandos e interpretarlos.
        while (!quit_app)
        {
            char command[MAX_COMMAND_SIZE + 1];

            exit_code = free_control_memory( controlfd, command );
            if (exit_code != 0 || quit_app) break;

            if (command[0] == '\0')
            {
                // No hay nada que leer. Dormimos un rato el proceso para no quemar CPU.
                nanosleep(&polling_time, NULL);
            }
            else
            {
                if (strcmp( command, QUIT_COMMAND ) == 0)
                {
                    quit_app = true;
                }

                // Aquí va código para detectar e interpretar más comandos...
                //                
            }
        }

        stop_alarm();
    }

    // Vamos a salir del programa...
    puts( "Ha llegado orden de terminar ¡Adiós!" );

    if (fifo_created)
    {
        cleanup_control_shm();
    }

    if (fifo_opened)
    {
        close(controlfd);
    }

    return exit_code;
}

int make_control_shm()
{
    // Como no hay función en el estándar de C ni en el de C++ para crear tuberías, usamos directamente mkfifo()
    // de la librería del sistema.
    int return_code = mkfifo( CONTROL_FIFO_PATH, 0666 );
    if (return_code < 0)
    {
        if (errno == EEXIST)
        {
            // Si ya existe la tubería, no deberíamos usarla porque habrían varios servidores usando el mismo canal
            // de control. Algunos mensajes llegarían a un servidor y otros al otro. 
            fputs( "Error: Hay otro servidor en ejecución.\n", stderr );
        }
        else
        {
            fprintf( stderr, "Error (%d) al crear la tubería: %s\n", errno, strerror(errno) );
        }
        
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

void cleanup_control_shm()
{
    // Eliminar la tubería con nombre. Nadie más podrá conectarse.
    int return_code = unlink( CONTROL_FIFO_PATH );
    if (return_code < 0)
    {
        fprintf( stderr, "Error (%d) al borrar la tubería: %s\n", errno, strerror(errno) );
    }
}

int alloc_control_memory(int* controlfd)
{
    // Abrir la tubería, recientemente creada, usando su nombre como un archivo convencional.
    // También se puede abrir con std::fopen() o std::ifstream,  pero necesitamos pasar O_NONBLOCK a open() porque
    // sino se bloquea el proceso hasta que otro proceso abre la tubería para lectura.
    *controlfd = open( CONTROL_FIFO_PATH, O_RDONLY | O_NONBLOCK );
    if (controlfd < 0)
    {
        fprintf( stderr, "Error (%d) al abrir la tubería: %s\n", errno, strerror(errno) );
        return EXIT_FAILURE;
    }

    // Teniendo un descriptor de archivos del sistema operativo se puede obtener un FILE de la librería estándar de
    // C usando fdopen(). Así sería posible usar las facilidades de la librería estándar. Como lo que vamos a hacer
    // es sencillo, lo haremos trabajando directamente con el descriptor de archivo y la librería del sistema.

    // Desactivar el modo O_NONBLOCK para que read() sea bloqueante.
    int fd_flags = fcntl( *controlfd, F_GETFL); 
    fcntl( *controlfd, F_SETFL, fd_flags & (~O_NONBLOCK) );

    return EXIT_SUCCESS;
}

int free_control_memory(int controlfd, char* command)
{
    // La dificultad está en que como se trata de comunicacion orientada a flujos, no se conserva la división entre
    // mensajes (comandos). Tenemos que elegir un delimitador ('\n') al mandar los mensajes y buscarlo al leer.

    char* buffer = command;
    char byte;
    size_t bytes_read = 0;

    while (true)
    {
        int return_code = read( controlfd, &byte, 1 );
        if (return_code < 0) 
        {
            if (errno == EINTR)
            {
                // Llegó una señal ¿debe terminar la aplicación o reintentar read()?
                if (quit_app)
                {
                    command[0] = '\0';
                    return EXIT_SUCCESS;
                }
                else continue;
            }
            else { 
                fprintf( stderr, "Error (%d) al leer de la tubería: %s\n", errno,
                    strerror(errno) );
                return EXIT_FAILURE;
            }
        }
        else if (return_code == 0)
        {
            // Ya no hay ningún proceso con el otro extremo abierto
            break;
        }
        else
        {
            if (byte == '\n')
            {
                break;
            }

            // Se leen los primeros MAX_COMMAND_SIZE de la línea.
            // El resto de caracteres se descartan.
            if (bytes_read < MAX_COMMAND_SIZE)
            {
                *buffer++ = byte;
                ++bytes_read;
            }
        }
    }

    *buffer = '\0';
    return EXIT_SUCCESS;
}

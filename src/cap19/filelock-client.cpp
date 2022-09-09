// filelock-client.cpp - Cliente del ejemplo del uso de bloqueos de archivos
//
//  El programa servidor utiliza alarm() y las señales del sistema para mostrar periódicamente la hora. Además,
//  crea un archivo con el PID del proceso. El cliente puede usar este archivo para conocer el PID para enviar una
//  señal al servidor.
//
//  Esta técnica es muy usada por los servicios del sistema. Frecuentemente crean un subdirectorio con el nombre del
//  servicio dentro de /var/run. Allí colocan un archivo '.pid' con el PID del proceso, así como otros recursos
//  necesarios para la comunicación con el servicio, como sockets de dominio UNIX o FIFO.
//  El archivo '.pid' permite a los clientes saber si el servicio está en ejecución y mandarle señales para detenerlo
//  o reiniciarlo. 
//
//  Compilar:
//
//      g++ -lfmtlib -o filelock-client filelock-client.cpp
//

#include <unistd.h>
#include <signal.h>

#include <cerrno>       // La librería estándar de C está disponible tanto en cabeceras estilo <stdlib.h> como
#include <cstring>      // <cstdlib>. La primera es para usar con C mientras quela segunda es la recomendada en C++
#include <iostream>     // pues mete las funciones en el espacio de nombres 'std', como el resto de la
#include <fstream>      // librería estándar de C++.
#include <string>
#include <system_error>

#include <fmt/core.h>   // Hasta que std::format (C++20) esté disponible

#include "filelock-server.h"

int protected_main()
{
    std::ifstream pidfile_stream{ PID_FILENAME };
    
    // Comprobamos si se pudo abrir el archivo con el PID.
    // Con la API de la librería estándar de C++ no se pueden conocer más detalles sobre el motivo, como:
    // si el archivo no existe, no se tienen permisos suficientes, etc.
    if ( ! pidfile_stream.is_open() )
    {
        std::cerr << fmt::format( "Error: No se puedo abrir '{}'.\n", PID_FILENAME );
        std::cerr << "Quizás el servidor no se esté ejecutando o no se tengan permisos suficientes\n";
        return EXIT_FAILURE;
    }

    // Usamos std::ifstream::read() para intentar leer el archivo en una sola llamada al sistema read().
    // Es importante hacerlo en una sola llamada porque, como el servidor también escribe en el archivo en una sola
    // operación, así estamos seguros de que la lectura es o antes o después de la escritura (pero nunca en medio)
    // gracias a la semántica de coherencia POSIX. Por tanto, incluso sin usar bloqueo de archivos, el PID se leerá
    // completo o no se leerá nada.
    //
    // NOTA: Para estar seguros, sería mejor usar las funciones open() y read() de la librería del sistema directamente
    // y no std::ifstream

    char buffer[20];
    pidfile_stream.read( buffer, sizeof(buffer) );
    pid_t server_pid = std::stoi( std::string( buffer, pidfile_stream.gcount() ));

    std::cout << "Cerrando el servidor...\n";

    kill( server_pid, SIGTERM );

    std::cout << "¡Adiós!\n";

    return EXIT_SUCCESS;
}

int main()
{
    try
    {
        return protected_main();
    }
    catch(std::exception& e)
    {
        std::cerr << fmt::format( "Error: Excepción: {}\n", e.what() );
    }

    return EXIT_FAILURE;
}

## Ejemplos utilizados durante las clases de Sistemas Operativos

[![CC-0 license](https://img.shields.io/badge/License-CC--0-blue.svg)](https://creativecommons.org/publicdomain/zero/1.0)

### Descripción

En los apuntes de la asignatura y durante las clases de Sistemas Operativos del Grado en Ingeniería Informática, solemos hacer referencia a algunos programas de ejemplo que usamos para ilustrar lo explicado.
Este proyecto es una recopilación de dichos programas.

Concretamente los ejemplos incluidos son:

 * `src/cap06/syscall.s` - Llamadas al sistema en el simulador de MIPS SPIMi.
 * `src/cap06/softstack.c` - Ejemplo básico en C para mostrar la pila de software con `strace`.
 * `src/cap06/softstack.cpp` - Ejemplo básico en C++ para mostrar la pila de software con `strace`.
 * `src/cap09/fork.cpp` - Creación de procesos mediante `fork().
 * `src/cap09/fork-exec.cpp` - Ejecutar otros programas mediante `fork()` y `exec()`.
 * `src/cap10/fifo-client.c` - Cliente en C de comunicación entre procesos mediante tuberías con nombre.
 * `src/cap10/fifo-client.cpp` - Cliente en C++ de comunicación entre procesos mediante tuberías con nombre.
 * `src/cap10/fifo-server.c` - Servidor de comunicación entre procesos mediante tuberías con nombre.
 * `src/cap10/pipe.cpp` - Comunicación entre procesos mediante tuberías.
 * `src/cap10/redir.cpp` - Redirección de E/S estándar usando tuberías.
 * `src/led_test.cpp` - Acceso a la E/S serial en UNIX. Ejemplo con el control de un LED tricolor conectado a un Arduino.
 * `src/mmap.cpp` - Ficheros mapeados en memoria.
 * `src/shared-memory-sync.cpp` - Sincronización mediante semáforos con memoria compartida entre procesos.
 * `src/shared-memory.cpp` - Comunicación mediante memoria compartida
 * `src/threads.cpp` - Creación de hilos en C++.
 * `src/threads-mutex.cpp` - Sincronización de hilos mediante mutex.
 * `src/threads-pthread.cpp` - Creación de hilos mediante POSIX pthread.
 * `src/threads-mutex-pthread.cpp` - Sincronización de hilos mediante mutex de POSIX pthread.
 * `src/otros/yash.cpp` - Ejemplo muy básico del funcionamiento interno de una shell.

### Requisitos

Para la compilación de estos ejemplos en Linux es necesario tener instalados los siguientes paquetes:

 * `build-essential`
 * `cmake`

Mientras que en Microsoft Windows se necesita **Visual Studio Build Tools**, que incluye una versión de **CMake**.

### Compilación

Los ejemplos se compilan de manera sencilla siguiendo estos pasos:

 1. Ir al directorio del proyecto.
 2. Ejecutar `cmake -B build`
 3. Ejecutar `cmake --build build`

En Microsoft Windows estos comandos deben ejecutarse desde la consola de **Developer Command Prompt**.

En cada sistema solo se compilará los ejemplos compatibles.
Los ejecutables resultantes de la compilación son almacenados en la carpeta
`build/Debug/`.

### Autor y contacto

Si tienes cualquier cuestión no dudes en ponerte en contacto [conmigo](mailto:jmtorres@ull.es).

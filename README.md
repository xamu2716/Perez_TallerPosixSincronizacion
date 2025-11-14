# 📘 Taller de Sincronización POSIX  
👤 Autor: **Xamuel Pérez Madrigal**

---

## 🚀 Descripción general

Este taller implementa y prueba **mecanismos de sincronización POSIX** usando:

1. **Procesos + Semáforos POSIX nombrados + Memoria compartida**  
2. **Hilos POSIX (pthread) + mutex + variables de condición**

El objetivo es demostrar cómo coordinar el acceso a datos compartidos, evitando
condiciones de carrera y bloqueos indeseados.

---

## 🗂 Estructura de archivos

- 🔹 `producer.c` – Código del proceso **productor**
- 🔹 `producer.h` – Cabecera con constantes y estructura compartida del productor
- 🔹 `consumer.c` – Código del proceso **consumidor**
- 🔹 `consumer.h` – Cabecera con constantes y estructura compartida del consumidor
- 🔹 `posixSincro.c` – Programa con **hilos productores** y un **hilo spooler**
- 🔹 `posixSincro.h` – Cabecera con constantes y prototipos de `producer` y `spooler`
- 🔹 `Makefile` – Compilación automática de todos los programas

---

## 🟦 Actividad 1: Productor – Consumidor 🧺

### 🧠 Idea

Se implementa el clásico problema **Productor–Consumidor** usando:

- **Semáforos POSIX nombrados**:
  - `/vacio` → cuenta espacios libres en el búfer
  - `/lleno` → cuenta elementos disponibles para consumir
- **Memoria compartida POSIX**:
  - Segmento llamado `/memoria_compartida`
  - Contiene un **búfer circular** de tamaño fijo y dos índices: `entrada` y `salida`

El productor genera **10 elementos** y los inserta en un búfer de tamaño **5**.
Cuando el búfer está lleno, el productor se **bloquea** en `sem_wait(vacio)` hasta
que el consumidor libere espacio.  
El consumidor extrae los datos, los imprime y, al terminar, cierra y elimina
los objetos compartidos (semáforos y memoria).

### 🧾 Estructura compartida

La estructura `compartir_datos` (declarada en `producer.h` y `consumer.h`) contiene:

- `bus[BUFFER]` → arreglo que actúa como búfer circular
- `entrada` → índice de escritura del productor
- `salida` → índice de lectura del consumidor

---

## 🟩 Actividad 2: Sincronización con Hilos POSIX 🧵

### 🧠 Idea

En `posixSincro.c` se crean:

- **10 hilos productores** → generan mensajes de texto
- **1 hilo spooler** → imprime los mensajes de forma ordenada

Se usa:

- `pthread_mutex_t` → para garantizar exclusión mutua sobre el búfer
- `pthread_cond_t buf_cond` → para indicar que hay espacio disponible
- `pthread_cond_t spool_cond` → para indicar que hay líneas listas para imprimir

Cada hilo productor guarda sus cadenas en un arreglo circular global `buf[][]`.
El spooler espera hasta que haya líneas por imprimir y luego las va sacando
del búfer una por una, manteniendo la salida consistente.

---



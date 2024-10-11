# Proyecto 2 - Computacion Paralela y Distribuida

### Prerequisitos
- Unix system
- gcc 

### Dependencias
- libreria Open MPI
- Libreria Open SSL

Instalacion de Open SSL:
1. `brew install openssl`

## Programa Secuencial

`gcc -o bfsec secuencial.c`

`./bfsec`

## Programa Paralelo con BruteForce

### uso

`mpicc -I$(brew --prefix open-mpi)/include -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib bf_txt.c -o bf_txt -lssl -lcrypto`

`mpirun -np 4 bf_txt`

> Nota: Se debe especificar en el archivo `./input.txt`:
> - La llave privada para encriptar
> - El mensaje que encriptaremos
> 3. La *llave publica* (mensaje a buscar dentro del mensaje encriptado)

> Ejemplo:
> 123123
>
> daniflow es dani y el flow
>
> daniflow es dani

## Programa Paralelo de "Fases"
Implementacion paralela de algoritmo de Bruteforce 
Este empieza como cualquier bruteforce, pero luego va aumentando sus rangos de prueba por un exponente
n_i ^ exp. Donde n_i es el numero de proceso y exp es el exponente.

### Uso
`mpicc -I$(brew --prefix open-mpi)/include -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib fases.c -o fases -lssl -lcrypto`

`mpirun -np 4 fases`

> Nota: Se debe especificar en el archivo `./input.txt`

> de la misma manera que en el de arriba.


## Programa Paralelo de "bf2"
 algoritmo para desencriptacion utilizando AllReduce

### Uso

`mpicc -I$(brew --prefix open-mpi)/include -I$(brew --prefix openssl)/include -L$(brew --prefix openssl)/lib bf2.c -o bf2 -lssl -lcrypto`

`mpirun -np 4 bf2`

> Nota: Se debe especificar en el archivo `./input.txt`:
> igual que en el de arribita





//nota: la clave usada es bastante pequeña, cuando sea aleatoria la aceleración variará

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h>

void descifrar(long clave, char *texto_cifrado, int longitud) {
    DES_key_schedule programa_clave;
    DES_cblock clave_des;

    // Establecer paridad de la clave
    for (int i = 0; i < 8; ++i) {
        clave <<= 1;
        clave_des[i] = (clave & (0xFE << i * 8)) >> (i * 8);
    }

    // Inicializar el programa de la clave
    DES_set_odd_parity(&clave_des);
    DES_set_key_checked(&clave_des, &programa_clave);

    // Descifrar los datos
    for (int i = 0; i < longitud; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(texto_cifrado + i), (DES_cblock *)(texto_cifrado + i), &programa_clave, DES_DECRYPT);
    }
}

void mi_cifrado(long clave, char *texto_cifrado, int longitud) {
    DES_key_schedule programa_clave;
    DES_cblock clave_des;

    // Establecer paridad de la clave
    for (int i = 0; i < 8; ++i) {
        clave <<= 1;
        clave_des[i] = (clave & (0xFE << i * 8)) >> (i * 8);
    }

    // Inicializar el programa de la clave
    DES_set_odd_parity(&clave_des);
    DES_set_key_checked(&clave_des, &programa_clave);

    // Cifrar los datos
    for (int i = 0; i < longitud; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(texto_cifrado + i), (DES_cblock *)(texto_cifrado + i), &programa_clave, DES_ENCRYPT);
    }
}

int probarClave(long clave, char *texto_cifrado, int longitud, char busqueda[]){
    char temp[longitud+1];
    memcpy(temp, texto_cifrado, longitud);
    temp[longitud]=0;
    descifrar(clave, temp, longitud);
    return strstr((char *)temp, busqueda) != NULL;
}

int main(int argc, char *argv[]){
    // Inicialización MPI
    int N, id;
    long superior = (1L <<56); //límite superior de claves DES 2^56
    long mi_inferior, mi_superior;
    MPI_Status estado;
    MPI_Request solicitud;
    int bandera;
    char linea_clave[64];
    unsigned char linea_cifrada[64];
    MPI_Comm comunicador = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comunicador, &N);
    MPI_Comm_rank(comunicador, &id);
    double tiempo_inicio, tiempo_fin;

    // Lectura de txt
    char nombre_archivo_entrada[] = "input.txt"; // Nombre del archivo de entrada
    char busqueda[64]; // Cadena de búsqueda

    // Abrir el archivo de entrada
    FILE *archivo_entrada = fopen(nombre_archivo_entrada, "r");
    if (archivo_entrada == NULL) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    // Leer la clave
    if (fgets(linea_clave, sizeof(linea_clave), archivo_entrada) == NULL) {
        perror("Error al leer la clave del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    long clave = atol(linea_clave); // Convertir la clave a long
    if (id == 0){
        printf("------ Cifrado ------\n");
        printf("Clave privada a utilizar: %ld\n", clave);
    }

    // Leer el texto cifrado
    if (fgets(linea_cifrada, sizeof(linea_cifrada), archivo_entrada) == NULL) {
        perror("Error al leer el texto cifrado del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    // Leer cadena de búsqueda
    if (fgets(busqueda, sizeof(busqueda), archivo_entrada) == NULL) {
        perror("Error al leer el texto cifrado del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    if (id == 0){
        printf("Mensaje Original: %s", linea_cifrada);
    }

    // Cifrar texto
    char buffer[512];
    mi_cifrado(clave, linea_cifrada, sizeof(linea_cifrada));
    fclose(archivo_entrada);

    if (id == 0){
        printf("Mensaje Cifrado: %s\n", linea_cifrada);
        printf("\n------ Descifrando por Fases ------\n");
    }

    MPI_Barrier(MPI_COMM_WORLD); // Barrera MPI
    int longitud_cifrado = strlen(linea_cifrada);
    if(id==0){
        tiempo_inicio = MPI_Wtime();
    }

    // Descifrado
    int rango_por_nodo = superior / N;
    mi_inferior = rango_por_nodo * id;
    mi_superior = rango_por_nodo * (id+1) -1;
    if(id == N-1){
        //compensar residuo
        mi_superior = superior;
    }

    long encontrado = 0;

    MPI_Irecv(&encontrado, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comunicador, &solicitud);
    long limite = mi_superior;
    long exp = 1;
    long inicio = mi_superior;

    for(int i = 0; encontrado==0; ++i){
        int indice_temp = i;

        if (mi_superior < superior && i > mi_superior) {
            indice_temp = mi_superior;
            long diferencia = i - mi_superior;

            for (int exp_local = 0; exp_local < exp; exp_local++) {
                indice_temp *= mi_superior;
            }

            indice_temp += diferencia;

            if (indice_temp == limite) {
                exp += 2;
                for (int exp_local = 0; exp_local <= exp; exp_local++) {
                   indice_temp *= mi_superior;
                }
            }
        }

        if(probarClave(indice_temp, (char *)linea_cifrada, longitud_cifrado, busqueda)){
            encontrado = indice_temp;
            for(int nodo = 0; nodo < N; nodo++) {
                MPI_Send(&encontrado, 1, MPI_LONG, nodo, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }

    if (id==0) {
        tiempo_fin = MPI_Wtime();
        MPI_Wait(&solicitud, &estado);
        descifrar(encontrado, (char *)linea_cifrada, longitud_cifrado);
        printf("%li %s\n", encontrado, linea_cifrada);
        printf("Tardó %f ms en ejecutarse\n", (tiempo_fin-tiempo_inicio) * 1000);
    }

    MPI_Finalize();
}
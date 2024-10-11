#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <unistd.h>
#include <openssl/des.h>
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void descifrar(long clave, char *texto_cifrado, int longitud) {
    DES_key_schedule programa_clave;
    DES_cblock clave_des;

    // Establecer la paridad de la clave
    for (int i = 0; i < 8; ++i) {
        clave <<= 1;
        clave_des[i] = (clave & (0xFE << i * 8)) >> (i * 8);
    }

    // Inicializar el programa de claves
    DES_set_odd_parity(&clave_des);
    DES_set_key_checked(&clave_des, &programa_clave);

    // Descifrar los datos
    for (int i = 0; i < longitud; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(texto_cifrado + i), (DES_cblock *)(texto_cifrado + i), &programa_clave, DES_DECRYPT);
    }
}

void mi_cifrado(long clave, char *texto_plano, int longitud) {
    DES_key_schedule programa_clave;
    DES_cblock clave_des;

    // Establecer la paridad de la clave
    for (int i = 0; i < 8; ++i) {
        clave <<= 1;
        clave_des[i] = (clave & (0xFE << i * 8)) >> (i * 8);
    }

    // Inicializar el programa de claves
    DES_set_odd_parity(&clave_des);
    DES_set_key_checked(&clave_des, &programa_clave);

    // Cifrar los datos
    for (int i = 0; i < longitud; i += 8) {
        DES_ecb_encrypt((DES_cblock *)(texto_plano + i), (DES_cblock *)(texto_plano + i), &programa_clave, DES_ENCRYPT);
    }
}

int probar_clave(long clave, char *texto_cifrado, int longitud, char busqueda[]) {
    char temp[longitud+1];
    memcpy(temp, texto_cifrado, longitud);
    temp[longitud] = 0;
    descifrar(clave, temp, longitud);
    return strstr((char *)temp, busqueda) != NULL;
}

int main(int argc, char *argv[]) {
    // Inicialización MPI
    int num_procesos, id_proceso;
    long limite_superior = (1L << 56); // Límite superior de claves DES 2^56
    long mi_inferior, mi_superior;
    MPI_Status estado;
    MPI_Request solicitud;
    int bandera;
    char linea_clave[64];
    unsigned char linea_cifrada[64];
    MPI_Comm comunicador = MPI_COMM_WORLD;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(comunicador, &num_procesos);
    MPI_Comm_rank(comunicador, &id_proceso);
    double tiempo_inicio, tiempo_fin;

    // Lectura del archivo de entrada
    char nombre_archivo_entrada[] = "input.txt";
    char busqueda[64];

    FILE *archivo_entrada = fopen(nombre_archivo_entrada, "r");
    if (archivo_entrada == NULL) {
        perror("Error al abrir el archivo de entrada");
        return 1;
    }

    if (fgets(linea_clave, sizeof(linea_clave), archivo_entrada) == NULL) {
        perror("Error al leer la clave del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    long clave = atol(linea_clave);
    if (id_proceso == 0) {
        printf("------ Proceso de Cifrado ------\n");
        printf("Clave privada a utilizar: %ld\n", clave);
    }

    if (fgets((char*)linea_cifrada, sizeof(linea_cifrada), archivo_entrada) == NULL) {
        perror("Error al leer el texto a cifrar del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    if (fgets(busqueda, sizeof(busqueda), archivo_entrada) == NULL) {
        perror("Error al leer la cadena de búsqueda del archivo");
        fclose(archivo_entrada);
        return 1;
    }

    if (id_proceso == 0) {
        printf("Mensaje Original: %s", linea_cifrada);
    }

    char buffer[512];
    mi_cifrado(clave, (char*)linea_cifrada, sizeof(linea_cifrada));
    fclose(archivo_entrada);

    if (id_proceso == 0) {
        printf("Mensaje Cifrado: %s\n", linea_cifrada);
        printf("\n------ Iniciando Ataque de Fuerza Bruta ------\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);
    int longitud_cifrado = strlen((char*)linea_cifrada);
    if (id_proceso == 0) {
        tiempo_inicio = MPI_Wtime();
    }

    int rango_por_nodo = limite_superior / num_procesos;
    mi_inferior = rango_por_nodo * id_proceso;
    mi_superior = rango_por_nodo * (id_proceso + 1) - 1;
    if (id_proceso == num_procesos - 1) {
        mi_superior = limite_superior;
    }

    long encontrada = 0;

    MPI_Irecv(&encontrada, 1, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, comunicador, &solicitud);

    for (long i = mi_inferior; i < mi_superior && (encontrada == 0); ++i) {
        if (probar_clave(i, (char*)linea_cifrada, longitud_cifrado, busqueda)) {
            encontrada = i;
            for (int nodo = 0; nodo < num_procesos; nodo++) {
                MPI_Send(&encontrada, 1, MPI_LONG, nodo, 0, MPI_COMM_WORLD);
            }
            break;
        }
    }

    if (id_proceso == 0) {
        MPI_Wait(&solicitud, &estado);
        tiempo_fin = MPI_Wtime();
        descifrar(encontrada, (char*)linea_cifrada, longitud_cifrado);
        printf("Clave encontrada: %li\nMensaje descifrado: %s\n", encontrada, linea_cifrada);
        printf("Tiempo de ejecución: %f ms\n", (tiempo_fin - tiempo_inicio) * 1000);
    }

    MPI_Finalize();
}
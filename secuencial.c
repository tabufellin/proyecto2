#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Función para cifrar el texto
void cifrar(char *texto, char *clave) {
    int longTexto = strlen(texto);
    int longClave = strlen(clave);
    for (int i = 0; i < longTexto; i++) {
        texto[i] = texto[i] ^ clave[i % longClave];
    }
}

// Función para descifrar el texto
void descifrar(char *texto, char *clave) {
    cifrar(texto, clave); // El cifrado y el descifrado son idénticos en este caso
}

int main() {
    char *texto = "TextoDePrueba456";
    char textoCifrado[60]; // Aumentamos el tamaño para mayor seguridad
    char clave[60]; // Aumentamos el tamaño para mayor seguridad
    
    // Caracteres alfanuméricos permitidos (incluyendo algunos caracteres especiales)
    char alfanumerico[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!@#$%^&*";

    // Solicitar la clave al usuario
    printf("Introduce la clave: ");
    scanf("%59s", clave);

    // Obtener la longitud de la clave
    int longClave = strlen(clave);
    clave[longClave] = '\0';

    // Copiar el texto original en la variable textoCifrado
    strncpy(textoCifrado, texto, sizeof(textoCifrado) - 1);
    textoCifrado[sizeof(textoCifrado) - 1] = '\0';

    // Cifrar el texto con la clave proporcionada
    cifrar(textoCifrado, clave);

    // Medir el tiempo y realizar la búsqueda de la clave
    clock_t tiempo_inicio = clock();

    // Implementación de la búsqueda por fuerza bruta
    char claveEncontrada[60];
    for (int i = 0; i < longClave; i++) {
        claveEncontrada[i] = alfanumerico[0];
    }
    claveEncontrada[longClave] = '\0';

    while (strcmp(claveEncontrada, clave) != 0) {
        // Generar la siguiente clave candidata
        for (int i = longClave - 1; i >= 0; i--) {
            int indice = strchr(alfanumerico, claveEncontrada[i]) - alfanumerico;
            if (indice < strlen(alfanumerico) - 1) {
                claveEncontrada[i] = alfanumerico[indice + 1];
                break;
            } else {
                claveEncontrada[i] = alfanumerico[0];
            }
        }
    }

    clock_t tiempo_fin = clock();
    double tiempo_transcurrido = (double)(tiempo_fin - tiempo_inicio) / CLOCKS_PER_SEC;

    // Imprimir resultados
    printf("Longitud de la clave: %d, Clave encontrada: %s, Tiempo transcurrido: %f segundos\n", 
           longClave, claveEncontrada, tiempo_transcurrido);

    return 0;
}
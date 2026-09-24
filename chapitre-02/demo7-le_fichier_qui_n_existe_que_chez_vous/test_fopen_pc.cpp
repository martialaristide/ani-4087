#include <cstdio>
#include <cstring>
#include <cerrno>

int main() {
    printf("Tentative d'ouverture de \"assets/donnees.txt\" avec fopen()...\n");

    FILE *f = fopen("assets/donnees.txt", "r");

    if (f) {
        printf("fopen() a REUSSI !\n");
        char buffer[256] = {0};
        fread(buffer, 1, sizeof(buffer) - 1, f);
        printf("Contenu lu : %s\n", buffer);
        fclose(f);
    } else {
        printf("fopen() a ECHOUE (errno=%d: %s)\n", errno, strerror(errno));
    }

    return 0;
}

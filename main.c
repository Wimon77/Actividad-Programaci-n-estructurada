/*
Programa: Registro de votantes
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>


/* Structs */
// Direccion votantes 
typedef struct {
    char calle[64];
    char cruzamientos[64];
    char colonia[64];
} DireccionVotante;

// Direccion dependientes
typedef struct {
    char calle[64];
    char cruzamientos[64];
    char colonia[64];
} DireccionDependiente;

// Dependiente 
typedef struct {
    int edad;                   
    char nombre[40];
    char apellidopat[40];
    char apellidomat[40];
    char curp[19];              
    DireccionDependiente direccion;
} Dependiente;

// Votantes
typedef struct {
    int edad;                   
    char nombre[40];
    char apellidopat[40];
    char apellidomat[40];
    char curp[19];              
    int distrito;               
    char celular[20];
    char email[60];
    DireccionVotante direccion;
    Dependiente *deps;          
    int numDeps;
} Votante;


Votante *lista = NULL; 
int nVotantes = 0;      
int capVotantes = 0;    
const int MAX_DEPS = 2;
const char *ARCHIVO = "votantes.txt";

// funciones 


void trim_newline(char *s){
    if(!s) return;
    size_t L = strlen(s);
    if(L>0 && s[L-1]=='\n') s[L-1]='\0';
}

// Leer cadena
void leer_cadena(const char *msg, char *dest, int maxlen){
    while(1){
        printf("%s", msg);
        if(!fgets(dest, maxlen, stdin)){
            clearerr(stdin);
            continue;
        }
        trim_newline(dest);
        if(strlen(dest)==0){
            printf("No dejes el campo vacio. Intenta otra vez.\n");
            continue;
        }
        if(strchr(dest, '|')){
            printf("Por favor no uses el caracter '|' en los campos.\n");
            continue;
        }
        return;
    }
}

// Leer entero con rango 
int leer_entero_rango(const char *msg, int min, int max){
    char buf[128];
    int val;
    while(1){
        printf("%s", msg);
        if(!fgets(buf, sizeof(buf), stdin)){ clearerr(stdin); continue; }
        if(sscanf(buf, "%d", &val)==1 && val>=min && val<=max) return val;
        printf("Valor invalido. Debe estar entre %d y %d.\n", min, max);
    }
}


void scopy(char *dest, const char *src, int maxlen){
    strncpy(dest, src, maxlen-1);
    dest[maxlen-1]='\0';
}

//Validacion de curp

int extraer_fecha_de_curp(const char *curp, int *anio, int *mes, int *dia){
    if(!curp || strlen(curp) < 10) return 0;
    char yy[3] = {curp[4], curp[5], '\0'};
    char mm[3] = {curp[6], curp[7], '\0'};
    char dd[3] = {curp[8], curp[9], '\0'};
    if(!isdigit(yy[0]) || !isdigit(yy[1]) || !isdigit(mm[0]) || !isdigit(mm[1]) || !isdigit(dd[0]) || !isdigit(dd[1]))
        return 0;
    int y = atoi(yy);
    int m = atoi(mm);
    int d = atoi(dd);
    if(m<1 || m>12 || d<1 || d>31) return 0;
    //siglo
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int curYY = (now->tm_year + 1900) % 100;
    int fullYear = (y <= curYY) ? 2000 + y : 1900 + y;
    *anio = fullYear; *mes = m; *dia = d;
    return 1;
}

int edad_desde_fecha(int anio, int mes, int dia){
    time_t t = time(NULL);
    struct tm *now = localtime(&t);
    int edad = (now->tm_year + 1900) - anio;
    if(mes > (now->tm_mon+1) || (mes == (now->tm_mon+1) && dia > now->tm_mday)) edad--;
    return edad;
}

int edad_desde_curp(const char *curp){
    int a,m,d;
    if(!extraer_fecha_de_curp(curp, &a, &m, &d)) return -1;
    return edad_desde_fecha(a,m,d);
}

// Verifica que el curp corresponda a edad en rango
int verificar_edad_por_curp(const char *curp, int min, int max){
    int e = edad_desde_curp(curp);
    if(e < 0) return 0;
    if(e < min) return 0;
    if(max >= 0 && e > max) return 0;
    return 1;
}

/* Gestion de arreglo dinamico */

// Asegura que 'lista' tenga capacidad para al menos nVotantes+1
void asegurar_capacidad(){
    if(nVotantes < capVotantes) return;
    int nueva = (capVotantes==0) ? 4 : capVotantes * 2;
    Votante *tmp = (Votante*) realloc(lista, nueva * sizeof(Votante));
    if(!tmp){
        printf("Error: memoria insuficiente\n");
        exit(1);
    }
    lista = tmp;
    capVotantes = nueva;
}

//Buscar votante por curp
int buscar_votante_por_curp(const char *curp){
    for(int i=0;i<nVotantes;i++){
        if(strcmp(lista[i].curp, curp)==0) return i;
    }
    return -1;
}

//Buscar dependiente por curp
int buscar_dependiente_por_curp(const Votante *v, const char *curp){
    for(int i=0;i<v->numDeps;i++){
        if(strcmp(v->deps[i].curp, curp)==0) return i;
    }
    return -1;
}

/* funciones principales */

//Agregar un votante
void agregar_votante(){
    Votante v;
    memset(&v, 0, sizeof(Votante));
    v.numDeps = 0;
    v.deps = (Dependiente*) malloc(MAX_DEPS * sizeof(Dependiente));
    if(!v.deps){ printf("Error de memoria\n"); return; }

    // CURP y validacion de longitud
    while(1){
        leer_cadena("CURP (18 caracteres): ", v.curp, sizeof(v.curp));
        if(strlen(v.curp) != 18) {
            printf("La CURP debe tener 18 caracteres.\n"); continue;
        }
        if(buscar_votante_por_curp(v.curp) >= 0){
            printf("Ya existe un votante con esa CURP.\n"); continue;
        }
        // verificar con el curp que es mayor o igual a 18
        if(!verificar_edad_por_curp(v.curp, 18, -1)){
            printf("La fecha en la CURP indica edad menor a 18. Debe ser mayor de 18.\n"); continue;
        }
        break;
    }

    leer_cadena("Nombre: ", v.nombre, sizeof(v.nombre));
    leer_cadena("Apellido paterno: ", v.apellidopat, sizeof(v.apellidopat));
    leer_cadena("Apellido materno: ", v.apellidomat, sizeof(v.apellidomat));

    // Edad, en esta parte se ingres la edad y debe coincidir con el curp
    while(1){
        v.edad = leer_entero_rango("Edad (>=18): ", 18, 120);
        int edadCurp = edad_desde_curp(v.curp);
        if(edadCurp < 0){ printf("Curp invalida para calcular edad.\n"); continue; }
        if(v.edad != edadCurp){
            printf("La edad ingresada (%d) no coincide con la edad del curp (%d).\n", v.edad, edadCurp);
            continue;
        }
        break;
    }

    v.distrito = leer_entero_rango("Distrito (1-9): ", 1, 9);

    // Celular y verificacion
    while(1){
        leer_cadena("Celular: ", v.celular, sizeof(v.celular));
        char ver[64];
        leer_cadena("Repite celular: ", ver, sizeof(ver));
        if(strcmp(v.celular, ver) == 0) break;
        printf("No coinciden los celulares. Intenta de nuevo.\n");
    }

    // Email y verificacion
    while(1){
        leer_cadena("Email: ", v.email, sizeof(v.email));
        char ver[64];
        leer_cadena("Repite email: ", ver, sizeof(ver));
        if(strcmp(v.email, ver) == 0) break;
        printf("No coinciden los emails. Intenta de nuevo.\n");
    }

    // Direccion del votante
    printf("== Direccion del votante ==\n");
    leer_cadena("Calle: ", v.direccion.calle, sizeof(v.direccion.calle));
    leer_cadena("Cruzamientos: ", v.direccion.cruzamientos, sizeof(v.direccion.cruzamientos));
    leer_cadena("Colonia: ", v.direccion.colonia, sizeof(v.direccion.colonia));

    // arreglo dinamico
    asegurar_capacidad();
    lista[nVotantes] = v;
    nVotantes++;
    printf("Votante agregado correctamente.\n");
}

//Actualizar informacion de un votante ya registrado
void actualizar_votante(){
    char curp[19];
    leer_cadena("Curp del votante a actualizar: ", curp, sizeof(curp));
    int idx = buscar_votante_por_curp(curp);
    if(idx < 0){ printf("Votante no encontrado.\n"); return; }
    Votante *v = &lista[idx];

    printf("Dejar(ENTER) para no cambiar un campo.\n");
    char buffer[128];
    printf("Nombre actual: %s\nNuevo nombre: ", v->nombre);
    if(fgets(buffer, sizeof(buffer), stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(v->nombre, buffer, sizeof(v->nombre)); }
    printf("Apellido paterno actual: %s\nNuevo: ", v->apellidopat);
    if(fgets(buffer, sizeof(buffer), stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(v->apellidopat, buffer, sizeof(v->apellidopat)); }
    printf("Apellido materno actual: %s\nNuevo: ", v->apellidomat);
    if(fgets(buffer, sizeof(buffer), stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(v->apellidomat, buffer, sizeof(v->apellidomat)); }

    printf("Edad actual: %d\nNueva edad (ENTER para conservar): ", v->edad);
    if(fgets(buffer, sizeof(buffer), stdin)){
        trim_newline(buffer);
        if(strlen(buffer)>0){
            int nv = atoi(buffer);
            int edadCurp = edad_desde_curp(v->curp);
            if(nv >= 18 && nv == edadCurp) v->edad = nv;
            else printf("No se actualiza la edad: debe coincidir con la CURP y ser >=18.\n");
        }
    }

    printf("Distrito actual: %d\nNuevo (1-9 o ENTER): ", v->distrito);
    if(fgets(buffer, sizeof(buffer), stdin)){
        trim_newline(buffer);
        if(strlen(buffer)>0){
            int d = atoi(buffer);
            if(d>=1 && d<=9) v->distrito = d;
            else printf("Distrito invalido.\n");
        }
    }

    printf("øCambiar celular? (s/n): ");
    if(fgets(buffer,sizeof(buffer),stdin) && (buffer[0]=='s' || buffer[0]=='S')){
        while(1){
            char ver[64];
            leer_cadena("Nuevo celular: ", v->celular, sizeof(v->celular));
            leer_cadena("Repite celular: ", ver, sizeof(ver));
            if(strcmp(v->celular, ver)==0) break;
            printf("No coinciden.\n");
        }
    }

    printf("øCambiar email? (s/n): ");
    if(fgets(buffer,sizeof(buffer),stdin) && (buffer[0]=='s' || buffer[0]=='S')){
        while(1){
            char ver[64];
            leer_cadena("Nuevo email: ", v->email, sizeof(v->email));
            leer_cadena("Repite email: ", ver, sizeof(ver));
            if(strcmp(v->email, ver)==0) break;
            printf("No coinciden.\n");
        }
    }

    // Actualizar direccion
    printf("== Actualizar direccion ==\n");
    leer_cadena("Calle: ", v->direccion.calle, sizeof(v->direccion.calle));
    leer_cadena("Cruzamientos: ", v->direccion.cruzamientos, sizeof(v->direccion.cruzamientos));
    leer_cadena("Colonia: ", v->direccion.colonia, sizeof(v->direccion.colonia));

    printf("Votante actualizado.\n");
}

// Elimina un votante ya registrado 
void eliminar_votante(){
    char curp[19];
    leer_cadena("Curp del votante a eliminar: ", curp, sizeof(curp));
    int idx = buscar_votante_por_curp(curp);
    if(idx < 0){ printf("Votante no encontrado.\n"); return; }

    // liberar memoria de dependientes del votante
    free(lista[idx].deps);

    // mover elementos a la izquierda
    for(int i=idx;i<nVotantes-1;i++) lista[i]=lista[i+1];
    nVotantes--;
    printf("Votante eliminado.\n");
}

//Agregar maximo 2 dependientes por votante 
void agregar_dependiente(){
    char curpV[19];
    leer_cadena("Curp del votante due√±o del dependiente: ", curpV, sizeof(curpV));
    int idx = buscar_votante_por_curp(curpV);
    if(idx < 0){ printf("Votante no encontrado.\n"); return; }
    Votante *v = &lista[idx];
    if(v->numDeps >= MAX_DEPS){ printf("Ya tiene %d dependientes.\n", MAX_DEPS); return; }

    Dependiente d;
    memset(&d,0,sizeof(d));
    while(1){
        leer_cadena("Curp del dependiente: ", d.curp, sizeof(d.curp));
        if(strlen(d.curp) != 18){ printf("Curp debe tener 18 caracteres.\n"); continue; }
        if(!verificar_edad_por_curp(d.curp, 10, 17)){
            printf("La curp no corresponde a edad entre 10 y 17.\n"); continue;
        }
        break;
    }

    leer_cadena("Nombre del dependiente: ", d.nombre, sizeof(d.nombre));
    leer_cadena("Apellido paterno: ", d.apellidopat, sizeof(d.apellidopat));
    leer_cadena("Apellido materno: ", d.apellidomat, sizeof(d.apellidomat));

    while(1){
        d.edad = leer_entero_rango("Edad del dependiente (10-17): ", 10, 17);
        int ecurp = edad_desde_curp(d.curp);
        if(ecurp < 0){ printf("Curp invalida para fecha.\n"); continue; }
        if(d.edad != ecurp){ printf("La edad no coincide con la Curp (edad Curp=%d). Intenta otra vez.\n", ecurp); continue; }
        break;
    }

    // direccion del dependiente
    leer_cadena("Calle: ", d.direccion.calle, sizeof(d.direccion.calle));
    leer_cadena("Cruzamientos: ", d.direccion.cruzamientos, sizeof(d.direccion.cruzamientos));
    leer_cadena("Colonia : ", d.direccion.colonia, sizeof(d.direccion.colonia));

    // agregar al votante
    v->deps = (Dependiente*) realloc(v->deps, (v->numDeps + 1) * sizeof(Dependiente));
    if(!v->deps){ printf("Error de memoria\n"); return; }
    v->deps[v->numDeps] = d;
    v->numDeps++;
    printf("Dependiente agregado.\n");
}

//Actualizar la informacion de los dependientes ya registrados
void actualizar_dependiente(){
    char curpV[19], curpD[19];
    leer_cadena("Curp del votante due√±o: ", curpV, sizeof(curpV));
    int idxV = buscar_votante_por_curp(curpV);
    if(idxV < 0){ printf("Votante no encontrado.\n"); return; }
    Votante *v = &lista[idxV];
    leer_cadena("Curp del dependiente a actualizar: ", curpD, sizeof(curpD));
    int idxD = buscar_dependiente_por_curp(v, curpD);
    if(idxD < 0){ printf("Dependiente no encontrado.\n"); return; }

    Dependiente *d = &v->deps[idxD];
    printf("Dejar (ENTER) para no cambiar.\n");
    char buffer[128];
    printf("Nombre actual: %s\nNuevo: ", d->nombre);
    if(fgets(buffer,sizeof(buffer),stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(d->nombre, buffer, sizeof(d->nombre)); }
    printf("Apellido paterno actual: %s\nNuevo: ", d->apellidopat);
    if(fgets(buffer,sizeof(buffer),stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(d->apellidopat, buffer, sizeof(d->apellidopat)); }
    printf("Apellido materno actual: %s\nNuevo: ", d->apellidomat);
    if(fgets(buffer,sizeof(buffer),stdin)){ trim_newline(buffer); if(strlen(buffer)>0) scopy(d->apellidomat, buffer, sizeof(d->apellidomat)); }

    printf("Edad actual: %d\nNueva (10-17 o ENTER): ", d->edad);
    if(fgets(buffer,sizeof(buffer),stdin)){
        trim_newline(buffer);
        if(strlen(buffer)>0){
            int nv = atoi(buffer);
            int ecurp = edad_desde_curp(d->curp);
            if(nv>=10 && nv<=17 && nv==ecurp) d->edad = nv;
            else printf("No se actualiza: edad invalida o no coincide con Curp.\n");
        }
    }

    // actualizar direccion
    leer_cadena("Calle (dep): ", d->direccion.calle, sizeof(d->direccion.calle));
    leer_cadena("Cruzamientos (dep): ", d->direccion.cruzamientos, sizeof(d->direccion.cruzamientos));
    leer_cadena("Colonia (dep): ", d->direccion.colonia, sizeof(d->direccion.colonia));

    printf("Dependiente actualizado.\n");
}

/* Eliminar a los dependientes ya registrados */
void eliminar_dependiente(){
    char curpV[19], curpD[19];
    leer_cadena("Curp del votante due√±o: ", curpV, sizeof(curpV));
    int idxV = buscar_votante_por_curp(curpV);
    if(idxV < 0){ printf("Votante no encontrado.\n"); return; }
    Votante *v = &lista[idxV];
    leer_cadena("Curp del dependiente a eliminar: ", curpD, sizeof(curpD));
    int idxD = buscar_dependiente_por_curp(v, curpD);
    if(idxD < 0){ printf("Dependiente no encontrado.\n"); return; }

    // mover dependientes a la izquierda
    for(int i=idxD;i<v->numDeps-1;i++) v->deps[i] = v->deps[i+1];
    v->numDeps--;
    v->deps = (Dependiente*) realloc(v->deps, (v->numDeps) * sizeof(Dependiente));
    if(v->numDeps==0){
        free(v->deps);
        v->deps = NULL;
    }
    printf("Dependiente eliminado.\n");
}

//Funcioon que permita verificar la edad mediante el curp
void funcion_verificar_edad_por_curp(){
    char curp[19];
    leer_cadena("Teclea CURP para verificar edad: ", curp, sizeof(curp));
    int edad = edad_desde_curp(curp);
    if(edad < 0) printf("Curp invalida o no se pudo extraer fecha.\n");
    else printf("Edad inferida por curp: %d\n", edad);
}

/*guardar y cargar archivo*/

void guardar_archivo(){
    FILE *f = fopen(ARCHIVO, "w");
    if(!f){ printf("No se pudo abrir %s para escribir.\n", ARCHIVO); return; }
    fprintf(f, "VOTANTES|%d\n", nVotantes);
    for(int i=0;i<nVotantes;i++){
        Votante *v = &lista[i];
        fprintf(f, "VOTANTE|%s|%s|%s|%s|%d|%d|%s|%s\n", v->curp, v->nombre, v->apellidopat, v->apellidomat, v->edad, v->distrito, v->celular, v->email);
        fprintf(f, "DIRV|%s|%s|%s\n", v->direccion.calle, v->direccion.cruzamientos, v->direccion.colonia);
        fprintf(f, "DEPS|%d\n", v->numDeps);
        for(int j=0;j<v->numDeps;j++){
            Dependiente *d = &v->deps[j];
            fprintf(f, "DEP|%s|%s|%s|%s|%d\n", d->curp, d->nombre, d->apellidopat, d->apellidomat, d->edad);
            fprintf(f, "DIRD|%s|%s|%s\n", d->direccion.calle, d->direccion.cruzamientos, d->direccion.colonia);
        }
    }
    fclose(f);
    printf("Datos guardados en %s\n", ARCHIVO);
}

//  cargar archivo
int split_line(char *line, char **tokens, int maxTokens){
    int cnt = 0;
    char *p = strtok(line, "|");
    while(p && cnt < maxTokens){
        tokens[cnt++] = p;
        p = strtok(NULL, "|");
    }
    return cnt;
}

void cargar_archivo(){
    FILE *f = fopen(ARCHIVO, "r");
    if(!f) return; // si no existe, no pasa nada
    char line[512];
    if(!fgets(line, sizeof(line), f)){ fclose(f); return; }
    trim_newline(line);
    int total = 0;
    if(sscanf(line, "VOTANTES|%d", &total) != 1){ fclose(f); return; }
    for(int i=0;i<total;i++){
        if(!fgets(line, sizeof(line), f)) break;
        trim_newline(line);
        char *tok[12];
        int nt = split_line(line, tok, 12);
        if(nt < 9) break;
        
        Votante v;
        memset(&v,0,sizeof(v));
        v.deps = NULL; v.numDeps = 0;
        scopy(v.curp, tok[1], sizeof(v.curp));
        scopy(v.nombre, tok[2], sizeof(v.nombre));
        scopy(v.apellidopat, tok[3], sizeof(v.apellidopat));
        scopy(v.apellidomat, tok[4], sizeof(v.apellidomat));
        v.edad = atoi(tok[5]);
        v.distrito = atoi(tok[6]);
        scopy(v.celular, tok[7], sizeof(v.celular));
        scopy(v.email, tok[8], sizeof(v.email));

        // leer DIRV
        if(!fgets(line, sizeof(line), f)) break;
        trim_newline(line);
        char *tokd[8];
        int nd = split_line(line, tokd, 8);
        if(nd>=4) { scopy(v.direccion.calle, tokd[1], sizeof(v.direccion.calle)); scopy(v.direccion.cruzamientos, tokd[2], sizeof(v.direccion.cruzamientos)); scopy(v.direccion.colonia, tokd[3], sizeof(v.direccion.colonia)); }

        // leer DEPS|k
        if(!fgets(line, sizeof(line), f)) break;
        trim_newline(line);
        int k = 0;
        if(sscanf(line, "DEPS|%d", &k) != 1) k = 0;
        if(k > MAX_DEPS) k = MAX_DEPS;
        for(int j=0;j<k;j++){
            if(!fgets(line, sizeof(line), f)) break;
            trim_newline(line);
            char *tokp[12];
            int np = split_line(line, tokp, 12);
            if(np < 6) break;
            Dependiente d;
            memset(&d,0,sizeof(d));
            scopy(d.curp, tokp[1], sizeof(d.curp));
            scopy(d.nombre, tokp[2], sizeof(d.nombre));
            scopy(d.apellidopat, tokp[3], sizeof(d.apellidopat));
            scopy(d.apellidomat, tokp[4], sizeof(d.apellidomat));
            d.edad = atoi(tokp[5]);

            // leer DIRD
            if(!fgets(line, sizeof(line), f)) break;
            trim_newline(line);
            char *tokdd[8];
            int ndd = split_line(line, tokdd, 8);
            if(ndd>=4){ scopy(d.direccion.calle, tokdd[1], sizeof(d.direccion.calle)); scopy(d.direccion.cruzamientos, tokdd[2], sizeof(d.direccion.cruzamientos)); scopy(d.direccion.colonia, tokdd[3], sizeof(d.direccion.colonia)); }

            // anexar
            v.deps = (Dependiente*) realloc(v.deps, (v.numDeps+1) * sizeof(Dependiente));
            v.deps[v.numDeps] = d;
            v.numDeps++;
        }

        // anexar votante a lista
        asegurar_capacidad();
        lista[nVotantes] = v;
        nVotantes++;
    }
    fclose(f);
}

// menus

void mostrar_menu(){
    printf("\n===== MENU =====\n");
    printf("1) Agregar votante\n");
    printf("2) Actualizar votante\n");
    printf("3) Eliminar votante\n");
    printf("4) Agregar dependiente (max 2)\n");
    printf("5) Actualizar dependiente\n");
    printf("6) Eliminar dependiente\n");
    printf("7) Verificar edad por CURP\n");
    printf("8) Guardar datos en archivo\n");
    printf("9) Salir (guardar antes)\n");
    printf("=================\n");
}

//MAIN

int main(){
    // Cargar archivo si existe
    cargar_archivo();

    int op;
    do{
        mostrar_menu();
        op = leer_entero_rango("Opcion: ", 1, 9);
        switch(op){
            case 1: agregar_votante(); break;
            case 2: actualizar_votante(); break;
            case 3: eliminar_votante(); break;
            case 4: agregar_dependiente(); break;
            case 5: actualizar_dependiente(); break;
            case 6: eliminar_dependiente(); break;
            case 7: funcion_verificar_edad_por_curp(); break;
            case 8: guardar_archivo(); break;
            case 9: guardar_archivo(); break;
            default: printf("Opcion invalida.\n");
        }
    } while(op != 9);

    // liberar memoria
    for(int i=0;i<nVotantes;i++){
        free(lista[i].deps);
    }
    free(lista);
    printf("Programa finalizado.\n");
    return 0;
}

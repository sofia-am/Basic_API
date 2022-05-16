/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include <jansson.h>
#include <pwd.h>

#define PORT 8538

int acumulador;
/**
 * Decode a u_map into a string
 */
char * print_map(const struct _u_map * map) {
  char * line, * to_return = NULL;
  const char **keys, * value;
  int len, i;
  if (map != NULL) {
    keys = u_map_enum_keys(map);
    for (i=0; keys[i] != NULL; i++) {
      value = u_map_get(map, keys[i]);
      len = snprintf(NULL, 0, "key is %s, value is %s", keys[i], value);
      line = malloc((len+1)*sizeof(char));
      snprintf(line, (len+1), "key is %s, value is %s", keys[i], value);
      if (to_return != NULL) {
        len = strlen(to_return) + strlen(line) + 1;
        to_return = realloc(to_return, (len+1)*sizeof(char));
        if (strlen(to_return) > 0) {
          strcat(to_return, "\n");
        }
      } else {
        to_return = malloc((strlen(line) + 1)*sizeof(char));
        to_return[0] = 0;
      }
      strcat(to_return, line);
      free(line);
    }
    return to_return;
  } else {
    return NULL;
  }
}

/**
 * Callback function for the web application on /prueba url call
 */
int incrementar_contador(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data) {
  acumulador++;

  ulfius_set_string_body_response(response, 200, "Ok\n");
  return U_CALLBACK_CONTINUE;
}

int devolver_contador(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data){
  char *string = malloc(sizeof(char)*30);
  sprintf(string, "El valor del contador es %d\n", acumulador);
  ulfius_set_string_body_response(response, 200, string);
  return U_CALLBACK_CONTINUE;
}

int agregar_usuario(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data){
  acumulador++;

  ulfius_set_string_body_response(response, 200, "Ok\n");
  return U_CALLBACK_CONTINUE;
}

int listar_usuarios(__attribute__((unused))const struct _u_request * request, struct _u_response * response,__attribute__((unused)) void * user_data) {
  int status;
  json_t *usuarios = json_object();
  json_t *array = json_array();
  char *data = "data";
  json_t *root = json_string(data);
  struct passwd *p;

  if((status = json_array_append(array, root)) == -1){
    perror("Error al insertar valor dentro del array");
  }

  setpwent();
  while((p = getpwent())) {
    json_t *value = json_real((double)(p->pw_uid));
    status = json_object_set(usuarios, p->pw_name, value);
   
    if(status == -1){
      perror("Error al crear JSON de usuarios");
      exit(1);
    }
  }

  if((status = json_array_append(array, usuarios)) == -1){
    perror("Error al insertar valores dentro del array");
  }
  
  ulfius_set_json_body_response(response, 200, array);
  if(status == -1){
      perror("Error al setear respuesta");
      exit(1);
  }

  free(array);
  free(usuarios);
  return U_CALLBACK_CONTINUE;
}
/**
 * main function
 */
int main(void) {
  struct _u_instance instance;
  acumulador = 0;

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK) {
    fprintf(stderr, "Error al inicializar instancia \n");
    return(1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/print_cont", NULL, 0, &devolver_contador, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", "/usuario", NULL, 0, &listar_usuarios, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/inc", NULL, 0, &incrementar_contador, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/usuario", NULL, 0, &agregar_usuario, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK) {
    printf("Start framework on port %d\n", instance.port);
    // Wait for the user to press <enter> on the console to quit the application
    //getchar();
  } else {
    fprintf(stderr, "Error starting framework\n");
    exit(0);
  }
  
  while(1){

  }
  //printf("End framework\n");

  //ulfius_stop_framework(&instance);
  //ulfius_clean_instance(&instance);

  //return 0;
}
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

#define PORT 8537

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
/*   char *url_params = print_map(request->map_url), *headers = print_map(request->map_header); //*post_params = print_map(request->map_post_body);
  printf("La información que obtuve es: \nurl_params: %s, \nheaders: \n%s, \n", url_params, headers);
  json_t *params = json_object();
  json_error_t * error;
  params = ulfius_get_json_body_request(request, error);
  const char *key;
  json_t *value;

  json_object_foreach(params, key, value){
      printf("Par clave valor: |%s||%s|\n", key, json_string_value(value));
  } */
  int status;
  json_t *usuarios = json_object();
  struct passwd *p;
  setpwent();

  while((p = getpwent())) {
    json_t *value = json_real((double)(p->pw_uid));
    status = json_object_set(usuarios, p->pw_name, value);
   
    if(status == -1){
      perror("Error al crear JSON de usuarios");
      exit(1);
    }
  }

  ulfius_set_json_body_response(response, 200, usuarios);
  if(status == -1){
      perror("Error al setear respuesta");
      exit(1);
  }

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
    fprintf(stderr, "Error al inicializar instancia (GET)\n");
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
  }
  
  while(1){

  }
  //printf("End framework\n");

  //ulfius_stop_framework(&instance);
  //ulfius_clean_instance(&instance);

  //return 0;
}
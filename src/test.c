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
static uid_t euid, ruid;
/*Chequea si la string ingresada tiene caracteres entre [A-Za-z0-9].
  Retorna 1 si cumple, 0 si tiene otros caracteres.
*/
int validarString(char *string){
  int i;
  for (i = 0; string[i] != '\0'; i++)
  {
    if (!(string[i] >= 65 && string[i] <= 90) && !(string[i] >= 97 && string[i] <= 122) && !(string[i] >= 48 && string[i] <= 57))
    {
      return 0;
    }
  }
  return 1;
}

/* Set the effective UID to the real UID. */
void do_setuid (void)
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid (euid);
#else
  status = setreuid (ruid, euid);
#endif
  if (status < 0) {
    fprintf (stderr, "Couldn't set uid.\n");
    exit (status);
    }
}

/* Restore the effective UID to its original value. */
void undo_setuid (void)
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid (ruid);
#else
  status = setreuid (euid, ruid);
#endif
  if (status < 0) {
    fprintf (stderr, "Couldn't unset uid.\n");
    exit (status);
    }
}

/**
 * Callback function for the web application on /prueba url call
 */
int incrementar_contador(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data) {
  acumulador++;
/*   json_t *request = json_object();
  request = ulfius_get_json_body_request(); */
  json_t *respuesta = json_object();

  json_object_set(respuesta, "code", json_integer(200));
  json_object_set(respuesta, "description", json_integer(acumulador));

  ulfius_set_json_body_response(response, 200, respuesta);
  return U_CALLBACK_CONTINUE;
}

int devolver_contador(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data){
  json_t *respuesta = json_object();

  json_object_set(respuesta, "code", json_integer(200));
  json_object_set(respuesta, "description", json_integer(acumulador));

  ulfius_set_json_body_response(response, 200, respuesta);
  return U_CALLBACK_CONTINUE;
}

int agregar_usuario(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data){
  acumulador++;
  json_t *json_request = json_object();
  json_error_t error;
  json_request = ulfius_get_json_body_request(request, &error);
  //json_t *value = json_object();
  //printf("Valores: nombre: %s | pass %s\n", json_string_value(json_object_get(json_request, "username")), json_string_value(json_object_get(json_request, "password")));
  char *user = json_string_value(json_object_get(json_request, "username"));
  char *password =json_string_value(json_object_get(json_request, "password"));

  if(validarString(user) && validarString(user)){
    printf("password %s | user %s\n", password, user);
    struct passwd *usuarios = getpwent();
    struct passwd nuevo_usuario;
    usuarios = getpwnam(user); //chequeamos si no existe un usuario con ese nombre
    if(usuarios != NULL){
      ulfius_set_string_body_response(response, 409,"{ \"error\": {\"status_code\": 409,\"status\": \"User already exists\"}}");
      return U_CALLBACK_CONTINUE;
    }
    nuevo_usuario.pw_name = user;
    nuevo_usuario.pw_passwd = password;

    do_setuid();
    FILE *f = fopen("/etc/passwd", "a");
    undo_setuid();

    if(f != NULL){
      putpwent(&nuevo_usuario, f);
      fclose(f);
    }else{
      perror("Error al abrir el archivo");
      exit(1);
    }
  }else{
    ulfius_set_string_body_response(response, 409,"{ \"error\": {\"status_code\": 409,\"status\":\"Invalid username/password\"}}");
    return U_CALLBACK_CONTINUE;
  }

  ulfius_set_string_body_response(response, 200, "OK\n");
  return U_CALLBACK_CONTINUE;
}

int listar_usuarios(__attribute__((unused))const struct _u_request * request, struct _u_response * response,__attribute__((unused)) void * user_data) {
  int status;
  json_t *usuarios = json_object();
  json_t *array = json_object();
  char *data = "data";
  //json_t *root = json_string(data);
  struct passwd *p;

  if((status = json_object_set(array, data, usuarios)) == -1){
    perror("Error al insertar valor");
  }

  setpwent();
  while((p = getpwent())) {
    json_t *value = json_integer(p->pw_uid);
    status = json_object_set(usuarios, p->pw_name, value);
   
    if(status == -1){
      perror("Error al crear JSON de usuarios");
      exit(1);
    }
  }

/*   if((status = json_array_append(array, usuarios)) == -1){
    perror("Error al insertar valores dentro del array");
  } */
  
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
  ruid = getuid();
  euid = 0;
  undo_setuid();

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
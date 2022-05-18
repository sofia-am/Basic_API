#include <stdio.h>
#include <string.h>
#include <ulfius.h>
#include <jansson.h>
#include <pwd.h>
#include <time.h>
#include <yder.h>

#define PORT 8538

int acumulador;
static uid_t euid, ruid;
/*Chequea si la string ingresada tiene caracteres entre [A-Za-z0-9].
  Retorna 1 si cumple, 0 si tiene otros caracteres.
*/
int validarString(char *string)
{
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
void do_setuid(void)
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid(euid);
#else
  status = setreuid(ruid, euid);
#endif
  if (status < 0)
  {
    fprintf(stderr, "Couldn't set uid.\n");
    exit(status);
  }
}

/* Restore the effective UID to its original value. */
void undo_setuid(void)
{
  int status;

#ifdef _POSIX_SAVED_IDS
  status = seteuid(ruid);
#else
  status = setreuid(euid, ruid);
#endif
  if (status < 0)
  {
    fprintf(stderr, "Couldn't unset uid.\n");
    exit(status);
  }
}

/**
 * Callback function for the web application on /prueba url call
 */
int incrementar_contador(__attribute__((unused)) const struct _u_request *request, struct _u_response *response, __attribute__((unused)) void *user_data)
{
  acumulador++;
  json_t *respuesta = json_object();
  struct _u_map *map = request->map_header;
  const char *value = (char*)u_map_get(map, "X-Forwarded-For");
  //printf("IP es %s\n", value);

  json_object_set(respuesta, "code", json_integer(200));
  json_object_set(respuesta, "description", json_integer(acumulador));

  y_log_message(Y_LOG_LEVEL_INFO, "Usuario creado desde: %s", value);
  ulfius_set_json_body_response(response, 200, respuesta);
  return U_CALLBACK_CONTINUE;
}

int devolver_contador(__attribute__((unused)) const struct _u_request *request, struct _u_response *response, __attribute__((unused)) void *user_data)
{
  json_t *respuesta = json_object();

  json_object_set(respuesta, "code", json_integer(200));
  json_object_set(respuesta, "description", json_integer(acumulador));

  //y_log_message(Y_LOG_LEVEL_INFO, "prueba");
  ulfius_set_json_body_response(response, 200, respuesta);
  return U_CALLBACK_CONTINUE;
}

int agregar_usuario(__attribute__((unused)) const struct _u_request *request, struct _u_response *response, __attribute__((unused)) void *user_data)
{
  json_t *json_request = json_object();
  json_error_t error;
  json_request = ulfius_get_json_body_request(request, &error);
  char *user = (char *)json_string_value(json_object_get(json_request, "username"));
  char *password = (char *)json_string_value(json_object_get(json_request, "password"));

  if (validarString(user) && validarString(user))
  {
    struct passwd *usuarios = getpwent();
    usuarios = getpwnam(user); // chequeamos si no existe un usuario con ese nombre

    if (usuarios != NULL)
    {
      y_log_message(Y_LOG_LEVEL_ERROR, "Usuario %d ya existe.", usuarios->pw_uid);
      ulfius_set_string_body_response(response, 409, "{ \"error\": {\"status_code\": 409,\"status\": \"User already exists\"}}");
      return U_CALLBACK_CONTINUE;
    }

    char *command = malloc(sizeof(char) * 40);
    sprintf(command, "sudo useradd -p %s %s", password, user);
    system(command);

    usuarios = getpwnam(user);

    //---------------------------- realiza una request al servicio Contador de Usuarios -------------------------------
    struct _u_request *req = malloc(sizeof(struct _u_request));
    struct _u_response *res = malloc(sizeof(struct _u_response));

    ulfius_init_request(req);
    ulfius_init_response(res);

    req->http_verb =  "POST";
    req->http_url = "http://contadordeusuarios.com/contador/increment";
    req->auth_basic_user = "user1";
    req->auth_basic_password = "root";

    ulfius_send_http_request(req, res);

    if(res->status == 200){
      printf("OK\n");
    }else{
      printf("Error la hacer la request, error: %ld\n", res->status);
      y_log_message(Y_LOG_LEVEL_ERROR, "Error al hacer la request al servicio Contador de Usuarios");
    }
    //-----------------------------------------------------------------------------------------------------------------

    time_t t;
    t = time(NULL);
    struct tm tm = *localtime(&t);
    char *time_string = malloc(sizeof(char) * 20);
    sprintf(time_string, "%d-%d-%d %d:%d:%d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    json_t *respuesta = json_object();
    json_object_set(respuesta, "id", json_integer(usuarios->pw_uid));
    json_object_set(respuesta, "username", json_string(user));
    json_object_set(respuesta, "created_at", json_string(time_string));

    y_log_message(Y_LOG_LEVEL_INFO, "Usuario %d creado", usuarios->pw_uid);
    ulfius_set_json_body_response(response, 200, respuesta);
    return U_CALLBACK_CONTINUE;
  }
  else
  {
    y_log_message(Y_LOG_LEVEL_ERROR, "Usuario o password inválidos");
    ulfius_set_string_body_response(response, 409, "{ \"error\": {\"status_code\": 409,\"status\":\"Invalid username/password\"}}");
    return U_CALLBACK_CONTINUE;
  }
}

int listar_usuarios(__attribute__((unused)) const struct _u_request *request, struct _u_response *response, __attribute__((unused)) void *user_data)
{
  int status;
  json_t *array = json_array();
  char *data = "data";
  json_t *respuesta = json_object();
  //json_t *root = json_string(data);
  int i = 0;
  struct passwd *p;
/* 
  if ((status =  json_array_append(array, root)) == -1)
  {
    perror("Error al insertar valor");
    exit(1);
  } */

  setpwent();
  while ((p = getpwent()))
  {
    json_t *usuarios = json_object();
    status = json_object_set(usuarios, "user_id", json_integer(p->pw_uid)); 
    status = json_object_set(usuarios, "username", json_string(p->pw_name));
    i++;
    if(status == -1)
    {
      perror("Error al crear JSON de usuarios");
      exit(1);
    }
    if((status = json_array_append(array, usuarios)) == -1)
    {
      perror("Error al insertar valores dentro del array");
      exit(1);
    }
  }
  endpwent();

  json_object_set(respuesta, data, array);
  
  y_log_message(Y_LOG_LEVEL_INFO, "Usuarios creados: %d", i);

  ulfius_set_json_body_response(response, 200, respuesta);
  if (status == -1)
  {
    perror("Error al setear respuesta");
    exit(1);
  }

  free(array);
  //free(usuarios);
  return U_CALLBACK_CONTINUE;
}

/**
 * main function
 */
int main(void)
{
  y_init_logs("API_log", Y_LOG_MODE_FILE, Y_LOG_LEVEL_INFO, "/home/sofia/Documents/OperativosII/soii---2022---laboratorio-vi-sofia-am/log/log_info.log", "Inicializando el log");

  struct _u_instance instance;
  acumulador = 0;
  ruid = getuid();
  euid = 0;
  undo_setuid();

  // Initialize instance with the port number
  if (ulfius_init_instance(&instance, PORT, NULL, NULL) != U_OK)
  {
    fprintf(stderr, "Error al inicializar instancia \n");
    return (1);
  }

  // Endpoint list declaration
  ulfius_add_endpoint_by_val(&instance, "GET", "/print_cont", NULL, 0, &devolver_contador, NULL);
  ulfius_add_endpoint_by_val(&instance, "GET", "/usuario", NULL, 0, &listar_usuarios, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/inc", NULL, 0, &incrementar_contador, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/usuario", NULL, 0, &agregar_usuario, NULL);

  // Start the framework
  if (ulfius_start_framework(&instance) == U_OK)
  {
    printf("Start framework on port %d\n", instance.port);
    // Wait for the user to press <enter> on the console to quit the application
    // getchar();
  }
  else
  {
    fprintf(stderr, "Error starting framework\n");
    exit(0);
  }

  while (1)
  {
  }

  // TODO: signal handling para cerrar los archivos
  
  // printf("End framework\n");

  // ulfius_stop_framework(&instance);
  // ulfius_clean_instance(&instance);

  // return 0;
}
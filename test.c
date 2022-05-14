/**
 * test.c
 * Small Hello World! example
 * to compile with gcc, run the following command
 * gcc -o test test.c -lulfius
 */
#include <stdio.h>
#include <ulfius.h>

#define PORT 8537

int acumulador;

/**
 * Callback function for the web application on /prueba url call
 */
int callback_set(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data) {
  acumulador++;
  ulfius_set_string_body_response(response, 200, "Ok\n");
  return U_CALLBACK_CONTINUE;
}

/* int callback_post_test (const struct _u_request * request, struct _u_response * response, void * user_data) {
  char * post_params = print_map(request->map_post_body);
  char * response_body = msprintf("Hello World!\n%s", post_params);
  ulfius_set_string_body_response(response, 200, response_body);
  o_free(response_body);
  o_free(post_params);
  return U_CALLBACK_CONTINUE;
} */

int callback_get(__attribute__((unused))const struct _u_request * request, struct _u_response * response, __attribute__((unused))void * user_data){
  char *string = malloc(sizeof(char)*30);
  sprintf(string, "El valor del contador es %d\n", acumulador);
  ulfius_set_string_body_response(response, 200, string);
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
  ulfius_add_endpoint_by_val(&instance, "GET", "/imprimir", NULL, 0, &callback_get, NULL);
  ulfius_add_endpoint_by_val(&instance, "POST", "/increment", NULL, 0, &callback_set, NULL);

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
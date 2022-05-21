### Lab6 Sistemas Operativos II
## Ingeniería en Computación - FCEFyN - UNC
### Alumna: Sofía Amallo

# Sistemas Embebidos

### Servicio de Usuarios
Este servicio se encargará de crear usuarios y listarlos. Estos usuarios deberán poder _logearse_ vía _SSH_ luego de su creación.

#### POST /api/users
Endpoints para la creación de usuario en el sistema operativo:

Request
```C    
        curl --request POST \
            --url http://lab3.com/api/users \
            -u user1:root \
            --header 'accept: application/json' \
            --header 'content-type: application/json' \
            --data '{"username": "myuser", "password": "mypassword"}'
```
Respuesta
```C

        {
            "id": 142,
            "username": "myuser",
            "created_at": "2019-06-22 02:19:59"
        }

```
El _\<Mensaje\>_ para el log será: _Usuario \<Id\> creado_.

Cada vez que se cree un usuario, este servicio deberá incrementar el contador en el servicio de contador de usuarios. 
En caso que el servicio de contador no este diposnible o falle, logear al falla.
  
#### GET /api/users
Endpoint para obtener todos los usuarios del sistema operativo y sus identificadores.

Request
```C
    curl --request GET \
        --url  http://lab3.com/api/users \
        -u user1:root \
        --header 'accept: application/json' \
        --header 'content-type: application/json'
```
Respuesta
```C
    {
      "data": [
          {
              "user_id": 2,
              "username": "user1",  
          },
          {
              "user_id": 1,
              "username": "user2"
          },
          ...
      ]
    }
```
El  _\<Mensaje\>_ para el log será:  _Usuario creados: \<cantidad de usuario del SO\>_
 
### Servicio de contador
Este microservicio pose los endpoint del laboratorio 5, lleva un contador. Este contador debe ser implementado con el mismo _Media Type_ , `application/json`.

#### POST /contador/increment

Request

```C
    curl --request POST \
        --url  http://contadordeusuarios.com/contador/increment \
        -u user1:root \
        --header 'accept: application/json' \
        --header 'content-type: application/json'
```

Respuesta
```C
    {
        "code": 200,
        "description": "<new_value>"
    }
```

El  _\<Mensaje\>_ para el log será:  _Contador Incrementado desde: \<El ip del cliente que incremento el contador.\>_


#### GET /contador/value
Este endpoint permite saber el valor actual del contador

Request
```C
    curl --request GET \
        --url  http://contadordeusuarios.com/contador/value \
        -u user1:root \
        --header 'accept: application/json' \
        --header 'content-type: application/json'
```
Respuesta

```C
    {
        "code": 200,
        "description": "<value>"
    }
```

Este endpoint no tiene ningún requerimiento para logging.

#
Para probar que los usuarios creados funcionan:

    ssh <nombre-de-usuario>@<IP>

Para probar la rotación del log:

    logrotate -d /etc/logrotate.d/log_config.log 
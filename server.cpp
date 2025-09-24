#include <iostream>
#include <sys/socket.h> // Para sockets en general
#include <sys/un.h> // Para los sockets UNIX 
#include <fcntl.h>
#include <unistd.h>
#include <vector>

using namespace std;

int MAX_CLIENTS = 0;
int MAX_MSG_SIZE = 1024;

vector<int> clients_fd;

// Ejemplo con UNIX sockets
int main(){
    // 1. Creamos el socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        cerr << "Error al crear el socket" << endl;
        return 1;
    }

    // Get current flags
    int flags = fcntl(server_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL)");
        cerr << "Error al leer flags del socket. Se cerrará el servidor." << endl;
        close(server_fd);
        return 1;
    }

    // Set O_NONBLOCK flag
    if (fcntl(server_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl(F_SETFL)");
        cerr << "Error al establecer non-blocking. Se cerrará el servidor." << endl;
        close(server_fd);
        return 1;
    }

    // 2. Vinculamos el socket a una dirección (en este caso, como un socket UNIX, usamos un archivo)
    sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "/tmp/socket-chat-nb");
    unlink(address.sun_path);  // Aseguramos que el socket no exista 

    // el :: es para salir del namespace std en esta llamada ya que existe un std::bind que no es el que queremos usar
    int bind_result = ::bind(server_fd, (sockaddr*)&address, sizeof(address)); 
    if (bind_result < 0) {
        cerr << "Error al vincular el socket" << endl;
        return 1;
    }

    // 3. Escuchamos conexiones entrantes
    int listen_result = listen(server_fd, MAX_CLIENTS); // 5 es el número máximo de conexiones en cola
    if (listen_result < 0) {
        cerr << "Error al escuchar en el socket" << endl;
        return 1;
    } else {
        cout << "Escuchando en el socket...\n" << endl;
    }

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd >= 0) {
            cout << "Nueva conexión, socket " << client_fd << ".\n" << endl;

            // Get current flags
            flags = fcntl(client_fd, F_GETFL, 0);
            if (flags < 0) {
                perror("fcntl(F_GETFL)");
                cerr << "Error al leer flags del socket de cliente. Se cerrará el cliente y el servidor." << endl;
                close(client_fd);
                close(server_fd);
                return 1;
            }

            // Set O_NONBLOCK flag
            if (fcntl(client_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("fcntl(F_SETFL)");
                cerr << "Error al establecer non-blocking de cliente. Se cerrará el cliente." << endl;
                close(client_fd);
            } else {
                clients_fd.push_back(client_fd);

                const char* welcome = "Te damos la bienvenida. Este servidor es no-bloqueante. Escribí /quit para salir.";
                send(client_fd, welcome, strlen(welcome), 0);
            }
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                perror("accept");
                // tuki :3
            }
        }

        for (int i = 0; i < clients_fd.size(); i++) {
            int client = clients_fd[i];

            char buffer[MAX_MSG_SIZE];

            ssize_t bytes_received = recv(client, buffer, sizeof(buffer), 0);
            if (bytes_received < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    continue;
                } else {
                    cout << "Error al recibir datos del socket " << client << "." << endl;
                    close(client);
                    clients_fd.erase(clients_fd.begin() + i);
                }
            } else if (bytes_received == 0) {
                cout << "Cliente con socket " << client << " desconectado." << endl;

                close(client);
                clients_fd.erase(clients_fd.begin() + i);
            } else {
                buffer[bytes_received] = '\0';
                cout << "Socket " << client << ": " << buffer << endl;
            }
        }
    }

    close(server_fd);

    return 0;
}
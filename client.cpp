#include <iostream>
#include <sys/socket.h> // Para sockets en general
#include <sys/un.h> // Para los sockets UNIX 
#include <unistd.h>
#include <thread>
#include <mutex>

using namespace std;

int MAX_MSG_SIZE = 1024;

bool connected = true;
mutex connected_mutex;

void sender(int client_fd) {
    // 3. Enviamos un mensaje al servidor usando send

    while (1) {
        string input_usuario;

        getline(cin, input_usuario);

        unique_lock<mutex> lock_a(connected_mutex);
        if (!connected) {
            break;
        }
        lock_a.unlock();

        if (input_usuario == "") {
            continue;
        }

        const char* message = input_usuario.c_str();

        ssize_t bytes_sent = send(client_fd, message, strlen(message), 0);
        if (bytes_sent == -1) {
            cerr << "Error al enviar datos al socket" << endl;
            exit(1);
        }

        if (input_usuario == "/quit") {
            unique_lock<mutex> lock_b(connected_mutex);
            connected = false;
            lock_b.unlock();

            // 5. Cerramos el socket
            close(client_fd);
            break;
        }
    }
}

void receiver(int client_fd) {
    // 4. Leemos la respuesta del servidor usando recv

    while (1) {
        char buffer[MAX_MSG_SIZE];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

        // wrapper para el mutex q se desbloquea automaticamente cuando se sale del scope
        unique_lock<mutex> lock(connected_mutex);
        if (!connected) {
            break;
        }
        if (string(buffer, bytes_received) == "/client_quit") {
            connected = false;

            cout << "--- CONEXIÓN CERRADA O RECHAZADA ---\nPresione cualquier tecla para continuar..." << endl;

            break;
        }
        lock.unlock();

        if (bytes_received == -1) {
            cerr << "Error al recibir datos del socket" << endl;
            exit(1);
        }

        cout << string(buffer, bytes_received) << endl;
    }
}

// Ejemplo con UNIX sockets
int main(){
    // 1. Creamos el socket
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        cerr << "Error al crear el socket" << endl;
        return 1;
    }

    // 2. Conectamos al servidor
    sockaddr_un address;
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, "/tmp/socket-chat-nb");

    int connect_result = connect(client_fd, (sockaddr*)&address, sizeof(address));
    if (connect_result == -1) {
        cerr << "Error al conectar al servidor" << endl;
        return 1;
    }

    // 2 y medio. threads de comunicación sender y receiver
    thread sender_t(sender, client_fd);
    thread receiver_t(receiver, client_fd);

    sender_t.join();
    receiver_t.join();

    return 0;
}

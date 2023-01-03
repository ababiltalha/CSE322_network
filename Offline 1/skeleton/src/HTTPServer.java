import java.io.*;
import java.net.*;

public class HTTPServer {
    static final int PORT = 5077;

    public static void main(String[] args) {
        // Create a server socket and bind it to a port
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Server started.\nListening for connections on port : " + PORT + " ...\n");
            while (true) {
                // Listen for incoming client connections
                Socket clientSocket = serverSocket.accept();
                System.out.println("Connection received");
                // Create a new thread for the client
                Thread serverWorker = new ServerWorker(clientSocket);
                serverWorker.start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}

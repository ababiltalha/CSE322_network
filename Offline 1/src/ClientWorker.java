import java.io.*;
import java.net.Socket;

public class ClientWorker extends Thread{
    private final static int PORT = 5077;
    private final Socket serverSocket;
    private final BufferedReader in;
    private final PrintWriter pr;
    private final DataInputStream dataIn;
    private final DataOutputStream dataOut;

    private final static int chunkSize = 32;
    private final String request;

    public ClientWorker(String request) throws IOException {
        this.request = request;
        this.serverSocket = new Socket("localhost", PORT);
        this.in = new BufferedReader(new InputStreamReader(serverSocket.getInputStream()));
        this.pr = new PrintWriter(serverSocket.getOutputStream(), true);
        this.dataIn = new DataInputStream(serverSocket.getInputStream());
        this.dataOut = new DataOutputStream(serverSocket.getOutputStream());
    }

    public void uploadFile(File file) throws IOException {
        int bytes = 0;
        FileInputStream fileUpload = new FileInputStream(file);
        this.dataOut.writeLong(file.length());
        this.dataOut.flush();
        byte[] fileData = new byte[chunkSize];
        while ((bytes = fileUpload.read(fileData)) != -1) {
            // send the file to server
            dataOut.write(fileData, 0, bytes);
            dataOut.flush();
        }
        // close the file here
        fileUpload.close();
    }

    @Override
    public void run() {
        if (!request.startsWith("UPLOAD")) {
            pr.println(this.request);
            try {
                String response = in.readLine();
                System.out.println(response);
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                try {
                    this.serverSocket.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        } else {
            String fileName = request.split(" ")[1];
            File file = new File(fileName);
            if (!file.exists()) {
                System.out.println("File not found");
            } else {
                pr.println(this.request);
                try {
                    System.out.println("Uploading file...");
                    String response = in.readLine();
                    System.out.println(response);
                    if (response.equals("OK")) {
                        System.out.println("Server acknowledged the request");
                        pr.println("UPLOADING " + fileName);
                        uploadFile(file);
                        System.out.println("Upload complete");
                    } else if (response.equals("ERROR")) {
                        System.out.println("Server did not acknowledge the request");
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    try {
                        serverSocket.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }
    }


}

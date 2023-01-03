import java.io.*;
import java.net.*;
import java.util.Date;

class ServerWorker extends Thread {
    private final Socket clientSocket;
    private final BufferedReader in;
    private final PrintWriter pr;
    private final DataInputStream dataIn;
    private final DataOutputStream dataOut;
    private final static int chunkSize = 32;


    public ServerWorker(Socket clientSocket) throws IOException {
        this.clientSocket = clientSocket;
        this.in = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        this.pr = new PrintWriter(clientSocket.getOutputStream(), true);
        this.dataIn = new DataInputStream(clientSocket.getInputStream());
        this.dataOut = new DataOutputStream(clientSocket.getOutputStream());
        writeLog("Connection received\n");
    }

    public static String readFileData(File file, int fileLength) throws IOException {
        FileInputStream fileIn = null;
        byte[] fileData = new byte[fileLength];

        try {
            fileIn = new FileInputStream(file);
            fileIn.read(fileData);
        } finally {
            if (fileIn != null)
                fileIn.close();
        }

        return String.valueOf(fileData);
    }

    public void writeLog(String str){
        try {
            FileWriter fw = new FileWriter("log.txt", true);
            fw.write(str);
            fw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    @Override
    public void run() {
        try {
            // read data from the client and send a response
            String input = in.readLine();
//                if(input.startsWith("GET") ||  input.startsWith("UPLOAD"))
            System.out.println("input : " + input);
            writeLog("input : " + input + "\n");
//              String content = readFileData(new File("index.html"), 1000);
            if (input == null) return;
            if (input.length() > 0) {
                if (input.startsWith("GET")) handleGetRequest(input);
                else if (input.startsWith("UPLOAD")) {
                    System.out.println("Upload debug");
                    handleUploadRequest(input);
                }
                else {
                    System.out.println("Invalid request");
                    sendResponse("400 Bad Request");
                }
            }
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            try {
                clientSocket.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }

    public void handleGetRequest(String input) {
        String response = "";
//        System.out.println("GET request : " + input);
        String[] tokens = input.split(" ");
        String filePath = tokens[1];
        try {
            if(filePath.endsWith(".docx") || filePath.endsWith(".txt") || filePath.endsWith(".mp4") ||
                    filePath.endsWith(".pdf") || filePath.endsWith(".jpeg") ||
                    filePath.endsWith(".jpg") || filePath.endsWith(".png")) {
                File file = new File(filePath.substring(1));
                if(file.exists()){
                    int fileLength = (int) file.length();
                    response += getResponseHeader(200, "OK", getContentType(filePath), fileLength);
                    sendResponse(response);
                    writeLog("\n\nResponse : \n" + response);

                    // send file data
                    FileInputStream fileIn = new FileInputStream(file);
                    byte[] fileData = new byte[chunkSize];
                    int bytes = 0;
                    while((bytes = fileIn.read(fileData)) != -1) {
                        dataOut.write(fileData, 0, bytes);
                        dataOut.flush();
                    }
                }
            } else {
                String content = getHTMLContent(filePath);
                response += getResponseHeader(200, "OK", "text/html", content.length()) + content;
                sendResponse(response);
                writeLog("\n\nResponse : \n" + response);
            }
        } catch (FileNotFoundException e) {
            response += getResponseHeader(404, "Not Found", "", 0);
            sendResponse(response);
            writeLog("\n\nResponse : \n" + response);
        } catch (Exception e) {
//            e.printStackTrace();
        }
    }

    public void handleUploadRequest(String input) {
        String response = "";
        long fileSize = -1;
        String[] tokens = input.split(" ", 2);
        if (tokens.length != 2) {
            sendResponse("ERROR");
            writeLog("\n\nResponse : ERROR\n");
            return;
        }
//        sendResponse("OK");
        pr.println("OK");
        pr.flush();
        System.out.println("Acknowledge sent");
        String fileName = tokens[1];
        try {
            if(in.readLine().startsWith("UPLOADING ")){
                int bytes = 0;
                FileOutputStream fileOut = new FileOutputStream("client/"+fileName);
                fileSize = dataIn.readLong();
                System.out.println("fileSize : " + fileSize + " bytes");
                byte[] fileData = new byte[chunkSize];
                while (fileSize > 0 &&
                        (bytes = this.dataIn.read(fileData, 0, (int) Math.min(fileData.length, fileSize))) != -1) {
                    fileOut.write(fileData, 0, bytes);
                    fileSize -= bytes;
                }
                fileOut.close();
                System.out.println("Upload complete");
            } else System.out.println("Upload failed");
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (fileSize == 0) {
                response += getResponseHeader(200, "OK", "", 0);
            } else {
                response += getResponseHeader(417, "Upload failed", "", 0);
            }
            sendResponse(response);
            writeLog("\n\nResponse : \n" + response);
        }
    }

    public String getResponseHeader(int statusCode, String statusMessage, String contentType, int contentLength) {
        String response = "";
        response += "HTTP/1.1 " + statusCode + " " + statusMessage + "\r\n";
        response += "Server: Java HTTP Server: 1.0\r\n";
        response += "Date: " + new Date() + "\r\n";
        response += "Content-Type: " + contentType + "\r\n";
        response += "Content-Length: " + contentLength + "\r\n";
        response += "\r\n";
        return response;
    }

    public void sendResponse(String response) {
        pr.write(response);
        pr.flush();
    }

    public String getContentType(String fileName) {
        String contentType = "";
        if(fileName.endsWith(".html")) contentType = "text/html";
        else if(fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) contentType = "image/jpeg";
        else if(fileName.endsWith(".png")) contentType = "image/png";
//        else if(fileName.endsWith(".mp4")) contentType = "video/mp4";
        else if(fileName.endsWith(".pdf")) contentType = "application/pdf";
        else if(fileName.endsWith(".txt") || fileName.endsWith(".docx")) contentType = "text/plain";
        else contentType = "application/octet-stream";
        return contentType;
    }

    public String getHTMLContent(String fileName) throws FileNotFoundException {
        String html = "";

        // home directory
        if(fileName.equals("/"))
            return "<h1>/</h1><ul><li><a href=\"root\">root</a></li><li><a href=\"client\">client</a></li></ul>";

        File folder = new File(fileName.substring(1));
        if(folder.exists()) {
            File[] listOfFiles = folder.listFiles();
            html += "<h1>" + fileName + "</h1>";
            if(listOfFiles == null) return html;
            html += "<ul>";
            // make list of the files in the directory
            for (File file :
                    listOfFiles) {
                html += "<li><a href=\"" + fileName + "/" + file.getName() + "\">" + file.getName() + "</a></li>";
            }
            html += "</ul>";
            return html;
        } else throw new FileNotFoundException();
    }



}
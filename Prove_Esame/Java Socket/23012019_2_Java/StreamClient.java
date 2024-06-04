import java.io.*;
import java.net.*;

public class StreamClient {
    public static void main(String[] args) {
        InetAddress addr = null;
        int port = -1;

        Socket socket = null;
        DataInputStream inSock = null;
        DataOutputStream outSock = null;

        BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));

        String buffer;

        try {
            if (args.length == 2) {
                addr = InetAddress.getByName(args[0]);
                port = Integer.parseInt(args[1]);
                if (port < 1024 || port > 65535) {
                    System.out.println("[ERROR] la porta deve essere compresa nel range valido [1024-65535 ");
                    System.exit(1);
                }
            } else {
                System.out.println("[ERROR] java StreamClient serverAddr serverPort ");
                System.exit(1);
            }
        } catch (Exception e) {
            System.out.println("[ERROR] Problemi, i seguenti: ");
            e.printStackTrace();
            System.out.println("java StreamClient serverAddr serverPort ");
            System.exit(2);
        }

        // CREAZIONE SOCKET
        try {
            socket = new Socket(addr, port);
            socket.setSoTimeout(300000);
            System.out.println("[DEBUG] Creata la socket: " + socket + " sulla porta locale " + socket.getLocalPort());
        } catch (Exception e) {
            System.out.println("[ERROR] Problemi nella creazione della socket: ");
            e.printStackTrace();
            System.exit(1);

        }
        // CREAZIONE STREAM
        try {
            inSock = new DataInputStream(socket.getInputStream());
            outSock = new DataOutputStream(socket.getOutputStream());
        } catch (IOException e) {
            System.out.println("[ERROR] Problemi nella creazione degli stream su socket: ");
            e.printStackTrace();
            System.exit(1);
        }

        // CICLO INTERAZIONE CLIENT

        String targa;
        Boolean success;

        String fileName;
        int fileNumber, nread, buffer_size = 4096;
        long fileLength;
        boolean found;
        FileOutputStream fileStream;
        byte[] byteBuffer = new byte[buffer_size];
        try {
            System.out.println("Servizi: 1: elimina prenotazione, 2: download foto prenotazione ^D per terminare");

            while ((buffer = stdIn.readLine()) != null) {

                if (buffer.equals("1")) { // Servizio 1
                    outSock.writeChar('1');

                    System.out.println("inserire il numero di targa: ");
                    targa = stdIn.readLine();
                    // input check
                    if (targa.length() != 7) {
                        System.out.println("input not valid: " + targa);
                        continue;
                    } else {
                        outSock.writeUTF(targa);
                    }

                    success = inSock.readBoolean();

                    // risposta dal server
                    if (success) {
                        System.out.println("Successo, rimozione avvenuta");
                    } else {
                        System.out.println("non ho trovato la targa richiesta: " + targa);
                    }

                } else if (buffer.equals("2")) { // Servizio 2
                    outSock.writeChar('2');

                    System.out.println("inserire il numero di targa: ");
                    targa = stdIn.readLine();
                    // input check
                    if (targa.length() != 7) {
                        System.out.println("input not valid: " + targa);
                        continue;
                    } else {
                        outSock.writeUTF(targa);
                    }

                    found = inSock.readBoolean();

                    if (found) {
                        fileNumber = inSock.readInt();
                        for (int i = 0; i < fileNumber; i++) {
                            fileName = inSock.readUTF();

                            fileLength = inSock.readLong();

                            fileStream = new FileOutputStream(fileName);

                            while (fileLength > 0) {
                                nread = inSock.read(byteBuffer, 0,
                                        fileLength > buffer_size ? buffer_size : (int) fileLength);
                                fileStream.write(byteBuffer, 0, nread);
                                fileLength -= nread;
                            }

                            System.out.println("ricevuto: " + fileName);
                            fileStream.close();
                        }
                    }

                } else
                    System.out.println("Nessun metodo corrispondente");

                System.out.println("Servizi: 1: elimina prenotazione, 2: download foto prenotazione ^D per terminare");

            }

        } catch (Exception e1) {
            System.out.println("Problemi nel ciclo di interazione");
            e1.printStackTrace();
        }

        // fine ciclo

        // CHIUSURA SOCKET
        try {
            socket.shutdownOutput();
            socket.shutdownInput();
            socket.close();
        } catch (IOException e) {
            System.out.println("Problemi nella chiusura delle socket");
            e.printStackTrace();
            System.exit(1);
        }

    }// main

}
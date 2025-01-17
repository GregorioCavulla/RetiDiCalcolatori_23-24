import java.io.*;
import java.net.*;

public class StreamThread extends Thread {

    private Socket clientSocket = null; // socket per comunicare con il client
    private String[][] dati; // struttura dati condivisa tra i thread

    public StreamThread(Socket socket, String[][] dati) {
        this.clientSocket = socket;
        this.dati = dati;
    }

    public StreamThread(Socket socket) {
        this.clientSocket = socket;
    }

    @Override
    public void run() {
        System.out.println("[DEBUG] Attivazione figlio: " + Thread.currentThread().getName());

        DataInputStream inSock; // stream di input
        DataOutputStream outSock; // stream di output

        // CREAZIONE STREAM INPUT E OUTPUT
        try {
            inSock = new DataInputStream(clientSocket.getInputStream());
            outSock = new DataOutputStream(clientSocket.getOutputStream());
        } catch (IOException ioe) {
            System.out.println("[ERROR] Problemi nella creazione degli stream di input/output ");
            ioe.printStackTrace();
            return;
        }

        // CORPO DELLA THREAD
        try {
            while (true) {
                System.out.println("[DEBUG] pre ricezione");
                char service = inSock.readChar();
                System.out.println("[DEBUG]ricevuto richiesta di servizio " + service);

                if (service == '1') {

                    int fileTrovati = -1;

                    String dirName;
                    dirName = inSock.readUTF();

                    char carInput;
                    carInput = inSock.readChar();

                    int occInput;
                    occInput = inSock.readInt();

                    System.out.println(
                            "[DEBUG]: dirname: " + dirName + ", car: " + carInput + ", occ: " + occInput + ".");

                    File dir;
                    int occ;
                    String[] files = { "" };
                    fileTrovati = 0;

                    dir = new File(dirName);

                    if (dir.isDirectory() && dir.exists() && dir.canRead()) {
                        for (String f : dir.list()) {
                            occ = 0;
                            for (char car : f.toCharArray()) {
                                if (car == carInput) {
                                    occ++;
                                    if (occ == occInput) {
                                        files[fileTrovati] = f;
                                        fileTrovati++;
                                    }
                                }
                            }
                        }
                    }

                    outSock.writeInt(fileTrovati);

                    for (String f : files) {
                        System.out.println("trovato: " + f);
                        outSock.writeUTF(f);
                        break;
                    }

                    outSock.writeInt(-1);

                    // FINE
                } else if (service == '2') {
                    // SERVIZIO 2

                    // FINE;
                }
            } // fine ciclo interazione

        } catch (EOFException e) {
            System.out.println("Terminazione. (Thread=" + getName() + ")");
        } catch (SocketTimeoutException e) {
            System.out.println("[ERROR]: Socket timed out. (Thread=" + getName() + ")");
            e.printStackTrace();
        } catch (IOException e) {
            System.out.println("[ERROR]: Lettura/Scrittura su stream fallita. (Thread=" + getName() + ")");
            e.printStackTrace();
        } catch (Exception e) {
            System.out.println("[ERROR]: Errore irreversibile. (Thread=" + getName() + ")");
            e.printStackTrace();
        } finally {
            System.out.println("[DEBUG]: Terminazione server thread. (Thread=" + getName() + ")");
            try {
                clientSocket.close();
            } catch (IOException e) {
                System.out.println("[ERROR]: Chiusura socket fallita. (Thread=" + getName() + ")");
                e.printStackTrace();
            }
        }
    }
}
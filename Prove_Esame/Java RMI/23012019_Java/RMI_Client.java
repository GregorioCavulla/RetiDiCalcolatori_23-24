
/**
 * Client.java
 */

import java.rmi.*;
import java.io.*;

class RMI_Client {

	public static void main(String[] args) {
		int registryPort = 1099; // Porta di default del registry RMI
		String registryHost = null; // Host del registry RMI, di default localhost
		String serviceName = "ServerRMI"; // Nome del servizio RMI, di default ServerRMI

		BufferedReader stdIn = new BufferedReader(new InputStreamReader(System.in));

		// CONTROLLO I PARAMETRI
		if (args.length != 1 && args.length != 2) {
			System.out.println("[ERROR]: Client RegistryHost [registryPort]");
			System.exit(1);
		} else {
			registryHost = args[0];
			if (args.length == 2) {
				try {
					registryPort = Integer.parseInt(args[1]);
				} catch (Exception e) {
					System.out.println("[ERROR]: Client NomeHost [registryPort], registryPort deve essere un intero");
					System.exit(1);
				}
			}
		}

		System.out.println("[DEBUG] Parametri controllati");
		System.out.println("Invio richieste a " + registryHost + " per il servizio di nome " + serviceName);

		// CONNESSIONE AL SERVIZIO RMI REMOTO
		try {
			String completeName = "//" + registryHost + ":" + registryPort + "/" + serviceName;
			RMI_InterfaceFile serverRMI = (RMI_InterfaceFile) Naming.lookup(completeName);
			System.out.println("Client RMI: Servizio \"" + serviceName + "\" connesso");

			/**
			 * Inizio corpo Client
			 */
			System.out.println("\nRichieste di servizio fino a fine file\n");

			String service;
			System.out.println("Servizio (1=elimina_prenotazione, 2=visualizza_prenotazioni), EOF per terminare: ");

			while ((service = stdIn.readLine()) != null) {
				// 1
				if (service.equals("1")) { // gestione servizio 1
					/**
					 * INIZIALIZZO I PARAMETRI DA PASSARE E RICEVERE DAL METODO REMOTO
					 */
					String targa;
					int risposta;

					try {
						System.out.println("targa? "); // richiesta parametro
						targa = stdIn.readLine(); // acquisizione parametro
						/*
						 * eventuali altre richieste di parametri da passare al metodo remoto
						 */
						try {
							risposta = serverRMI.elimina_prenotazione(targa);
							if (risposta == 1) {
								System.out.println("successo, prenotazione eliminata");
							} else {
								System.out.println("[ERRORE]: Server");
							}
						} catch (RemoteException re) {
							System.out.println("Errore remoto: " + re.toString());
						}
					} catch (IOException e) {
						System.out.println("errore di input");
					}
				} // fine servizio 1

				else if (service.equals("2")) { // gestione servizio 2
					/**
					 * INIZIALIZZO I PARAMETRI DA PASSARE E RICEVERE DAL METODO REMOTO
					 */
					String tipoVeicolo;
					Dato[] risposta;

					try {
						// richiesta parametro
						System.out.println("tipoVeicolo? "); // richiesta parametro
						tipoVeicolo = stdIn.readLine(); // acquisizione parametro
						if (!tipoVeicolo.equals("auto") && !tipoVeicolo.equals("camper")) {
							System.out.println("[ERRORE]: il tipo veicolo può essere \"auto\" o \"camper\" ");
							continue;
						}
						/*
						 * eventuali altre richieste di parametri da passare al metodo remoto
						 */
						try {
							// chiamata al metodo remoto
							risposta = serverRMI.visualizza_prenotazioni(tipoVeicolo);
							if (risposta != null && risposta.length > 0) {
								for (Dato d : risposta) {
									System.out.println(d.toString() + "\n");
								}
							} else {
								System.out.println("[ERRORE]: non ho trovato prenotazioni compliant");
							}
						} catch (RemoteException re) {
							System.out.println("Errore remoto: " + re.toString());
						}
					} catch (IOException e) { // per eventuali parsing dell'input
						System.out.println("errore di input");
					}
				} // fine servizio 2

				else { // servizio non disponibile
					System.out.println("Servizio non disponibile");
				}
				System.out.println("Servizio (1=elimina_prenotazione, 2=visualizza_prenotazioni), EOF per terminare: ");
			} // while !EOF

		} catch (Exception e) {
			System.err.println("ClientRMI: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
	}
}

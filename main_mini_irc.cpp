/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main_mini_irc.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjurdana <pjurdana@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/07 11:36:43 by pjurdana          #+#    #+#             */
/*   Updated: 2025/10/08 09:54:16 by pjurdana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <netdb.h>
#include <iostream>
#include <netinet/in.h>


#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>


bool	send_message(int fd, const std::string &str)
{

	const char *buffer = str.c_str();
	size_t to_send = str.size();
	while (to_send > 0)
	{
		ssize_t sent = send(fd, buffer + (str.size() - to_send), to_send, 0); // ssize_t type d'entier signe ca permet d'avoir la taille d'une variable utilise par send (read/write aussi), en gros le nombre d'octet qui seront bien envoye 
		if (sent < 0)
		{
			if (errno == EINTR) // permet de relancer lechange de donnee si il a ete interompue par un autre, EINTR = Interrupted System Call
				continue;
			return (false); // tout a ete transfere 
		}
		to_send -= static_cast<size_t>(sent); // on retire en octet ce qui a ete envoye
	}
	return (true);
}


ssize_t reception_buffer(int fd, std::string &buffer)
{

	char	tmp[1024]; // completement fixe(peut etre modifier) ca sera traiter en plusieurs iterations si plus gros ( dans l'idee hein)

	ssize_t	recept = recv(fd, tmp, sizeof(tmp), 0);
	if (recept > 0) // si y en a on append
		buffer.append(tmp, tmp + recept);// append methode de std::string ca rajoute a la fin, strcat en gros mais c++

	return (recept);
}


bool	complete_line(std::string &client_input, std::string &line) //
{

	size_t pos = client_input.find("\r\n"); // ("\r\n") norme IRC de chaque fin de message pour savoir si la ligne est fini

	if (pos == std::string::npos) // litteralement echec, c'est la constante d'erreur du size_t (-1 binaire)
		return (false);

	line = client_input.substr(0, pos); // ca devrait aller la
	client_input.erase(0, pos + 2); // on vire le \r\n
	return (true);
}






int	main(void)
{
	int sock;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		std::cerr << "SOCKET FAILED" << std::endl;
		return EXIT_FAILURE;
	}



	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET,SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		std::cerr << "SETSOCKOPT FAILED" << std::endl;
		close(sock);
		return EXIT_FAILURE;
	}




	struct sockaddr_in addr; // variable de type struct utilise pour les IPv4
	std::memset(&addr, 0, sizeof(addr)); // on met tout a zero
	addr.sin_family = AF_INET; //donne la "famille", la IPv4, utilise par bind/connect
	addr.sin_port = htons(6667); // on donne le port voulu, htons ??? -> convertit un entier 16-bits de l’ordre d’octets host vers network byte order (big-endian). Sans htons le port sera probablement erroné. ca va vous ?
	addr.sin_addr.s_addr = INADDR_ANY; // en gros on accepte toute les IPs

	if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) // bind va associer le socket a ladresse/port defini par addr. on &addr par ce que le langage est vieux, mais surtout par ce qu'il attend une struct, le size of est pour la taille de la struct
	{
		std::cerr << "BIND FAILED" << std::endl;
		close(sock);
		return EXIT_FAILURE;
	}

	if (listen(sock, SOMAXCONN) < 0) // place le socket en mode ecoute, le second parametre est la "backlog" nombre max de user en attente, c'est une valeur systeme ca peut etre modifie par un petit int
	{
		std::cerr << "LISTEN FAILED" << std::endl;
		close(sock);
		return EXIT_FAILURE;
	}

	std::cout << "Listening on port 6667...\n";

	for (;;) // decouverte de la boucle infini avec for
	{
		struct sockaddr_in client; // meme struct mais la pour le client on va bourrer ses infos IP/port dedans a l'identique qui nous sont transmis par accept
		socklen_t client_len = sizeof(client); // ca c'est pour recuperer la taille que la struct va devoir prendre
		int client_fd = accept(sock, (struct sockaddr*)&client, &client_len); // accept va du coup nous refiler un fd celui du client pour communiquer avec
		if (client_fd < 0)
		{
			std::cerr << "ACCEPT FAILED" << std::endl;
			/* si interruption par signal, on peut continuer */
			if (errno == EINTR) // permet de relancer lechange de donnee si il a ete interompue par un autre, EINTR = Interrupted System Call
				continue;
			break;
		}

		char *client_ip = inet_ntoa(client.sin_addr); // traduis l'IP binaire en X.X.X.X
		unsigned short client_port = ntohs(client.sin_port); // recupere le port du client, MAIS sin_port est en network byte order, ntohs() sert a convertir en ordre d’octets host avant affichage ou comparaison. moi ca va pas.
		std::cout << "Connection from " << (client_ip ? client_ip : "??") // Truc chelou pour eviter un plantage ca n'a pas l'air tres utile.
					<< ":" << client_port << "\n";



		 // plein de variable oui ALED
		std::string client_input;
		std::string line;
		std::string nickname;
		std::string user;

		bool registered = false;


		for (;;)
		{
			ssize_t recept = reception_buffer(client_fd, client_input);
			if (recept == 0)
			{
				std::cout << "Connection closed by the client" << std::endl;
				break;
			}
			else if (recept < 0)
			{
				if (errno == EINTR) // permet de relancer lechange de donnee si il a ete interompue par un autre, EINTR = Interrupted System Call
					continue ;
				std::cerr << "RECEPTION_BUFFER FAILED" << std::endl;
				break;
			}
			
			while (complete_line(client_input, line))
			{
				std::cout << "DEBUG : {" << line << "}" << std::endl;


				std::string cmd;
				std::string params;
				size_t sp = line.find(' '); // on cherche l'espace pour separer la cmd du params
				if (sp == std::string::npos) // litteralement echec, c'est la constante d'erreur du size_t (-1 binaire)
				{
					cmd = line;
					params = "";
				} else
				{
					cmd = line.substr(0, sp);
					params = line.substr(sp + 1);
				}

				// normaliser cmd en majuscules simples (IRC est case-insensitive pour commandes)
				for (size_t i = 0; i < cmd.size(); ++i)
				{
					if (cmd[i] >= 'a' && cmd[i] <= 'z')
						cmd[i] = char(cmd[i] - 'a' + 'A'); // to_upper
				}

				if (cmd == "NICK") // pas du tout fonctionnel c'est normal
				{
					// param est le nick (peut contenir pas d'espaces)
					nickname = params;
					// enlever eventuels espaces
					if (!nickname.empty() && nickname[0] == ' ')
						nickname.erase(0, nickname.find_first_not_of(' '));
					// si déjà user renseigné, on peut s'enregistrer
				}
				else if (cmd == "USER")
				{
					// USER <username> <mode> <unused> :<realname>
					// nous garderons juste le premier token comme user
					size_t psp = params.find(' ');
					if (psp != std::string::npos) // litteralement echec, c'est la constante d'erreur du size_t (-1 binaire)
					{
						user = params.substr(0, psp);
					}
					else
					{
						user = params;
					}
				}
				else if (cmd == "PING")
				{
					// ping :token  ou PING token
					std::string token = params;
					if (token.empty())
						token = ":";
					// renvoyer PONG
					std::string pong = "PONG " + token + "\r\n";
					send_message(client_fd, pong);
					continue;
				}
				else if (cmd == "QUIT")
				{
					// client veut partir
					std::string bye = "ERROR :Closing Link\r\n";
					send_message(client_fd, bye);
					std::cout << "Client asked to QUIT\n";
					goto close_client;
				}

				// si on a nick et user et pas encore enregistré : envoyer 001..004
				if (!registered && !nickname.empty() && !user.empty())
				{
					std::string srv = "Le.FISC"; // nom du serveur affiché
					// welcome numeric 001
					std::string r001 = ":" + srv + " 001 " + nickname + " :Welcome to the minimal IRC server, " + nickname + "\r\n";
					std::string r002 = ":" + srv + " 002 " + nickname + " :Your host is " + srv + "\r\n";
					std::string r003 = ":" + srv + " 003 " + nickname + " :This is a test server\r\n";
					std::string r004 = ":" + srv + " 004 " + nickname + " Le.FISC 1.0 aiow\r\n";
					send_message(client_fd, r001);
					send_message(client_fd, r002);
					send_message(client_fd, r003);
					send_message(client_fd, r004);
					registered = true;
					std::cout << "Registered user: " << nickname << " / " << user << "\n";
				}


			}



			
		}
		close_client:
			close(client_fd);
			std::cout << "Closed client fd" << std::endl;
		// close(client_fd); // pas besoin d'un notion si ? si par ce que faut pas close la mdr; pas comme ca en tout cas
	}

	close(sock); // Notions.closes_ses_fd.42.fr
	return EXIT_SUCCESS;
	// je sais pas si ca marche lol.
	return (0);
}
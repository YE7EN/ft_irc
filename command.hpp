/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: pjurdana <pjurdana@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/10/08 10:19:49 by pjurdana          #+#    #+#             */
/*   Updated: 2025/10/08 10:31:26 by pjurdana         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//  all the commands

// une fonctions handle command pour generaliser je pense,
	// pointeur sur fonction + switch case ? (HARL)


// NICK : vérifier disponibilité, uniqueness, si déjà registered renvoyer change
			// message broadcast to channels; set nick; if both nick+user set and not
				// registered -> send 001..004.

// USER : stocker user/realname; same registration logic.

// PING : PONG :token

// QUIT : broadcast QUIT to channels, cleanup client

// JOIN <#chan> [key] : create channel if not exists, check modes (invite-only,
							// key, limit), add member, broadcast JOIN, send topic +
							// 	NAMES replies.

// PART <#chan> [msg] : remove member, broadcast PART

// PRIVMSG <target> :message : if #channel -> broadcast to channel except sender;
// 					if nick -> direct send to that client (ERR_NOSUCHNICK if absent)

// KICK <#chan> <user> [reason] : check caller is op, remove target, broadcast KICK

// INVITE <nick> <#chan> : check caller rights if needed, add to channel.invited,
// 						notify invitee with :inviter INVITE nick :#chan

// TOPIC <#chan> [ :<topic> ] : if no param -> reply current topic or RPL_NOTOPIC;
// 								if set -> check mode_t and op rights, set topic,
// 									broadcast TOPIC

// MODE <#chan> <modes> [params...] : parse modes string (+/-), apply changes with
// 									param consumption and rights checks (op-only),
// 										broadcast resulting MODE line.

























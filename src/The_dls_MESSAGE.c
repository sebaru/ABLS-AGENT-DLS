/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/src/Ths_dls_MESSAGE.c        Déclaration des fonctions pour la gestion des message                               */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                     jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_MESSAGE.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * ABLS-AGENT-DLS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ABLS-AGENT-DLS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ABLS-AGENT-DLS; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "dls.h"

/******************************************************************************************************************************/
/* Dls_data_MESSAGE_free_one: Libère la mémoire associée à un MESSAGE un plugin                                               */
/* Entrée : le pointeur vers la structure du message                                                                          */
/******************************************************************************************************************************/
 static void Dls_data_MESSAGE_free_one ( struct DLS_MESSAGE *bit )
  { Json_unref ( bit->source_node );
    g_free(bit);
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_free_all: Libère la mémoire associée aux messages d'un plugin                                             */
/* Entrée : le pointeur vers le plugin                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_free_all ( struct DLS_PLUGIN *plugin )
  { if (plugin->Dls_data_MESSAGE) g_slist_free_full ( plugin->Dls_data_MESSAGE, (GDestroyNotify) Dls_data_MESSAGE_free_one );
    plugin->Dls_data_MESSAGE = NULL;
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_create_by_array : Création d'un MESSAGE pour le plugin                                                    */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_MESSAGE *bit = g_try_malloc0 ( sizeof(struct DLS_MESSAGE) );
    if (!bit)
    { Info( __func__, "dls", tech_id, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    bit->etat = FALSE;                                    /* A l'init, le message est OFF. Json_get_bool ( element, "etat" ); */
    bit->source_node = json_node_ref ( element );
    bit->last_on = 0;                                            /* A l'init, il n'y a pas de last on (en dixieme de seconde) */
    gchar *libelle = Json_get_string ( element, "libelle" );
    bit->libelle_is_dynamic = ( libelle && g_utf8_strchr( libelle, -1, '$') ? TRUE : FALSE );

    plugin->Dls_data_MESSAGE = g_slist_prepend ( plugin->Dls_data_MESSAGE, bit );
    Info( __func__, "dls", tech_id, LOG_INFO,
              "Create bit DLS_MESSAGE '%s:%s'=%d", bit->tech_id, bit->acronyme, bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_lookup: Recherche un MESSAGE dans les plugins DLS                                                         */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MESSAGE *Dls_data_MESSAGE_lookup ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Agent_vars->Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_MESSAGE;
          while (liste)
           { struct DLS_MESSAGE *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_set: Emet le message en parametre                                                                         */
/* Entrée : les plugin vars, le message                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_set ( struct DLS_PLUGIN *plugin, struct DLS_MESSAGE *msg )
  { if (!msg) return;
    msg->new_etat = TRUE;                                                         /* Sauvegarde de l'état souhaité du message */
    msg->new_etat_by_line = (plugin ? plugin->num_ligne : -1);                                 /* Sauvegarde du numéro de ligne */
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_send_to_api: Envoie l'historique et les notifications associees au message                                */
/* Entrée: plugin DLS, message et etat a publier                                                                              */
/* Sortie: TRUE si le traitement est accepte                                                                                */
/******************************************************************************************************************************/
 static gboolean Dls_data_MESSAGE_send_to_api ( struct DLS_PLUGIN *plugin, struct DLS_MESSAGE *msg, gboolean etat )
  { if (!(plugin && msg)) return(FALSE);

    guint top = Agent_get_top ( Agent );
    if (msg->etat)
     { gint rate_limit = Json_get_int ( msg->source_node, "rate_limit" );
       if (msg->last_on && top < msg->last_on + rate_limit * 10)
        { Info( __func__, "dls", msg->tech_id, LOG_WARNING,
                "Rate limit (=%d) for '%s:%s' reached: not sending", rate_limit, msg->tech_id, msg->acronyme );
          return(FALSE);
        }
       msg->last_on = top;
     }

    if (msg->etat == TRUE)                                                                                 /* Passage a  un ? */
     { gchar date_create[128];
       Get_datetime_usec ( date_create, sizeof(date_create) );           /* Mise à jour de de la date de création */
       gchar *dls_shortname = Json_get_string ( msg->source_node, "dls_shortname" );
/*------------------------------------------------ Envoi vers API ------------------------------------------------------------*/
       JsonNode *MSGNode = Json_create();
       if (MSGNode)
        { Json_add_string( MSGNode, "tech_id",  msg->tech_id );
          Json_add_string( MSGNode, "acronyme", msg->acronyme );
          Json_add_string( MSGNode, "libelle",  msg->libelle_converted );
          Json_add_string( MSGNode, "date_create", date_create );
          Json_add_bool( MSGNode, "alive", TRUE );
          Agent_send_mqtt_api_message ( Agent, MSGNode, FALSE, "DLS_HISTO" );
          Json_unref( MSGNode );
        }
       else Info( __func__, "mqtt", msg->tech_id, LOG_ALERT, "Cannot send DLS_HISTO to API: memory error" );
/*---------------------------------------------------- Envoi IMSG ------------------------------------------------------------*/
       gint notif_chat = Json_get_int ( msg->source_node, "notif_chat" );
       if (notif_chat == DLS_NOTIF_BY_DLS) { notif_chat = Json_get_int ( msg->source_node, "notif_chat_by_dls" ); }
       if (notif_chat == DLS_NOTIF_YES)
        { JsonNode *IMSGNode = Json_create();
          if (IMSGNode)
           { Json_add_string( IMSGNode, "tech_id", msg->tech_id );
             Json_add_string( IMSGNode, "acronyme", msg->acronyme );
             Json_add_string( IMSGNode, "dls_shortname", dls_shortname );
             Json_add_string( IMSGNode, "libelle", msg->libelle_converted );
             Agent_send_mqtt_local_message ( Agent, IMSGNode, FALSE, "SEND_IMSG" );
             Json_unref( IMSGNode );
           }
          else Info( __func__, "mqtt", msg->tech_id, LOG_ALERT, "Cannot send IMSG: memory error" );
        }
/*---------------------------------------------------- Envoi SMS -------------------------------------------------------------*/
       gint notif_sms = Json_get_int ( msg->source_node, "notif_sms" );
       if (notif_sms == DLS_NOTIF_BY_DLS) { notif_sms = Json_get_int ( msg->source_node, "notif_sms_by_dls" ); }
       if (notif_sms == DLS_NOTIF_YES || notif_sms == DLS_NOTIF_OVH_ONLY)
        { JsonNode *SMSNode = Json_create();
          if (SMSNode)
           { Json_add_string( SMSNode, "tech_id", msg->tech_id );
             Json_add_string( SMSNode, "acronyme", msg->acronyme );
             Json_add_string( SMSNode, "dls_shortname", dls_shortname );
             Json_add_string( SMSNode, "libelle", msg->libelle_converted );
             Json_add_int( SMSNode, "notif_sms", notif_sms );
             Agent_send_mqtt_local_message ( Agent, SMSNode, FALSE, "SEND_SMS" );
             Json_unref( SMSNode );
           }
          else Info( __func__, "mqtt", msg->tech_id, LOG_ALERT, "Cannot send SMS: memory error" );
        }
/*---------------------------------------------------- Envoi AUDIO -----------------------------------------------------------*/
       gchar *audio_zone_by_dls = Json_get_string ( msg->source_node, "audio_zone_by_dls" );
       gchar *audio_zone_name = (strlen(audio_zone_by_dls) ? audio_zone_by_dls : Json_get_string ( msg->source_node, "audio_zone_name" ));
       if (strcasecmp ( audio_zone_name, "ZD_NONE"))
        { gchar *audio_libelle = Json_get_string ( msg->source_node, "audio_libelle" );
          if (strlen(audio_libelle)) AUDIO_Send_to_zone ( audio_zone_name, audio_libelle );
        }
      }
     else if (msg->etat == 0)
      { JsonNode *histo = Convert_msg_off_to_histo ( msg );
        if(histo)
         { Agent_send_mqtt_api_message ( Agent, histo, FALSE, "DLS_HISTO" );
           Json_unref( histo );
         } else Info( __func__, "mqtt", msg->tech_id, LOG_ERR, "Error when convert '%s:%s' from msg off to histo",
                      msg->tech_id, msg->acronyme );
      }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_apply ( struct DLS_PLUGIN *plugin )
  { if (!plugin) return;

    GSList *liste = plugin->Dls_data_MESSAGE;
    while ( liste )
     { struct DLS_MESSAGE *msg = liste->data;
       gint freeze = Json_get_int ( msg->source_node, "freeze" );
       if ( msg->etat == TRUE && msg->new_etat == FALSE && Json_get_int ( msg->source_node, "typologie" ) == MSG_NOTIF )
        { /* pas de desactivation msg quand typologie = 0, donc no action */ }
       else if ( msg->etat == TRUE && msg->new_etat == FALSE && Json_get_int ( msg->source_node, "groupe" ) )
        { /* pas de desactivation msg quand dans un groupe, donc no action */ }
       else if ( msg->etat == TRUE && msg->new_etat == FALSE )             /* si le message est désactivé après run du plugin */
        { Dls_data_MESSAGE_send_to_api ( plugin, msg, FALSE );
          Info( __func__, "dls", plugin->tech_id, LOG_DEBUG,
                    "ligne %04d: Changing DLS_MSG '%s:%s'=FALSE", msg->new_etat_by_line, msg->tech_id, msg->acronyme );
          Agent_vars->audit_bit_interne_par_min++;
        }
       else if ( msg->etat == FALSE && msg->new_etat == TRUE )                       /* si message activé après run du plugin */
        { /* On commence par mettre a 0 les messages du meme groupe, s'il y en a /*/
          gint groupe = Json_get_int ( msg->source_node, "groupe" );
          if (groupe)
           { GSList *search = plugin->Dls_data_MESSAGE;
             while ( search )
              { struct DLS_MESSAGE *search_msg = search->data;
                if (search_msg != msg && Json_get_int ( search_msg->source_node, "groupe" ) == groupe )
                 { search_msg->new_etat = search_msg->etat = FALSE;
                   Dls_data_MESSAGE_send_to_api ( plugin, search_msg, FALSE );
                   Info( __func__, "dls", plugin->tech_id, LOG_DEBUG,
                    "ligne %04d: Changing DLS_MSG '%s:%s'=FALSE (via groupe %d)", msg->new_etat_by_line, msg->tech_id, msg->acronyme, groupe );
                   Agent_vars->audit_bit_interne_par_min++;
                 }
                search = g_slist_next ( search );
              }
           }
                                                                                      /* Calcul du libelle du nouveau message */
          gchar *libelle_source = Json_get_string(msg->source_node, "libelle");
          if (msg->libelle_is_dynamic)                                                     /* Conversion du libelle dynamique */
           { gchar *libelle_converted = Convert_libelle_dynamique ( libelle_source );
             if (libelle_converted)
              { g_snprintf ( msg->libelle_converted, sizeof(msg->libelle_converted), "%s", libelle_converted );
                g_free(libelle_converted);
              }
             else g_snprintf ( msg->libelle_converted, sizeof(msg->libelle_converted), "Conversion Error" );
             msg->next_top_check_libelle = Agent_get_top ( Agent ) + freeze;                                              /* Freeze time */
           }
          else g_snprintf ( msg->libelle_converted, sizeof(msg->libelle_converted), "%s", libelle_source ? libelle_source : "" ); /* Pas de conversion */
          Dls_data_MESSAGE_send_to_api ( plugin, msg, TRUE );
          Info( __func__, "dls", plugin->tech_id, LOG_DEBUG,
                    "ligne %04d: Changing DLS_MSG '%s:%s'=TRUE", msg->new_etat_by_line, plugin->tech_id, msg->acronyme );
          Agent_vars->audit_bit_interne_par_min++;
        }
       else if ( msg->etat && msg->libelle_is_dynamic && freeze >=0 &&              /* Update periodique du libelle dynamique */
                 msg->next_top_check_libelle <= Agent_get_top ( Agent ))
        { gchar *libelle_converted = Convert_libelle_dynamique ( Json_get_string(msg->source_node, "libelle") );
          gboolean libelle_changed = (libelle_converted && strcmp ( libelle_converted, msg->libelle_converted ));
          if (libelle_changed)
           { g_snprintf ( msg->libelle_converted, sizeof(msg->libelle_converted), "%s", libelle_converted );
             Dls_data_MESSAGE_send_to_api ( plugin, msg, TRUE );
           }
          g_free(libelle_converted);
          msg->next_top_check_libelle = Agent_get_top ( Agent ) + freeze;                                                 /* freeze time */
        }
       if (msg->etat != msg->new_etat) Dls_Monitor_mark ( plugin, DLS_MONITOR_MSG, msg );
       msg->etat = msg->new_etat;                                                                /* Sauvegarde du nouvel état */
       msg->new_etat = FALSE;                                             /* Préparation du futur calcul de l'état du message */
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

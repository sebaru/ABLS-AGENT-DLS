/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/src/audio.c  Distribution des messages audio                                                                 */
/* Projet Abls-Habitat                   Gestion d'habitat                                                05.10.2026  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * audio.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sebastien LEFEVRE
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

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "dls.h"

/******************************************************************************************************************************/
/* AUDIO_Send_to_zone: Envoi un message vocal à une zone de diffusion                                                         */
/* Entrée: La zone et l'audio a diffuser                                                                                      */
/******************************************************************************************************************************/
 void AUDIO_Send_to_zone ( gchar *audio_zone_name, gchar *audio_libelle )
  { if (!audio_zone_name || !audio_libelle) return;

    gchar *audio_tech_id = Agent_config_get_string ( Agent, "audio_tech_id" );
    if (!audio_tech_id || !*audio_tech_id) audio_tech_id = "AUDIO";

    Info( __func__, "audio", audio_tech_id, LOG_NOTICE,
          "Saying '%s' on audio_zone '%s'", audio_libelle, audio_zone_name );
    JsonNode *AudioNode = Json_create();
    if (!AudioNode)
     { Info( __func__, "audio", audio_tech_id, LOG_ALERT,
            "Cannot send '%s' to audio_zone '%s': memory error", audio_libelle, audio_zone_name );
       return;
     }

    Json_add_string ( AudioNode, "audio_libelle", audio_libelle );
    struct DLS_DI *bit = Dls_data_DI_lookup ( audio_tech_id, audio_zone_name );
    if (bit) Dls_data_DI_set_pulse ( NULL, bit );
    else Info( __func__, "audio", audio_tech_id, LOG_ERR,
               "DI '%s:%s' not found", audio_tech_id, audio_zone_name );

    Agent_send_mqtt_local_message ( Agent, AudioNode, FALSE, "AUDIO_ZONE/%s", audio_zone_name );
    Json_unref ( AudioNode );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
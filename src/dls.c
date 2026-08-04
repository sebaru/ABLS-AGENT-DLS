/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/dls.c  Gestion autonome du métier DLS                                                                       */
/* Projet Abls-Habitat                   Gestion d'habitat                                                01.08.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.c
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

 #include <string.h>

 #include "dls.h"
 struct ABLS_AGENT *Agent = NULL;                                                                 /* Structure de l'agent DLS */

/******************************************************************************************************************************/
/* main: Point d'entree de l'agent DLS                                                                                        */
/* Entree: argc - nombre d'arguments                                                                                          */
/*         argv - tableau des arguments                                                                                       */
/* Sortie: 0 en cas de succes, 1 en cas d'erreur                                                                              */
/******************************************************************************************************************************/
 gint main ( gint argc, gchar *argv[] )
  { setenv ( "ABLS_AGENT_TECH_ID", "DLS", 1 );
    Agent = Agent_init ( argv[0], "dls", ABLS_AGENT_DLS_VERSION, sizeof(struct ABLS_DLS_VARS), argc, argv );
    struct ABLS_DLS_VARS *vars = (struct ABLS_DLS_VARS *)Agent->vars;

    g_rw_lock_init ( &vars->Dls_plugins_lock );
    g_rw_lock_init ( &vars->Liste_DO_synchro );
    g_rw_lock_init ( &vars->Liste_AO_synchro );
    g_rw_lock_init ( &vars->Liste_visuel_synchro );
    g_rw_lock_init ( &vars->Liste_msg_synchro );

    vars->Top_check_horaire = TRUE;

    Agent_is_ready ( agent );                                                                             /* L'agent est pret */

    Agent_set_status ( agent, "Loading mappings..." );
    MAP_Init();
    MAP_Remap();
    Agent_set_status ( agent, "Loading plugins..." );
    Dls_Importer_plugins( agent );
    Dls_Load_horloge_ticks();

    Agent_set_status ( agent, "Agent is running." );
    while(agent->Agent_run == AGENT_IS_RUNNING)
     { Agent_loop ( agent );
/*----------------------------------------------------------- Loop D.L.S -----------------------------------------------------*/
       Prendre_heure();

       Dls_set_edge();
       Dls_set_cde_exterieure();
       Dls_foreach_plugins ( agent, Dls_run_plugin );
       Dls_reset_edge();
       Dls_reset_cde_exterieure();

/*----------------------------------------------------------- Ecoute du Master -----------------------------------------------*/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( agent ) ) != NULL )
        { if (Mqtt_topic_is ( mqtt_local_message, 2, "SET_AI", "+" ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl1" ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl2" ) );
             Dls_data_AI_set_from_thread_ai ( mqtt_local_message );
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 2, "SET_DI", "+" ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl1" ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl2" ) );
             Dls_data_DI_set_from_thread_di ( mqtt_local_message );
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 2, "SET_WATCHDOG", agent->agent_tech_id ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl1" ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl2" ) );
             Dls_data_WATCHDOG_set_from_thread_watchdog ( mqtt_local_message );
           }
          Json_unref ( mqtt_local_message );
        }

/*----------------------------------------------------------- Ecoute de l'API ------------------------------------------------*/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "DLS", "+", "RELOAD" ) )
           { gchar *target = Json_get_string ( mqtt_api_message, "mqtt_topic_lvl2" );
             Dls_Importer_un_plugin ( target );
           }
          else if ( Mqtt_topic_is ( mqtt_api_message, 2, "+", "DLS", "REMAP" ) )
           { MSRV_Remap(); }
          else if ( Mqtt_topic_is ( mqtt_api_message, 2, "+", "DLS", "RELOAD_HORLOGE_TICK" ) )
           { Dls_Load_horloge_ticks(); }
          Json_unref ( mqtt_api_message );
        }
     }

    Dls_Decharger_plugins();

    g_rw_lock_clear ( &vars->Dls_plugins_lock );
    g_rw_lock_clear ( &vars->Liste_DO_synchro );
    g_rw_lock_clear ( &vars->Liste_AO_synchro );
    g_rw_lock_clear ( &vars->Liste_visuel_synchro );
    g_rw_lock_clear ( &vars->Liste_msg_synchro );
    pthread_mutex_destroy  ( &vars->synchro );

    MAP_End();

    Agent_end ( agent );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

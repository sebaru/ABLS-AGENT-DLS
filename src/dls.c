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

/******************************************************************************************************************************/
/* main: Point d'entree de l'agent DLS                                                                                               */
/* Entree: argc - nombre d'arguments                                                                                                  */
/*         argv - tableau des arguments                                                                                                */
/* Sortie: 0 en cas de succes, 1 en cas d'erreur                                                                                       */
/******************************************************************************************************************************/
 gint main ( gint argc, gchar *argv[] )
  { struct ABLS_AGENT *agent = Agent_init ( argv[0], "dls", ABLS_AGENT_DLS_VERSION, sizeof(struct ABLS_DLS_VARS), argc, argv );
    struct ABLS_DLS_VARS *vars = (struct ABLS_DLS_VARS *)agent->vars;

    pthread_mutexattr_t m_attr;
    pthread_mutexattr_init ( &m_attr );
    pthread_mutex_init ( &vars->synchro, &m_attr );

    pthread_rwlockattr_t rw_attr;
    pthread_rwlockattr_init ( &rw_attr );
    pthread_rwlock_init ( &vars->Liste_DO_synchro, &rw_attr );
    pthread_rwlock_init ( &vars->Liste_AO_synchro, &rw_attr );
    pthread_rwlock_init ( &vars->Liste_visuel_synchro, &rw_attr );
    pthread_rwlock_init ( &vars->Liste_msg_synchro, &rw_attr );
    pthread_rwlock_init ( &vars->Maps_synchro, &rw_attr );


    vars->Top_check_horaire = TRUE;
    vars->temps_sched = 10000;

    Agent_is_ready ( agent );                                                                             /* L'agent est pret */

    MAP_init();
    MSRV_Remap();
    Agent_set_status ( agent, "Loading plugins..." );
    Dls_Importer_plugins();
    Dls_Load_horloge_ticks();

    Agent_set_status ( agent, "Agent is running." );
    while(agent->Agent_run == AGENT_IS_RUNNING)
     { Agent_loop ( agent );
/*----------------------------------------------------------- Loop D.L.S -----------------------------------------------------*/
       vars->top++;
       Dls_update_runtime_signals();
       Prendre_heure();

       pthread_mutex_lock ( &vars->synchro );
       Dls_set_edge();
       Dls_set_cde_exterieure();
       Dls_foreach_plugins ( NULL, Dls_run_plugin );
       Dls_foreach_plugins ( NULL, Dls_apply_message_cb );
       Dls_foreach_plugins ( NULL, Dls_apply_visuel_cb );
       Dls_reset_edge();
       Dls_reset_cde_exterieure();
       pthread_mutex_unlock ( &vars->synchro );

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

    pthread_rwlock_destroy ( &vars->Liste_DO_synchro );
    pthread_rwlock_destroy ( &vars->Liste_AO_synchro );
    pthread_rwlock_destroy ( &vars->Liste_visuel_synchro );
    pthread_rwlock_destroy ( &vars->Liste_msg_synchro );
    pthread_rwlock_destroy ( &vars->Maps_synchro );
    pthread_mutex_destroy  ( &vars->synchro );

    MAP_end();

    Agent_end ( agent );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

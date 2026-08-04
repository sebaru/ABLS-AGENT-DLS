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
    setenv ( "ABLS_TPS", "100", 1 );                                                                 /* 100 tour par secondes */
    Agent = Agent_init ( argv[0], "dls", ABLS_AGENT_DLS_VERSION, sizeof(struct ABLS_DLS_VARS), argc, argv );
    struct ABLS_DLS_VARS *vars = (struct ABLS_DLS_VARS *)Agent->vars;

    g_rw_lock_init ( &vars->Dls_plugins_lock );
    g_rw_lock_init ( &vars->Liste_DO_synchro );
    g_rw_lock_init ( &vars->Liste_AO_synchro );
    g_rw_lock_init ( &vars->Liste_visuel_synchro );
    g_rw_lock_init ( &vars->Liste_msg_synchro );

    Agent_is_ready ( Agent );                                                                             /* L'agent est pret */

    Agent_set_status ( Agent, "Loading mappings..." );
    MAP_Init();
    MAP_Remap();
    Agent_set_status ( Agent, "Loading plugins..." );
    Dls_Importer_plugins();
    Dls_Load_horloge_ticks();

    guint next_top_5hz   = next_top_2hz  = Agent->Top;                                                   /* Init des next top */
    guint next_top_1sec  = next_top_2sec = next_top_5sec = Agent->Top + 10;
    guint next_top_1min  = Agent->Top + 600;
    guint next_top_10min = Agent->Top + 6000;

    Agent_set_status ( Agent, "Agent is running." );
    while(Agent->Agent_run == AGENT_IS_RUNNING)
     { Agent_loop ( Agent );
/*----------------------------------------------------------- Loop D.L.S -----------------------------------------------------*/
       Dls_Start_top_horaire();
       g_rw_lock_reader_lock ( &vars->Dls_plugins_lock );
/******************************************************************************************************************************/
       if (Partage->top>=next_top_5hz)                                                             /* Toutes les 1/5 secondes */
        { next_top_5hz = Agent->Top + 2;
          Dls_data_MONO_set ( NULL, vars->sys_top_5hz, TRUE );
          Dls_data_BI_set   ( NULL, vars->sys_flipflop_5hz,
                             !Dls_data_BI_get ( vars->sys_flipflop_5hz) );
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_2hz)                                                             /* Toutes les 1/2 secondes */
         {next_top_2hz = Agent->Top + 5;
          Dls_data_MONO_set ( NULL, vars->sys_top_2hz, TRUE );
          Dls_data_BI_set   ( NULL, vars->sys_flipflop_2hz,
                             !Dls_data_BI_get ( vars->sys_flipflop_2hz) );
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_1sec)                                                                /* Toutes les secondes */
        { next_top_1sec = Agent->Top + 10;
          Dls_data_MONO_set ( NULL, vars->sys_top_1sec, TRUE );
          Dls_data_BI_set   ( NULL, vars->sys_flipflop_1sec,
                             !Dls_data_BI_get ( vars->sys_flipflop_1sec) );

          vars->audit_bit_interne_per_sec_hold += vars->audit_bit_interne_per_sec;
          vars->audit_bit_interne_per_sec_hold = vars->audit_bit_interne_per_sec_hold >> 1;
          vars->audit_bit_interne_per_sec = 0;                                                                  /* historique */
          Dls_data_AI_set ( vars->sys_bit_per_sec, (gdouble)vars->audit_bit_interne_per_sec_hold, TRUE );
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_2sec)                                                              /* Toutes les 2 secondes */
        { next_top_2sec = Agent->Top+20;
          Dls_data_BI_set ( NULL, vars->sys_flipflop_2sec,
                           !Dls_data_BI_get ( vars->sys_flipflop_2sec) );
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_5sec)                                                                /* Toutes les 5 secondes */
        { next_top_5sec = Agent->Top + 50;
          Dls_data_MONO_set ( NULL, vars->sys_top_5sec, TRUE );
          Dls_foreach_plugins ( NULL, Dls_run_archivage );                        /* Archivage au mieux toutes les 5 secondes */
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_10sec)                                                              /* Toutes les 10 secondes */
        { next_top_10sec = Agent->Top + 100;
          Dls_data_MONO_set ( NULL, vars->sys_top_10sec, TRUE );
          Dls_data_BI_set ( NULL, vars->sys_mqtt_connected, vars->MQTT_connected );
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_1min)                                                                   /* Toutes les minutes */
        { next_top_1min = Agent->Top + 600;
          Dls_data_MONO_set ( NULL, vars->sys_top_1min, TRUE );
          Dls_Start_top_horaire ();                                        /* Mise à jour des variables de gestion de l'heure */
          Dls_data_activer_horloge();
        }
/******************************************************************************************************************************/
       if (Agent->Top>=next_top_10min)                                                             /* Toutes les 10 minutes */
        { next_top_10min = Agent->Top + 6000;
        }

       Dls_set_edge();                                                                    /* Mise à zero des bits de egde up/down */
       Dls_set_cde_exterieure();                                           /* Mise à un des bits de commande exterieure (furtifs) */

       Partage->top_cdg_plugin_dls = 0;                                                         /* On reset le cdg plugin DLS */

       Dls_foreach_plugins ( NULL, Dls_run_plugin );                                                  /* Run all plugin D.L.S */

       Partage->Top_check_horaire = FALSE;                        /* Controle horaire effectué un fois par minute max */
       Dls_reset_edge();                                                                   /* Mise à zero des bit de egde up/down */
       Dls_reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */

       Dls_data_HORLOGE_clear();
       Dls_data_MONO_set ( NULL, Partage->sys_top_5hz,   FALSE );                     /* RaZ des Mono du plugin 'SYS' */
       Dls_data_MONO_set ( NULL, Partage->sys_top_2hz,   FALSE );
       Dls_data_MONO_set ( NULL, Partage->sys_top_1sec,  FALSE );
       Dls_data_MONO_set ( NULL, Partage->sys_top_5sec,  FALSE );
       Dls_data_MONO_set ( NULL, Partage->sys_top_10sec, FALSE );
       Dls_data_MONO_set ( NULL, Partage->sys_top_1min,  FALSE );


       Dls_set_edge();
       Dls_set_cde_exterieure();
       Dls_foreach_plugins ( Agent, Dls_run_plugin );
       Dls_reset_edge();
       Dls_reset_cde_exterieure();

       Dls_Stop_top_horaire();
/*----------------------------------------------------------- Ecoute du Master -----------------------------------------------*/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( Agent ) ) != NULL )
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
          else if (Mqtt_topic_is ( mqtt_local_message, 2, "SET_WATCHDOG", Agent->agent_tech_id ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl1" ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Json_get_string ( mqtt_local_message, "mqtt_topic_lvl2" ) );
             Dls_data_WATCHDOG_set_from_thread_watchdog ( mqtt_local_message );
           }
          Json_unref ( mqtt_local_message );
        }

/*----------------------------------------------------------- Ecoute de l'API ------------------------------------------------*/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( Agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "DLS", "+", "RELOAD" ) )
           { gchar *target = Json_get_string ( mqtt_api_message, "mqtt_topic_lvl2" );
             Dls_Importer_un_plugin ( target );
           }
          else if ( Mqtt_topic_is ( mqtt_api_message, 2, "+", "DLS", "REMAP" ) )
           { MAP_Remap(); }
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

    MAP_End();

    Agent_end ( Agent );
    return(0);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

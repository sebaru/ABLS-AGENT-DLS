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
 struct ABLS_AGENT *Agent = NULL;
 struct DLS_VARS *Agent_vars = NULL;

/******************************************************************************************************************************/
/* Dls_init: Initialisation du module DLS                                                                                     */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_init ( )
  { g_rw_lock_init ( &Agent_vars->Dls_plugins_lock );
    g_rw_lock_init ( &Agent_vars->Liste_DO_synchro );
    g_rw_lock_init ( &Agent_vars->Liste_AO_synchro );

    GError *error = NULL;
    Agent_vars->Thread_import_plugin_pool = g_thread_pool_new( Dls_Importer_un_plugin, NULL, g_get_num_processors(), TRUE, &error);
    if (error != NULL)
     { Info( __func__, "dls", NULL, LOG_ERR, "Thread_import_plugin_pool failed: %s", error->message );
       g_error_free(error);
     }
    g_thread_pool_set_max_unused_threads ( 1 );

    gpointer status = Agent_status_push ( Agent, "Loading mappings..." );
    MAP_Init();
    MAP_Remap();
    Agent_status_pop ( Agent, status );

    status = Agent_status_push ( Agent, "Loading plugins..." );
    Dls_Importer_plugins();
    Dls_Load_horloge_ticks();
    Agent_status_pop ( Agent, status );

    Agent_vars->next_top_2hz   = Agent_get_top ( Agent );
    Agent_vars->next_top_5hz   = Agent_get_top ( Agent );
    Agent_vars->next_top_1sec  = Agent_get_top ( Agent ) + 10;
    Agent_vars->next_top_2sec  = Agent_get_top ( Agent ) + 20;
    Agent_vars->next_top_5sec  = Agent_get_top ( Agent ) + 50;
    Agent_vars->next_top_10sec = Agent_get_top ( Agent ) + 100;
    Agent_vars->next_top_1min  = Agent_get_top ( Agent ) + 600;
    Agent_vars->next_top_10min = Agent_get_top ( Agent ) + 6000;
    Agent_vars->last_top       = Agent_get_top ( Agent );
  }
/******************************************************************************************************************************/
/* Dls_end: Liberation des ressources du module DLS                                                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_end ( void )
  { g_thread_pool_free (Agent_vars->Thread_import_plugin_pool, FALSE, TRUE);
    Dls_Decharger_plugins();

    g_rw_lock_clear ( &Agent_vars->Dls_plugins_lock );
    g_rw_lock_clear ( &Agent_vars->Liste_DO_synchro );
    g_rw_lock_clear ( &Agent_vars->Liste_AO_synchro );

    MAP_End();
  }
/******************************************************************************************************************************/
/* Dls_loop: Boucle principale de traitement DLS                                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_loop ( void )
  { Dls_Check_top_horaire ();                                              /* Mise à jour des variables de gestion de l'heure */
    guint top = Agent_get_top ( Agent );
    if (top >= Agent_vars->next_top_5hz)                                                           /* Toutes les 1/5 secondes */
     { Agent_vars->next_top_5hz = Agent_get_top ( Agent ) + 2;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_5hz, TRUE );
       Dls_data_BI_set   ( NULL, Agent_vars->sys_flipflop_5hz, !Dls_data_BI_get ( Agent_vars->sys_flipflop_5hz) );
     }
    if (top >= Agent_vars->next_top_2hz)                                                           /* Toutes les 1/2 secondes */
     { Agent_vars->next_top_2hz = top + 5;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_2hz, TRUE );
       Dls_data_BI_set   ( NULL, Agent_vars->sys_flipflop_2hz, !Dls_data_BI_get ( Agent_vars->sys_flipflop_2hz) );
       if (Agent_vars->nbr_plugins_monitored) Dls_foreach_plugins ( Dls_Monitor_flush, NULL );
     }
    if (top >= Agent_vars->next_top_1sec)                                                              /* Toutes les secondes */
     { Agent_vars->next_top_1sec = top + 10;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_1sec, TRUE );
       Dls_data_BI_set   ( NULL, Agent_vars->sys_flipflop_1sec, !Dls_data_BI_get ( Agent_vars->sys_flipflop_1sec) );

     }
    if (top >= Agent_vars->next_top_2sec)                                                            /* Toutes les 2 secondes */
     { Agent_vars->next_top_2sec = top + 20;
       Dls_data_BI_set ( NULL, Agent_vars->sys_flipflop_2sec, !Dls_data_BI_get ( Agent_vars->sys_flipflop_2sec) );
     }
    if (top >= Agent_vars->next_top_5sec)                                                            /* Toutes les 5 secondes */
     { Agent_vars->next_top_5sec = top + 50;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_5sec, TRUE );
     }
    if (top >= Agent_vars->next_top_10sec)                                                          /* Toutes les 10 secondes */
     { Agent_vars->next_top_10sec = top + 100;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_10sec, TRUE );
       Dls_data_BI_set ( NULL, Agent_vars->sys_mqtt_connected, Agent_is_mqtt_local_connected ( Agent ) );
       if (Agent_vars->nbr_plugins_monitored) Dls_foreach_plugins ( Dls_Monitor_watchdog, NULL );
     }
    if (top >= Agent_vars->next_top_1min)                                                               /* Toutes les minutes */
     { Agent_vars->next_top_1min = top + 600;
       Dls_data_MONO_set ( NULL, Agent_vars->sys_top_1min, TRUE );
       Dls_data_AI_set ( Agent_vars->sys_bit_par_min, (gdouble)Agent_vars->audit_bit_interne_par_min, TRUE );
       Agent_vars->audit_bit_interne_par_min = 0;
       Dls_data_activer_horloge();
       Run_thread_detached ( "Running Archive Thread", (GThreadFunc)Archive_all_thread, NULL );
     }
    if (top >= Agent_vars->next_top_10min)                                                           /* Toutes les 10 minutes */
     { Agent_vars->next_top_10min = top + 6000; }

    Dls_set_edge();
    Dls_set_cde_exterieure();
    Dls_foreach_plugins ( Dls_run_plugin, NULL );                                 /* Fait tourner tous les plugins, un par un */
    Dls_reset_edge();
    Dls_reset_cde_exterieure();
    Distribuer_outputs();

    Dls_Stop_top_horaire();

    Dls_data_HORLOGE_clear();
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_5hz,   FALSE );
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_2hz,   FALSE );
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_1sec,  FALSE );
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_5sec,  FALSE );
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_10sec, FALSE );
    Dls_data_MONO_set ( NULL, Agent_vars->sys_top_1min,  FALSE );
  }
/******************************************************************************************************************************/
/* Chrono: renvoi la difference de temps entre deux structures timeval                                                        */
/* Entrée: le temps avant, et le temps apres l'action                                                                         */
/* Sortie: un float                                                                                                           */
/******************************************************************************************************************************/
 static float Chrono ( struct timeval *avant, struct timeval *apres )
  { if (!(avant && apres)) return(0.0);
    else return( apres->tv_sec - avant->tv_sec + (apres->tv_usec - avant->tv_usec)/1000000.0 );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_set_cde_exterieure ( void )
  { while( Agent_vars->Set_Dls_Data )                                                            /* A-t-on une entrée a allumer ?? */
     { struct DLS_DI *di = Agent_vars->Set_Dls_Data->data;
       Info( __func__, "dls", di->tech_id, LOG_NOTICE, "Mise a 1 du bit DI %s:%s", di->tech_id, di->acronyme );
       Agent_vars->Set_Dls_Data = g_slist_remove ( Agent_vars->Set_Dls_Data, di );
       Agent_vars->Reset_Dls_Data = g_slist_append ( Agent_vars->Reset_Dls_Data, di );
       Dls_data_DI_set ( di, TRUE );                                                             /* Mise a un du bit d'entrée */
     }
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_reset_cde_exterieure ( void )
  { while( Agent_vars->Reset_Dls_Data )                                                 /* A-t-on un monostable a éteindre ?? */
     { struct DLS_DI *di = Agent_vars->Reset_Dls_Data->data;
       Info( __func__, "dls", di->tech_id, LOG_DEBUG, "Mise a 0 du bit DI %s:%s", di->tech_id, di->acronyme );
       Agent_vars->Reset_Dls_Data = g_slist_remove ( Agent_vars->Reset_Dls_Data, di );
       Dls_data_DI_set ( di, FALSE );                                                          /* Mise a zero du bit d'entrée */
     }
  }
/****************************************************D_VARS**************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_set_edge ( void )
  { while( Agent_vars->Set_Dls_MONO_Edge_up )                                            /* A-t-on un boolean up a allumer ?? */
     { struct DLS_MONO *mono = Agent_vars->Set_Dls_MONO_Edge_up->data;
       Agent_vars->Set_Dls_MONO_Edge_up   = g_slist_remove  ( Agent_vars->Set_Dls_MONO_Edge_up, mono );
       Agent_vars->Reset_Dls_MONO_Edge_up = g_slist_prepend ( Agent_vars->Reset_Dls_MONO_Edge_up, mono );
       mono->edge_up = TRUE;
     }
    while( Agent_vars->Set_Dls_MONO_Edge_down )                                        /* A-t-on un boolean down a allumer ?? */
     { struct DLS_MONO *mono = Agent_vars->Set_Dls_MONO_Edge_down->data;
       Agent_vars->Set_Dls_MONO_Edge_down   = g_slist_remove  ( Agent_vars->Set_Dls_MONO_Edge_down, mono );
       Agent_vars->Reset_Dls_MONO_Edge_down = g_slist_prepend ( Agent_vars->Reset_Dls_MONO_Edge_down, mono );
       mono->edge_down = TRUE;
     }
    while( Agent_vars->Set_Dls_BI_Edge_up )                                              /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BI *bi = Agent_vars->Set_Dls_BI_Edge_up->data;
       Agent_vars->Set_Dls_BI_Edge_up   = g_slist_remove  ( Agent_vars->Set_Dls_BI_Edge_up, bi );
       Agent_vars->Reset_Dls_BI_Edge_up = g_slist_prepend ( Agent_vars->Reset_Dls_BI_Edge_up, bi );
       bi->edge_up = TRUE;
     }
    while( Agent_vars->Set_Dls_BI_Edge_down )                                         /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BI *bi = Agent_vars->Set_Dls_BI_Edge_down->data;
       Agent_vars->Set_Dls_BI_Edge_down   = g_slist_remove  ( Agent_vars->Set_Dls_BI_Edge_down, bi );
       Agent_vars->Reset_Dls_BI_Edge_down = g_slist_prepend ( Agent_vars->Reset_Dls_BI_Edge_down, bi );
       bi->edge_down = TRUE;
     }
    while( Agent_vars->Set_Dls_DI_Edge_up )                                             /* A-t-on un boolean up a allumer ?? */
     { struct DLS_DI *di = Agent_vars->Set_Dls_DI_Edge_up->data;
       Agent_vars->Set_Dls_DI_Edge_up   = g_slist_remove  ( Agent_vars->Set_Dls_DI_Edge_up, di );
       Agent_vars->Reset_Dls_DI_Edge_up = g_slist_prepend ( Agent_vars->Reset_Dls_DI_Edge_up, di );
       di->edge_up = TRUE;
     }
    while( Agent_vars->Set_Dls_DI_Edge_down )                                         /* A-t-on un boolean down a allumer ?? */
     { struct DLS_DI *di = Agent_vars->Set_Dls_DI_Edge_down->data;
       Agent_vars->Set_Dls_DI_Edge_down   = g_slist_remove  ( Agent_vars->Set_Dls_DI_Edge_down, di );
       Agent_vars->Reset_Dls_DI_Edge_down = g_slist_prepend ( Agent_vars->Reset_Dls_DI_Edge_down, di );
       di->edge_down = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_reset_edge ( void )
  { while( Agent_vars->Reset_Dls_MONO_Edge_up )                                          /* A-t-on un boolean up a allumer ?? */
     { struct DLS_MONO *mono = Agent_vars->Reset_Dls_MONO_Edge_up->data;
       Agent_vars->Reset_Dls_MONO_Edge_up = g_slist_remove ( Agent_vars->Reset_Dls_MONO_Edge_up, mono );
       mono->edge_up = FALSE;
     }
    while( Agent_vars->Reset_Dls_MONO_Edge_down )                                      /* A-t-on un boolean down a allumer ?? */
     { struct DLS_MONO *mono = Agent_vars->Reset_Dls_MONO_Edge_down->data;
       Agent_vars->Reset_Dls_MONO_Edge_down = g_slist_remove ( Agent_vars->Reset_Dls_MONO_Edge_down, mono );
       mono->edge_down = FALSE;
     }
    while( Agent_vars->Reset_Dls_BI_Edge_up )                                            /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BI *bi = Agent_vars->Reset_Dls_BI_Edge_up->data;
       Agent_vars->Reset_Dls_BI_Edge_up = g_slist_remove ( Agent_vars->Reset_Dls_BI_Edge_up, bi );
       bi->edge_up = FALSE;
     }
    while( Agent_vars->Reset_Dls_BI_Edge_down )                                        /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BI *bi = Agent_vars->Reset_Dls_BI_Edge_down->data;
       Agent_vars->Reset_Dls_BI_Edge_down = g_slist_remove ( Agent_vars->Reset_Dls_BI_Edge_down, bi );
       bi->edge_down = FALSE;
     }
    while( Agent_vars->Reset_Dls_DI_Edge_up )                                            /* A-t-on un boolean up a allumer ?? */
     { struct DLS_DI *di = Agent_vars->Reset_Dls_DI_Edge_up->data;
       Agent_vars->Reset_Dls_DI_Edge_up = g_slist_remove ( Agent_vars->Reset_Dls_DI_Edge_up, di );
       di->edge_up = FALSE;
     }
    while( Agent_vars->Reset_Dls_DI_Edge_down )                                        /* A-t-on un boolean down a allumer ?? */
     { struct DLS_DI *di = Agent_vars->Reset_Dls_DI_Edge_down->data;
       Agent_vars->Reset_Dls_DI_Edge_down = g_slist_remove ( Agent_vars->Reset_Dls_DI_Edge_down, di );
       di->edge_down = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_bus : Envoi un message sur le bus système                                                                     */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et les paramètres du message                                  */
/******************************************************************************************************************************/
 void Dls_data_set_bus ( struct DLS_PLUGIN *plugin, gchar *agent_tech_id, gchar *commande )
  { JsonNode *RootNode = Json_create ();
    if (RootNode)
     { Json_add_string ( RootNode, "commande", commande );
       Agent_send_mqtt_api_message ( Agent, RootNode, FALSE, "SET_BUS/%s", agent_tech_id );
       Json_unref(RootNode);
     }
  }
/******************************************************************************************************************************/
/* Dls_PID_reset: Reset les données calculées du PID                                                                          */
/* Sortie : les bits somme et prev sont mis à 0                                                                               */
/******************************************************************************************************************************/
 void Dls_PID_reset ( struct DLS_PLUGIN *plugin, struct DLS_REGISTRE *input )
  { if (!input) return;

    input->pid_somme_erreurs = 0.0;
    input->pid_prev_erreur   = 0.0;
  }
/******************************************************************************************************************************/
/* Dls_get_top: Récupètre la valeur de l'horloge                                                                              */
/* Sortie : le top horloge                                                                                                    */
/******************************************************************************************************************************/
 gint Dls_get_top ( void )
  { return (Agent_get_top ( Agent )); }
/******************************************************************************************************************************/
/* Dls_PID: Gestion du PID                                                                                                    */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 void Dls_PID ( struct DLS_PLUGIN *plugin, struct DLS_REGISTRE *input, struct DLS_REGISTRE *consigne,
                struct DLS_REGISTRE *kp,struct DLS_REGISTRE *ki, struct DLS_REGISTRE *kd,
                struct DLS_REGISTRE *outputmin, struct DLS_REGISTRE *outputmax, struct DLS_REGISTRE *output
              )
  { if ( ! (input && consigne && kp && ki && kd && outputmin && outputmax && output ) ) return;

    gdouble erreur           = consigne->valeur - input->valeur;
    input->pid_somme_erreurs+= erreur;                                /* possibilité de débordement si trop long a stabiliser */
    gdouble variation_erreur = erreur - input->pid_prev_erreur;
    gdouble result = kp->valeur * erreur + ki->valeur * input->pid_somme_erreurs + kd->valeur * variation_erreur;
    input->pid_prev_erreur = erreur;

         if (result > outputmax->valeur ) result = outputmax->valeur;
    else if (result < outputmin->valeur ) result = outputmin->valeur;
    Info( __func__, "dls", input->tech_id, LOG_DEBUG,
              "ligne %04d: Changing DLS_PID for '%s:%s'=> '%s:%s'=%f. Somme_Erreur = %f, Variation_Erreur = %f",
              (plugin ? plugin->num_ligne : -1),
              input->tech_id, input->acronyme,
              output->tech_id, output->acronyme, result,
              input->pid_somme_erreurs, variation_erreur
            );
    Dls_data_REGISTRE_set ( plugin, output, result );
  }
 /******************************************************************************************************************************/
/* Dls_sync_all_output: Envoi une synchronisation globale de toutes les sorties DO et AO                                      */
/* Entrée : le Dls_tree correspondant                D_VARS                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_sync_all_output ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { if (!plugin->handle) return;                                                 /* si plugin non chargé, on ne l'éxecute pas */
    GSList *liste = plugin->Dls_data_DO;
    while ( liste )                                                                                     /* Pour toutes les DO */
     { struct DLS_DO *bit = liste->data;
       JsonNode *RootNode = Json_create ();
       if (RootNode)
        { Dls_DO_to_json ( RootNode, bit );
          g_rw_lock_writer_lock ( &Agent_vars->Liste_DO_synchro );
          Agent_vars->Liste_DO = g_slist_append ( Agent_vars->Liste_DO, RootNode );
          g_rw_lock_writer_unlock ( &Agent_vars->Liste_DO_synchro );
        }
       else Info( __func__, "dls", NULL, LOG_ERR, "JSon RootNode creation failed" );
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_AO;
    while ( liste )                                                                                     /* Pour toutes les AO */
     { struct DLS_AO *bit = liste->data;
       JsonNode *RootNode = Json_create ();
       if (RootNode)
        { Dls_AO_to_json ( RootNode, bit );
          g_rw_lock_writer_lock ( &Agent_vars->Liste_AO_synchro );
          Agent_vars->Liste_AO = g_slist_append ( Agent_vars->Liste_AO, RootNode );
          g_rw_lock_writer_unlock ( &Agent_vars->Liste_AO_synchro );
        }
       else Info( __func__, "dls", NULL, LOG_ERR, "JSon RootNode creation failed" );
       liste = g_slist_next ( liste );
     }
  }
/******************************************************************************************************************************/
/* Dls_run_plugin: Fait tourner les DLS synoptique en parametre                                                               */
/* Entrée : le plugin DLS correspondant                                                                                       */
/* Sortie : rien                                                                                                              */
/* Synchronisation: appelée via Dls_foreach_plugins, qui protège le plugin avec Dls_plugins_lock en lecture                   */
/******************************************************************************************************************************/
 void Dls_run_plugin ( struct DLS_PLUGIN *plugin, gpointer user_data )
  { struct timeval tv_avant, tv_apres;
    if (!plugin->handle) return;                                                 /* si plugin non chargé, on ne l'éxecute pas */

/*--------------------------------------------- Calcul des bits internals ----------------------------------------------------*/
    gboolean bit_comm_module = TRUE;
    GSList *liste = plugin->Arbre_Comm;
    while ( liste )                                                   /* Calcul de la COMM du DLS a partir de ses dependances */
     { struct DLS_WATCHDOG *bit = liste->data;
       bit_comm_module &= Dls_data_WATCHDOG_get( bit );
       liste = g_slist_next ( liste );
     }

    if ( Dls_data_MONO_get ( plugin->dls_comm ) != bit_comm_module )                    /* Mise à jour si écart */
     { Dls_data_MONO_set ( plugin, plugin->dls_comm, bit_comm_module );
       Dls_MONO_report_to_API ( plugin->dls_comm );
     }

/*-------------------------------------------------- Calcul du MEMSA_OK ------------------------------------------------------*/
    gboolean new_memsa_ok = bit_comm_module && !( Dls_data_MONO_get( plugin->dls_memsa_defaut ) ||
                                                  Dls_data_MONO_get( plugin->dls_memsa_defaut_fixe ) ||
                                                  Dls_data_MONO_get( plugin->dls_memsa_alarme ) ||
                                                  Dls_data_MONO_get( plugin->dls_memsa_alarme_fixe )
                                                );
    Dls_data_MONO_set ( plugin, plugin->dls_memsa_ok, new_memsa_ok );

/*-------------------------------------------------- Calcul du MEMSSP_OK -----------------------------------------------------*/
    Dls_data_MONO_set ( plugin, plugin->dls_memssp_ok,
                        !( Dls_data_MONO_get( plugin->dls_memssp_derangement ) ||
                           Dls_data_MONO_get( plugin->dls_memssp_derangement_fixe ) ||
                           Dls_data_MONO_get( plugin->dls_memssp_danger ) ||
                           Dls_data_MONO_get( plugin->dls_memssp_danger_fixe )
                         )
                      );

/*----------------------------------------------- Mise a jour des messages de comm -------------------------------------------*/
   if (bit_comm_module) Dls_data_MESSAGE_set ( plugin, plugin->dls_msg_comm_ok );
                   else Dls_data_MESSAGE_set ( plugin, plugin->dls_msg_comm_hs );

/*----------------------------------------------- Lancement du plugin --------------------------------------------------------*/
    gettimeofday( &tv_avant, NULL );
    if (plugin->enable && plugin->go)                                                  /* Si plugin enabled ET fonction go ok */
     { if(plugin->restart)
        { Info( __func__, "dls", plugin->tech_id, LOG_INFO, "Send '_START' to '%s'", plugin->tech_id ); }
       plugin->go( plugin );                                                                            /* On appel le plugin */
     }
    Dls_data_MESSAGE_apply ( plugin );                                             /* Application des nouveaux etats messages */
    Dls_data_VISUEL_apply ( plugin );
    plugin->restart = FALSE;
    gettimeofday( &tv_apres, NULL );
    plugin->conso+=Chrono( &tv_avant, &tv_apres );                                                         /* Ajoute la conso */
  }
/******************************************************************************************************************************/
/* main: Prend en charge l'agent                                                                                              */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: aucune                                                                                                             */
/******************************************************************************************************************************/
 gint main(gint argc, gchar *argv[])
  { setenv ( "ABLS_AGENT_TECH_ID", "SYS", 1 );
    setenv ( "ABLS_TPS", "100", 1 );
    Config_add_parameter ( "audio-tech-id", "AUDIO_TECH_ID", "Tech ID owning the audio zone DI", CONFIG_STRING );
    Agent = Agent_init ( argv[0], "dls", ABLS_AGENT_DLS_VERSION, sizeof(struct DLS_VARS), argc, argv );
    Agent_vars = Agent_get_vars ( Agent );

    Agent_subscribe_mqtt_local ( Agent, "SET_AI/+/+" );
    Agent_subscribe_mqtt_local ( Agent, "SET_DI/+/+" );
    Agent_subscribe_mqtt_local ( Agent, "SET_WATCHDOG/+/+" );
    Agent_subscribe_mqtt_local ( Agent, "SET_DI_PULSE/+/+" );
    Agent_subscribe_mqtt_local ( Agent, "SET_CI_PULSE/+/+" );
    Agent_subscribe_mqtt_api   ( Agent, "%s/DLS/RELOAD/+", Agent_get_domain_uuid ( Agent ) );
    Agent_subscribe_mqtt_api   ( Agent, "%s/DLS/MONITOR/+", Agent_get_domain_uuid ( Agent ) );
    Agent_subscribe_mqtt_api   ( Agent, "%s/DLS/AUDIO_ZONE/RENAME", Agent_get_domain_uuid ( Agent ) );
    Agent_subscribe_mqtt_api   ( Agent, "%s/SYNOPTIQUE/CLIC", Agent_get_domain_uuid ( Agent ) );

    Mnemo_create_AI   ( Agent, "BIT_PAR_MIN",   "Nombre de changements d'etat par minute", "bit/min", AGENT_ARCHIVE_1_MIN );
    Mnemo_create_MONO ( Agent, "TOP_1MIN",      "Impulsion toutes les minutes" );
    Mnemo_create_MONO ( Agent, "TOP_1SEC",      "Impulsion toutes les secondes" );
    Mnemo_create_MONO ( Agent, "TOP_5SEC",      "Impulsion toutes les 5 secondes" );
    Mnemo_create_MONO ( Agent, "TOP_10SEC",     "Impulsion toutes les 10 secondes" );
    Mnemo_create_MONO ( Agent, "TOP_2HZ",       "Impulsion toutes les demi-secondes" );
    Mnemo_create_MONO ( Agent, "TOP_5HZ",       "Impulsion toutes les 1/5 secondes" );
    Mnemo_create_BI   ( Agent, "MQTT_CONNECTED","TRUE si l'agent est connecté au MQTT" );
    Mnemo_create_BI   ( Agent, "FLIPFLOP_2SEC", "Creneaux d'une durée de deux secondes" );
    Mnemo_create_BI   ( Agent, "FLIPFLOP_1SEC", "Creneaux d'une durée d'une seconde" );
    Mnemo_create_BI   ( Agent, "FLIPFLOP_2HZ",  "Creneaux d'une durée d'une demi seconde" );
    Mnemo_create_BI   ( Agent, "FLIPFLOP_5HZ",  "Creneaux d'une durée d'un 5ième de seconde" );
    Mnemo_create_DI   ( Agent, "TOP_ALERTE_1",  "Demande d'alerte" );
    Mnemo_create_DI   ( Agent, "TOP_ALERTE_2",  "Demande d'alerte" );

    Agent_is_ready ( Agent );

    Dls_init();
    Agent_status_push ( Agent, "D.L.S Running" );

    while(Agent_is_running ( Agent ))                                              /* On tourne tant que necessaire */
     { Agent_loop ( Agent );                                             /* Loop sur l'agent pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       JsonNode *mqtt_local_message;
       while ( (mqtt_local_message = Agent_get_mqtt_local_message ( Agent ) ) != NULL )
        { if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_AI", "+", "+" ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Mqtt_get_topic_lvl ( mqtt_local_message, 1 ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Mqtt_get_topic_lvl ( mqtt_local_message, 2 ) );
             Dls_data_AI_set_from_thread_ai ( mqtt_local_message );
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_DI", "+", "+" ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Mqtt_get_topic_lvl ( mqtt_local_message, 1 ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Mqtt_get_topic_lvl ( mqtt_local_message, 2 ) );
             Dls_data_DI_set_from_thread_di ( mqtt_local_message );
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_WATCHDOG", "+", "+" ))
           { Json_add_string ( mqtt_local_message, "agent_tech_id", Mqtt_get_topic_lvl ( mqtt_local_message, 1 ) );
             Json_add_string ( mqtt_local_message, "agent_acronyme", Mqtt_get_topic_lvl ( mqtt_local_message, 2 ) );
             Dls_data_WATCHDOG_set_from_thread_watchdog ( mqtt_local_message );
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_DI_PULSE", "+", "+" ) )
           { gchar *from_agent_tech_id = Json_get_string ( mqtt_local_message, "from_agent_tech_id" );
             gchar *tech_id  = Mqtt_get_topic_lvl ( mqtt_local_message, 1 );
             gchar *acronyme = Mqtt_get_topic_lvl ( mqtt_local_message, 2 );
             if (!from_agent_tech_id)
              { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR,
                     "SET_DI_PULSE: 'from_agent_tech_id' is missing" ); }
             else
              { struct DLS_DI *bit = Dls_data_DI_lookup ( tech_id, acronyme );
                if (!bit) Info( __func__, Agent_get_classe ( Agent ), tech_id, LOG_ERR,
                                "SET_DI_PULSE from '%s': '%s:%s' not found. Dropping.",
                                from_agent_tech_id, tech_id, acronyme );
                else
                 { Info( __func__, Agent_get_classe ( Agent ), from_agent_tech_id, LOG_INFO,
                         "SET_DI_PULSE: '%s:%s'=PULSE", tech_id, acronyme );
                   Dls_data_DI_set_pulse ( NULL, bit );
                 }
              }
           }
          else if (Mqtt_topic_is ( mqtt_local_message, 3, "SET_CI_PULSE", "+", "+" ) )
           { gchar *tech_id  = Mqtt_get_topic_lvl ( mqtt_local_message, 1 );
             gchar *acronyme = Mqtt_get_topic_lvl ( mqtt_local_message, 2 );
             struct DLS_CI *bit = Dls_data_CI_lookup ( tech_id, acronyme );
             if (!bit) Info( __func__, Agent_get_classe ( Agent ), tech_id, LOG_ERR,
                             "SET_CI_PULSE: '%s:%s' not found. Dropping.", tech_id, acronyme );
             else
              { Info( __func__, Agent_get_classe ( Agent ), tech_id, LOG_INFO,
                      "SET_CI_PULSE: '%s:%s'=PULSE", tech_id, acronyme );
                Dls_data_CI_set_pulse ( NULL, bit );
              }
           }
          Json_unref ( mqtt_local_message );
        }
/****************************************************** Ecoute de l'api *******************************************************/
       JsonNode *mqtt_api_message;
       while ( (mqtt_api_message = Agent_get_mqtt_api_message ( Agent ) ) != NULL )
        { if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "AGENT", Agent_get_tech_id ( Agent ), "TEST" ) )
           { Info(__func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "Agent Test from API."); }
/*------------------------------------------------------------ Remap ---------------------------------------------------------*/
          else if ( Mqtt_topic_is ( mqtt_api_message, 3, "+", "DLS", "REMAP" ) )
           { MAP_Remap(); }
          else if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "DLS", "RELOAD", "+" ) )
           { gchar *target = Mqtt_get_topic_lvl ( mqtt_api_message, 3 );
             Dls_Reload_un_plugin ( target );                                            /* Use thread_pool donc non bloquant */
           }
          else if ( Mqtt_topic_is ( mqtt_api_message, 3, "+", "DLS", "RELOAD_HORLOGE_TICK" ) )
           { Dls_Load_horloge_ticks(); }
          else if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "DLS", "MONITOR", "+" ) )
           { gchar *target = Mqtt_get_topic_lvl ( mqtt_api_message, 3 );
             Dls_Monitor_set ( target, Json_get_bool ( mqtt_api_message, "enable" ) );
           }
          else if ( Mqtt_topic_is ( mqtt_api_message, 4, "+", "DLS", "AUDIO_ZONE", "RENAME" ) )
           { if ( !Json_has_member ( mqtt_api_message, "old_audio_zone_name" ) ||
                  !Json_has_member ( mqtt_api_message, "audio_zone_name" ) ||
                  !Json_has_member ( mqtt_api_message, "description" ) )
              { Info( __func__, "audio", Agent_get_tech_id ( Agent ), LOG_ERR,
                      "AUDIO_ZONE/RENAME: old_audio_zone_name, audio_zone_name or description is missing" ); }
             else
              { Info( __func__, "audio", Agent_get_tech_id ( Agent ), LOG_NOTICE,
                      "AUDIO_ZONE/RENAME: '%s' -> '%s' ('%s')",
                      Json_get_string ( mqtt_api_message, "old_audio_zone_name" ),
                      Json_get_string ( mqtt_api_message, "audio_zone_name" ),
                      Json_get_string ( mqtt_api_message, "description" ) );
                Dls_foreach_plugins ( Dls_plugin_update_audio_zone, mqtt_api_message );
              }
           }
          else if ( Mqtt_topic_is ( mqtt_api_message, 3, "+", "SYNOPTIQUE", "CLIC" ) )
           { if ( !Json_has_member ( mqtt_api_message, "tech_id" ) )
              { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR,
                     "SYN_CLIC: tech_id is missing" ); }
             else if ( !Json_has_member ( mqtt_api_message, "acronyme" ) )
              { Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_ERR,
                     "SYN_CLIC: acronyme is missing" ); }
             else
              { gchar *tech_id  = Json_get_string ( mqtt_api_message, "tech_id" );
                gchar *acronyme = Json_get_string ( mqtt_api_message, "acronyme" );
                struct DLS_DI *bit = Dls_data_DI_lookup ( tech_id, acronyme );
                if (!bit) Info( __func__, Agent_get_classe ( Agent ), tech_id, LOG_ERR,
                                "SYN_CLIC: '%s:%s' not found. Dropping.", tech_id, acronyme );
                else Dls_data_DI_set_pulse ( NULL, bit );
              }
           }
          else Info( __func__, Agent_get_classe ( Agent ), Agent_get_tech_id ( Agent ), LOG_NOTICE, "API sent unknown command %s", Json_get_string ( mqtt_api_message, "mqtt_topic" ) );
          Json_unref (mqtt_api_message);
        }
/********************************************************** Running DLS *******************************************************/
        Dls_loop();
     }

    Dls_end();
    Agent_end(Agent);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

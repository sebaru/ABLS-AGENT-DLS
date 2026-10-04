/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/include/dls.h  Declarations pour l'agent DLS                                                               */
/* Projet Abls-Habitat                   Gestion d'habitat                                                01.08.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls.h
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

#ifndef _ABLS_AGENT_DLS_H_
 #define _ABLS_AGENT_DLS_H_

 #include <abls-agent-libs.h>
 #include "dls_plugin.h"
 #include "map.h"
 #include "heure.h"
 #include "archive.h"

 struct DLS_VARS
  { gboolean initialized;
    GSList *Dls_plugins;
    GRWLock Dls_plugins_lock;

    GSList *Set_Dls_DI_Edge_up;
    GSList *Set_Dls_DI_Edge_down;
    GSList *Set_Dls_MONO_Edge_up;
    GSList *Set_Dls_MONO_Edge_down;
    GSList *Set_Dls_BI_Edge_up;
    GSList *Set_Dls_BI_Edge_down;
    GSList *Set_Dls_Data;
    GSList *Reset_Dls_DI_Edge_up;
    GSList *Reset_Dls_DI_Edge_down;
    GSList *Reset_Dls_MONO_Edge_up;
    GSList *Reset_Dls_MONO_Edge_down;
    GSList *Reset_Dls_BI_Edge_up;
    GSList *Reset_Dls_BI_Edge_down;
    GSList *Reset_Dls_Data;
    GSList *HORLOGE_actives;
    JsonNode *HORLOGE_ticks;

    struct DLS_BI *sys_flipflop_5hz;
    struct DLS_BI *sys_flipflop_2hz;
    struct DLS_BI *sys_flipflop_1sec;
    struct DLS_BI *sys_flipflop_2sec;
    struct DLS_BI *sys_mqtt_connected;
    struct DLS_MONO *sys_top_5hz;
    struct DLS_MONO *sys_top_2hz;
    struct DLS_MONO *sys_top_1sec;
    struct DLS_MONO *sys_top_5sec;
    struct DLS_MONO *sys_top_10sec;
    struct DLS_MONO *sys_top_1min;
    struct DLS_AI *sys_bit_per_sec;
    struct DLS_AI *sys_tour_per_sec;
    struct DLS_AI *sys_dls_wait;
    struct DLS_AI *sys_maxrss;
    struct DLS_AI *sys_log_per_min;

    GRWLock Liste_DO_synchro;
    GSList *Liste_DO;
    GRWLock Liste_AO_synchro;
    GSList *Liste_AO;
    GRWLock Liste_msg_synchro;
    GSList *Liste_msg;

    guint audit_bit_interne_per_sec;
    guint audit_bit_interne_per_sec_hold;
    guint nbr_plugins_monitored;                      /* Nombre de plugins en cours de monitoring, pour court-circuiter vite */

    guint next_top_2hz;
    guint next_top_5hz;
    guint next_top_1sec;
    guint next_top_2sec;
    guint next_top_5sec;
    guint next_top_10sec;
    guint next_top_1min;
    guint next_top_10min;
    guint last_top;
    GThreadPool *Thread_import_plugin_pool;                                                     /* Pool de threads de l'agent */
  };

 enum
  { MSG_ETAT,
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_NOTIF,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

 struct DLS_MESSAGE_EVENT
  { struct DLS_MESSAGE *msg;
    gboolean etat;
  };

 enum                                                              /* Classes de bits remontées par le monitoring temps réel */
  { DLS_MONITOR_AI = 1,
    DLS_MONITOR_AO,
    DLS_MONITOR_BI,
    DLS_MONITOR_CH,
    DLS_MONITOR_CI,
    DLS_MONITOR_DI,
    DLS_MONITOR_DO,
    DLS_MONITOR_MONO,
    DLS_MONITOR_REGISTRE,
    DLS_MONITOR_VISUEL,
    DLS_MONITOR_WATCHDOG,
    DLS_MONITOR_MSG
  };

 extern struct ABLS_AGENT *Agent;                                                                 /* Structure de l'agent DLS */
 extern struct DLS_VARS *Agent_vars;                                                /* Structure des variables de l'agent DLS */

 extern void Dls_init ( void );
 extern void Dls_end ( void );
 extern void Dls_loop ( void );

 extern void Dls_set_cde_exterieure ( void );
 extern void Dls_reset_cde_exterieure ( void );
 extern void Dls_set_edge ( void );
 extern void Dls_reset_edge ( void );
 extern void Dls_run_plugin ( struct DLS_PLUGIN *plugin );

 extern void Dls_Decharger_un_plugin ( gchar *tech_id );
 extern void Dls_Decharger_plugins ( void );
 extern void Dls_Reload_un_plugin ( gchar *tech_id );
 extern void Dls_Importer_un_plugin ( gpointer data, gpointer user_data );
 extern void Dls_Importer_plugins ( void );
 extern void Dls_Activer_plugin ( gchar *tech_id, gboolean actif );
 extern void Dls_foreach_plugins ( void (*do_plugin) (struct DLS_PLUGIN *) );
 extern void Dls_Acquitter_plugin ( gchar *tech_id );
 extern struct DLS_PLUGIN *Dls_get_plugin_by_tech_id ( gchar *tech_id );
 extern void Dls_sync_all_output ( gpointer user_data, struct DLS_PLUGIN *plugin );
 extern void Dls_Save_Data_to_API ( struct DLS_PLUGIN *plugin );

 extern void Dls_Monitor_set ( gchar *tech_id, gboolean actif );
 extern void Dls_Monitor_mark ( struct DLS_PLUGIN *plugin, gint classe, gpointer bit );
 extern void Dls_Monitor_mark_by_tech_id ( gint classe, gchar *tech_id, gpointer bit );
 extern void Dls_Monitor_flush ( struct DLS_PLUGIN *plugin );
 extern void Dls_Monitor_watchdog ( struct DLS_PLUGIN *plugin );
 extern void Dls_Monitor_clear ( struct DLS_PLUGIN *plugin );
 extern void Dls_Monitor_stop ( struct DLS_PLUGIN *plugin );

 extern void Dls_data_CI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_CI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_CI_report_to_API ( struct DLS_CI *bit );
 extern void Dls_data_CH_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_CH_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_CH_report_to_API ( struct DLS_CH *bit );
 extern void Dls_data_AI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_AI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_AI_to_json ( JsonNode *element, struct DLS_AI *bit );
 extern void Dls_data_AI_set ( struct DLS_AI *bit, gdouble valeur, gboolean in_range );
 extern gboolean Dls_data_AI_set_from_thread_ai ( JsonNode *request );
 extern void Dls_AI_report_to_API ( struct DLS_AI *bit );
 extern void Dls_data_AO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_AO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit );
 extern void Dls_AO_report_to_API ( struct DLS_AO *bit );
 extern void Dls_data_TEMPO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_TEMPO_to_json ( JsonNode *element, struct DLS_TEMPO *bit );
 extern void Dls_data_REGISTRE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_REGISTRE_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_REGISTRE_report_to_API ( struct DLS_REGISTRE *bit );
 extern void Dls_data_DI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_DI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_DI_to_json ( JsonNode *element, struct DLS_DI *bit );
 extern void Dls_data_DI_set ( struct DLS_DI *bit, gboolean valeur );
 extern gboolean Dls_data_DI_set_from_thread_di ( JsonNode *request );
 extern void Dls_DI_report_to_API ( struct DLS_DI *bit );
 extern void Dls_data_DO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_DO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit );
 extern void Dls_DO_report_to_API ( struct DLS_DO *bit );
 extern void Dls_data_MONO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_MONO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_MONO_report_to_API ( struct DLS_MONO *bit );
 extern void Dls_data_BI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_BI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_BI_report_to_API ( struct DLS_BI *bit );
 extern void Dls_data_HORLOGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_data_HORLOGE_clear ( void );
 extern void Dls_data_activer_horloge ( void );
 extern void Dls_Load_horloge_ticks ( void );
 extern void Dls_data_VISUEL_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_VISUEL_to_json ( JsonNode *RootNode, struct DLS_VISUEL *bit );
 extern void Dls_data_VISUEL_apply ( struct DLS_PLUGIN *plugin );
 extern void Dls_data_MESSAGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_data_MESSAGE_free_all ( struct DLS_PLUGIN *plugin );
 extern void Dls_data_MESSAGE_apply ( struct DLS_PLUGIN *plugin );
 extern void Dls_data_WATCHDOG_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern gboolean Dls_data_WATCHDOG_set_from_thread_watchdog ( JsonNode *request );

 extern void Distribuer_messages( void );                                                        /* Distribution des messages */
 extern void Distribuer_outputs( void );                                                 /* Distribution des sorties DO et AO */
 extern gchar *Convert_libelle_dynamique( gchar *libelle_src );                               /* Conversion libelle dynamique */

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/

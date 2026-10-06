/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/src/dls_monitor.c  Remontée temps réel des bits internes d'un plugin D.L.S vers l'API                       */
/* Projet Abls-Habitat                   Gestion d'habitat                                                01.10.2026 12:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls_monitor.c
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

 #include <glib.h>
 #include <string.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "dls.h"

 #define FACILITY_MONITOR          "monitor"
 #define DLS_MONITOR_WATCHDOG_TOP  600                 /* 60 secondes sans ordre de l'API et le monitoring s'arrete tout seul */

/******************************************************************************************************************************/
/* Dls_Monitor_bit_to_json: Formate un bit, quelle que soit sa classe, au format JSON attendu par la console                  */
/* Entrée: le JsonNode cible, la classe du bit et le pointeur du bit                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Monitor_bit_to_json ( JsonNode *element, gint classe, gpointer bit )
  { switch (classe)
     { case DLS_MONITOR_AI:
        { struct DLS_AI *target = bit;
          Json_add_string ( element, "classe",   "AI" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_double ( element, "valeur",   target->valeur );
          Json_add_string ( element, "unite",    target->unite );
          Json_add_bool   ( element, "in_range", target->in_range );
          break;
        }
       case DLS_MONITOR_AO:
        { struct DLS_AO *target = bit;
          Json_add_string ( element, "classe",   "AO" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_double ( element, "valeur",   target->valeur );
          Json_add_string ( element, "unite",    target->unite );
          break;
        }
       case DLS_MONITOR_BI:
        { struct DLS_BI *target = bit;
          Json_add_string ( element, "classe",   "BI" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_CH:
        { struct DLS_CH *target = bit;
          Json_add_string ( element, "classe",   "CH" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_int    ( element, "valeur",   target->valeur );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_CI:
        { struct DLS_CI *target = bit;
          Json_add_string ( element, "classe",   "CI" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_int    ( element, "valeur",   target->valeur );
          Json_add_string ( element, "unite",    target->unite );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_DI:
        { struct DLS_DI *target = bit;
          Json_add_string ( element, "classe",   "DI" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_DO:
        { struct DLS_DO *target = bit;
          Json_add_string ( element, "classe",   "DO" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_MONO:
        { struct DLS_MONO *target = bit;
          Json_add_string ( element, "classe",   "MONO" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     target->etat );
          break;
        }
       case DLS_MONITOR_REGISTRE:
        { struct DLS_REGISTRE *target = bit;
          Json_add_string ( element, "classe",   "REGISTRE" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_double ( element, "valeur",   target->valeur );
          Json_add_string ( element, "unite",    target->unite );
          break;
        }
       case DLS_MONITOR_VISUEL:
        { struct DLS_VISUEL *target = bit;
          Json_add_string ( element, "classe",   "VISUEL" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_string ( element, "mode",     target->mode );
          Json_add_string ( element, "color",    target->color );
          Json_add_string ( element, "libelle",  target->libelle );
          Json_add_string ( element, "badge",    target->badge );
          Json_add_double ( element, "valeur",   target->valeur );
          Json_add_string ( element, "unite",    target->unite );
          Json_add_bool   ( element, "cligno",   target->cligno );
          Json_add_bool   ( element, "noshow",   target->noshow );
          Json_add_bool   ( element, "disable",  target->disable );
          break;
        }
       case DLS_MONITOR_WATCHDOG:
        { struct DLS_WATCHDOG *target = bit;
          Json_add_string ( element, "classe",   "WATCHDOG" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     Dls_data_WATCHDOG_get ( target ) );
          Json_add_int    ( element, "decompte", Dls_data_WATCHDOG_get_time ( target ) );
          break;
        }
       case DLS_MONITOR_MSG:
        { struct DLS_MESSAGE *target = bit;
          Json_add_string ( element, "classe",   "MSG" );
          Json_add_string ( element, "tech_id",  target->tech_id );
          Json_add_string ( element, "acronyme", target->acronyme );
          Json_add_bool   ( element, "etat",     target->etat );
          Json_add_string ( element, "libelle",  target->libelle_converted );
          break;
        }
       default: Json_add_string ( element, "classe", "UNKNOWN" );
     }
  }
/******************************************************************************************************************************/
/* Dls_Monitor_add_bit: Ajoute un bit dans le tableau JSON des bits remontés                                                  */
/* Entrée: le tableau JSON, la classe du bit et le pointeur du bit                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Monitor_add_bit_in_array ( JsonArray *array_bits, gint classe, gpointer bit )
  { JsonNode *element = Json_create();
    if (!element) return;
    Dls_Monitor_bit_to_json ( element, classe, bit );
    Json_array_add_element ( array_bits, element );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_add_changed_bit: Callback de parcours de la table des bits modifiés                                            */
/* Entrée: le pointeur du bit (clé), la classe (valeur), le tableau JSON cible                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Monitor_add_changed_bit ( gpointer bit, gpointer classe, gpointer array_bits )
  { Dls_Monitor_add_bit_in_array ( array_bits, GPOINTER_TO_INT(classe), bit ); }
/******************************************************************************************************************************/
/* Dls_Monitor_add_all_bits: Ajoute tous les bits d'une liste de classe donnée dans le tableau JSON                           */
/* Entrée: le tableau JSON, la liste des bits et leur classe                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Monitor_add_all_bits_in_array ( JsonArray *array_bits, GSList *liste, gint classe )
  { while (liste)
     {
       Dls_Monitor_add_bit_in_array ( array_bits, classe, liste->data );
       liste = g_slist_next(liste);
     }
  }
/******************************************************************************************************************************/
/* Dls_Monitor_new_message: Prépare l'enveloppe JSON d'un message de monitoring                                               */
/* Entrée: le plugin, le flag 'full' (TRUE pour un instantané complet) et l'adresse du tableau de bits à remplir              */
/* Sortie: le JsonNode, ou NULL si erreur mémoire                                                                             */
/******************************************************************************************************************************/
 static JsonNode *Dls_Monitor_new_message ( struct DLS_PLUGIN *plugin, gboolean full, JsonArray **bits )
  { JsonNode *RootNode = Json_create();
    if (!RootNode) return(NULL);
    Json_add_string ( RootNode, "tech_id", plugin->tech_id );
    Json_add_bool   ( RootNode, "full",    full );
    *bits = Json_add_array ( RootNode, "bits" );
    return(RootNode);
  }
/******************************************************************************************************************************/
/* Dls_Monitor_snapshot: Envoie l'intégralité des bits du plugin, pour que la console parte d'un état cohérent                */
/* Entrée: le plugin                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Monitor_snapshot ( struct DLS_PLUGIN *plugin )
  { JsonArray *bits = NULL;
    JsonNode *RootNode = Dls_Monitor_new_message ( plugin, TRUE, &bits );
    if (!RootNode)
     { Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_ERR, "'%s': memory error", plugin->tech_id ); return; }

    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_AI,       DLS_MONITOR_AI );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_AO,       DLS_MONITOR_AO );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_BI,       DLS_MONITOR_BI );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_CH,       DLS_MONITOR_CH );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_CI,       DLS_MONITOR_CI );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_DI,       DLS_MONITOR_DI );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_DO,       DLS_MONITOR_DO );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_MONO,     DLS_MONITOR_MONO );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_REGISTRE, DLS_MONITOR_REGISTRE );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_VISUEL,   DLS_MONITOR_VISUEL );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_WATCHDOG, DLS_MONITOR_WATCHDOG );
    Dls_Monitor_add_all_bits_in_array ( bits, plugin->Dls_data_MESSAGE,  DLS_MONITOR_MSG );

    Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_INFO, "'%s': sending %d bits snapshot",
          plugin->tech_id, json_array_get_length ( bits ) );
    Agent_send_mqtt_api_message ( Agent, RootNode, FALSE, "DLS_MONITOR/%s", plugin->tech_id );
    Json_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_flush: Envoie à l'API les bits modifiés depuis le dernier appel. Appelé au top 2Hz                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: néant                                                                                                              */
/* Synchronisation: appelée via Dls_foreach_plugins, qui protège le plugin avec Dls_plugins_lock en lecture                   */
/******************************************************************************************************************************/
 void Dls_Monitor_flush ( struct DLS_PLUGIN *plugin, gpointer user_data )
  { if (!(plugin && plugin->debug && plugin->monitor_changed)) return;
    if (!g_hash_table_size ( plugin->monitor_changed )) return;

    JsonArray *bits = NULL;
    JsonNode *RootNode = Dls_Monitor_new_message ( plugin, FALSE, &bits );
    if (!RootNode)
     { Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_ERR, "'%s': memory error", plugin->tech_id );
       g_hash_table_remove_all ( plugin->monitor_changed );
       return;
     }

    g_hash_table_foreach ( plugin->monitor_changed, Dls_Monitor_add_changed_bit, bits );
    g_hash_table_remove_all ( plugin->monitor_changed );

    Agent_send_mqtt_api_message ( Agent, RootNode, FALSE, "DLS_MONITOR/%s", plugin->tech_id );
    Json_unref ( RootNode );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_mark: Mémorise un bit modifié par le plugin en cours d'execution                                               */
/* Entrée: le plugin qui réalise la modification, la classe du bit et le pointeur du bit                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Monitor_mark ( struct DLS_PLUGIN *plugin, gint classe, gpointer bit )
  { if (!(plugin && plugin->debug && plugin->monitor_changed && bit)) return;
    g_hash_table_insert ( plugin->monitor_changed, bit, GINT_TO_POINTER(classe) );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_mark_by_tech_id: Idem, pour les bits positionnés hors execution d'un plugin (AI/DI venant des threads)         */
/* Entrée: la classe du bit, le tech_id du plugin propriétaire et le pointeur du bit                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Monitor_mark_by_tech_id ( gint classe, gchar *tech_id, gpointer bit )
  { if (!Agent_vars->nbr_plugins_monitored || !tech_id) return;

    g_rw_lock_reader_lock ( &Agent_vars->Dls_plugins_lock );
    GSList *plugins = Agent_vars->Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp ( plugin->tech_id, tech_id ))
        { Dls_Monitor_mark ( plugin, classe, bit );
          break;
        }
       plugins = g_slist_next(plugins);
     }
    g_rw_lock_reader_unlock ( &Agent_vars->Dls_plugins_lock );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_clear: Vide la table des bits modifiés d'un plugin                                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: néant                                                                                                              */
/* Synchronisation: appelée via Dls_foreach_plugins, qui protège le plugin avec Dls_plugins_lock en lecture                   */
/******************************************************************************************************************************/
 void Dls_Monitor_clear ( struct DLS_PLUGIN *plugin, gpointer user_data )
  { if (plugin && plugin->monitor_changed) g_hash_table_remove_all ( plugin->monitor_changed ); }
/******************************************************************************************************************************/
/* Dls_Monitor_stop: Arrete le monitoring d'un plugin et libère la table associée                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Monitor_stop ( struct DLS_PLUGIN *plugin )
  { if (!plugin) return;
    if (plugin->debug && Agent_vars->nbr_plugins_monitored) Agent_vars->nbr_plugins_monitored--;
    plugin->debug = FALSE;
    plugin->monitor_until_top = 0;
    if (plugin->monitor_changed)
     { g_hash_table_destroy ( plugin->monitor_changed );
       plugin->monitor_changed = NULL;
     }
  }
/******************************************************************************************************************************/
/* Dls_Monitor_watchdog: Coupe le monitoring si l'API ne l'a pas relancé. Appelé toutes les 10 secondes                       */
/* Entrée: le plugin                                                                                                          */
/* Sortie: néant                                                                                                              */
/* Synchronisation: appelée via Dls_foreach_plugins, qui protège le plugin avec Dls_plugins_lock en lecture                   */
/******************************************************************************************************************************/
 void Dls_Monitor_watchdog ( struct DLS_PLUGIN *plugin, gpointer user_data )
  { if (!(plugin && plugin->debug)) return;
    if (Agent_get_top ( Agent ) < plugin->monitor_until_top) return;

    Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_NOTICE,
          "'%s' monitoring stopped by watchdog ('%s')", plugin->tech_id, plugin->name );
    Dls_Monitor_stop ( plugin );
  }
/******************************************************************************************************************************/
/* Dls_Monitor_set: Active ou désactive le monitoring d'un plugin, sur ordre de l'API                                         */
/* Entrée: le tech_id du plugin et le choix actif ou non                                                                      */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_Monitor_set ( gchar *tech_id, gboolean actif )
  { if (!tech_id)
     { Info( __func__, FACILITY_MONITOR, NULL, LOG_ERR, "tech_id is null." ); return; }

    struct DLS_PLUGIN *plugin = Dls_get_plugin_by_tech_id ( tech_id );
    if (!plugin)
     { Info( __func__, FACILITY_MONITOR, tech_id, LOG_ERR, "Plugin '%s' not found", tech_id ); return; }

    if (!actif)
     { if (!plugin->debug) return;
       Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_NOTICE,
             "'%s' monitoring stopped ('%s')", plugin->tech_id, plugin->name );
       Dls_Monitor_stop ( plugin );
       return;
     }

    plugin->monitor_until_top = Agent_get_top ( Agent ) + DLS_MONITOR_WATCHDOG_TOP;           /* Rearmement du chien de garde */
    if (plugin->debug) return;                                               /* Déjà actif: simple prolongation du monitoring */

    plugin->monitor_changed = g_hash_table_new ( NULL, NULL );               /* Clé = pointeur du bit: ni hash ni egal custom */
    plugin->debug = TRUE;
    Agent_vars->nbr_plugins_monitored++;
    Info( __func__, FACILITY_MONITOR, plugin->tech_id, LOG_NOTICE,
          "'%s' monitoring started ('%s')", plugin->tech_id, plugin->name );
    Dls_Monitor_snapshot ( plugin );                                  /* Envoi du snapshot a l'API pour le premier monitoring */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

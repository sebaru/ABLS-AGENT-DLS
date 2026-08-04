/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_archive.c  Gestion des archives                                                                      */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                27.11.2022 18:09:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_archive.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "dls.h"

 #define FACILITY_ARCHIVE "archive"

 /******************************************************************************************************************************/
/* Dls_Send_archive_to_API: Ajoute une archive dans la base de données                                                        */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Dls_Send_archive_to_API( gchar *tech_id, gchar *acronyme, gdouble valeur )
  { Info( __func__, FACILITY_ARCHIVE, tech_id, LOG_DEBUG, "Add Arch in list: '%s:%s'=%f", tech_id, acronyme, valeur );
    struct timeval tv;
    JsonNode *arch = Json_create();
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    Json_add_double( arch, "valeur",    valeur );
    Json_add_int   ( arch, "date_sec",  tv.tv_sec );
    Json_add_int   ( arch, "date_usec", tv.tv_usec );
    Agent_send_mqtt_api_message ( Agent, arch, FALSE, "DLS_ARCHIVE/%s/%s", tech_id, acronyme );
    Json_unref( arch );
  }

/******************************************************************************************************************************/
/* Dls_run_archivage: Gere l'archivage des bits internes le necessitant                                                       */
/* Entrée : le plugin a traiter                                                                                               */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_run_archivage ( struct DLS_PLUGIN *plugin )
  { if (!plugin) return;
    if (!plugin->enable) return;                                                        /* On archive pas les plugins disable */

    GSList *liste = plugin->Dls_data_AI;
    while ( liste )
     { struct DLS_AI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, (bit->in_range ? bit->valeur : 0.0) );            /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_DI;
    while ( liste )
     { struct DLS_DI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->etat*1.0 );                                  /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_DO;
    while ( liste )
     { struct DLS_DO *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->etat*1.0 );                                  /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_CH;
    while ( liste )
     { struct DLS_CH *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur*1.0 );                                /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_REGISTRE;
    while ( liste )
     { struct DLS_REGISTRE *bit = liste->data;
       if ( (bit->archivage && (bit->last_arch + bit->archivage <= Agent->Top))       /* Archivage demandé & il est temps ? */
          || bit->last_arch == 0)                                                                                 /* a L'init */
        { Dls_Send_archive_to_API( bit->tech_id, bit->acronyme, bit->valeur );                                    /* Archivage si besoin */
          bit->last_arch = Agent->Top;
        }
       liste = g_slist_next ( liste );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

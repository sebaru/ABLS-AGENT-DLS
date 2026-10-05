/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/src/messages.c        Fonctions utilitaires pour la gestion des messages                                    */
/* Projet Abls-Habitat                   Gestion d'habitat                                                05.10.2026 00:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * messages.c
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

 #include <stdio.h>
 #include <string.h>
 #include <sys/time.h>

 #include "dls.h"

/******************************************************************************************************************************/
/* Convert_libelle_dynamique: Convertit les references dynamiques d'un libelle                                                */
/* Entrée : le libelle source                                                                                                 */
/* Sortie: le nouveau libelle adapté, à libérer ensuite                                                                      */
/******************************************************************************************************************************/
 gchar *Convert_libelle_dynamique ( gchar *libelle_src )
  { gchar prefixe[128], tech_id[32], acronyme[64], suffixe[128], libelle[256], chaine[32];
    gint taille_result = 256;
    if (!libelle_src) return(NULL);
    gchar *result = g_try_malloc0 ( taille_result );
    if (!result)
     { Info( __func__, "dls", NULL, LOG_ERR, "Memory error for '%s'", libelle_src );
       return(NULL);
     }
    g_snprintf ( result, taille_result, "%s", libelle_src );
    g_snprintf ( libelle, sizeof(libelle), "%s", libelle_src );

    for(;;)
     { memset ( prefixe,  0, sizeof(prefixe)  );
       memset ( suffixe,  0, sizeof(suffixe)  );
       memset ( tech_id,  0, sizeof(tech_id)  );
       memset ( acronyme, 0, sizeof(acronyme) );

       sscanf ( libelle, "%127[^$]$%31[^:]:%63[a-zA-Z0-9_]%127[^\n]", prefixe, tech_id, acronyme, suffixe );
       if (prefixe[0] == '\0')
        { sscanf ( libelle, "$%31[^:]:%63[a-zA-Z0-9_]%127[^\n]", tech_id, acronyme, suffixe ); }

       if (tech_id[0] == '\0' || acronyme[0] == '\0') break;

       struct DLS_REGISTRE *reg;
       struct DLS_AI *ai;
       if ( (ai = Dls_data_AI_lookup ( tech_id, acronyme )) != NULL )
        { g_snprintf( chaine, sizeof(chaine), "%.02f %s", ai->valeur, ai->unite ); }
       else if ( (reg = Dls_data_REGISTRE_lookup ( tech_id, acronyme )) != NULL )
        { g_snprintf( chaine, sizeof(chaine), "%.02f %s", reg->valeur, reg->unite ); }
       else
        { g_snprintf( chaine, sizeof(chaine), "bit inconnu" ); }

       g_snprintf ( result, taille_result, "%s%s%s", prefixe, chaine, suffixe );
       g_snprintf ( libelle, sizeof(libelle), "%s", result );
     }
    return(result);
  }
/******************************************************************************************************************************/
/* Get_datetime_usec: Formate la date courante avec les centiemes de seconde                                                  */
/* Entrée: le buffer de sortie et sa taille                                                                                   */
/* Sortie: le buffer                                                                                                          */
/******************************************************************************************************************************/
 gchar *Get_datetime_usec ( gchar *buffer, gint taille_buffer )
  { gchar chaine[256];
    struct timeval tv;
    struct tm local;
    gettimeofday( &tv, NULL );
    localtime_r( (time_t *)&tv.tv_sec, &local );
    strftime( chaine, sizeof(chaine), "%F %T", &local );
    gchar *date_utf8 = g_locale_to_utf8( chaine, -1, NULL, NULL, NULL );
    g_snprintf( buffer, taille_buffer, "%s.%02d", date_utf8, (gint)tv.tv_usec/10000 );
    g_free(date_utf8);
    return(buffer);
  }
/******************************************************************************************************************************/
/* Convert_msg_off_to_histo: Conversion d'un message off en historique                                                        */
/* Entrée: pointeur vers le message DLS_MESSAGE                                                                               */
/* Sortie: pointeur vers le JsonNode de l'historique                                                                          */
/******************************************************************************************************************************/
 JsonNode *Convert_msg_off_to_histo ( struct DLS_MESSAGE *msg )
  { JsonNode *histo = Json_create();
    if (histo)
     { Json_add_string( histo, "tech_id",  msg->tech_id );
       Json_add_string( histo, "acronyme", msg->acronyme );
       Json_add_bool( histo, "alive", FALSE );
       gchar date_fin[256];
       Get_datetime_usec ( date_fin, sizeof(date_fin) );
       Json_add_string( histo, "date_fin", date_fin );
     }
    return( histo );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

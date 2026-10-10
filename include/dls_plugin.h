/******************************************************************************************************************************/
/* ABLS-AGENT-DLS/include/dls_plugin.h  Interface entre l'agent DLS et les plugins D.L.S compilés                             */
/* Projet Abls-Habitat                   Gestion d'habitat                                                03.10.2026 15:00:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * dls_plugin.h
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

#ifndef _ABLS_AGENT_DLS_PLUGIN_H_
 #define _ABLS_AGENT_DLS_PLUGIN_H_

 #include <time.h>
 #include <glib.h>
 #include <json-glib/json-glib.h>

 #define DLS_NBR_CARAC_TECHID     32
 #define DLS_NBR_CARAC_ACRONYME   64
 #define DLS_NBR_CARAC_UNITE      32
 #define DLS_NBR_CARAC_LIBELLE    128
 #define DLS_NBR_CARAC_FORME      32

 enum                                                                                  /* différent statut des temporisations */
  { DLS_TEMPO_NOT_COUNTING,                                                                 /* La tempo ne compte pas du tout */
    DLS_TEMPO_WAIT_FOR_DELAI_ON,                                       /* La tempo compte, en attendant le delai de mise à un */
    DLS_TEMPO_WAIT_FOR_MIN_ON,                                         /* Delai de MAU dépassé, en attente du creneau minimum */
    DLS_TEMPO_WAIT_FOR_MAX_ON,                                      /* Creneau minimum atteint, en attente du creneau maximum */
    DLS_TEMPO_WAIT_FOR_DELAI_OFF,                                /* Creneau max atteint, en attente du delai de remise a zero */
    DLS_TEMPO_WAIT_FOR_COND_OFF                                            /* Attend que la condition soit tombée avant reset */
  };

 struct DLS_TEMPO                                                                           /* Définition d'une temporisation */
  { gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gboolean init;                                   /* True si les données delai_on/off min_on/off ont bien été positionnées */
    guint status;                                                                               /* Statut de la temporisation */
    guint date_on;                                                              /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                                            /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
    guint delai_on;                                                     /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                                                  /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;                            /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;                            /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint random;                                         /* Est-ce une tempo random ? si oui, est la dynamique max du random */
  };

 struct DLS_AI
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gchar   unite[DLS_NBR_CARAC_UNITE];                                                                                        /* Km, h, ° ... */
    gdouble valeur;
    guint   in_range;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
   };

 struct DLS_AO
  { gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gchar   unite[DLS_NBR_CARAC_UNITE];                                                                           /* Km, h, ° ... */
    gdouble valeur;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_WATCHDOG
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    guint   top;
  };

 struct DLS_HORLOGE
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
  };

 struct DLS_MONO
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_BI
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DI
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_DO
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gboolean mono;
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
    guint   archivage;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DLS_CI
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gint    valeur;
    gchar   unite[32];
    gboolean etat;
    gint    archivage;
    guint   last_arch;
  };

 struct DLS_CH
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    guint   valeur;
    gint    archivage;
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
    guint old_top;                                                                         /* Date de debut du comptage du CH */
    gboolean etat;
  };

 struct DLS_VISUEL
  { gchar    forme[DLS_NBR_CARAC_FORME];
    gchar    tech_id[DLS_NBR_CARAC_TECHID];
    gchar    acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   *libelle;
    gchar   *mode;
    gchar   *color;
    gchar   *badge;
    gdouble  valeur;
    gchar    unite[32];
    gboolean cligno;
    gboolean noshow;
    gboolean disable;
    gboolean changed;
    guint    next_send;
  };

 struct DLS_MESSAGE
  { JsonNode *source_node;
    gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle_converted[DLS_NBR_CARAC_LIBELLE];                                                      /* Le libelle converti selon les "$" */
    gboolean etat;                                                                          /* Etat avant execution du plugin */
    gboolean new_etat;                                                                      /* Etat après execution du plugin */
    gint new_etat_by_line;                                                    /* Numéro de ligne du dernier changement d'état */
    guint last_on;                                                                       /* Date du dernier changement d'état */
    gboolean libelle_is_dynamic;                                                       /* TRUE si le libelle dispose d'un "$" */
    guint next_top_check_libelle;                       /* Date a laquelle réaliser le prochain controle du libelle dynamique */
  };

 struct DLS_REGISTRE
  { gchar   tech_id[DLS_NBR_CARAC_TECHID];
    gchar   acronyme[DLS_NBR_CARAC_ACRONYME];
    gchar   libelle[DLS_NBR_CARAC_LIBELLE];                                                                                     /* Km, h, ° ... */
    gdouble valeur;
    gchar   unite[32];
    gint    archivage;
    guint   last_arch;                                                   /* Date de dernier enregistrement en base de données */
    gdouble pid_somme_erreurs;                                                                                /* Calcul PID KI*/
    gdouble pid_prev_erreur;                                                                                 /* Calcul PID KD */
  };

 struct DLS_PLUGIN
  { gchar name[128];
    gchar shortname[ DLS_NBR_CARAC_ACRONYME ];
    gchar tech_id[DLS_NBR_CARAC_TECHID];
    gchar package[130];
    guint syn_id;
    guint dls_id;
    gboolean enable;

    GSList *Dls_data_BI;
    GSList *Dls_data_MONO;
    GSList *Dls_data_DI;
    GSList *Dls_data_DO;
    GSList *Dls_data_AI;
    GSList *Dls_data_AO;
    GSList *Dls_data_MSG;
    GSList *Dls_data_CI;
    GSList *Dls_data_CH;
    GSList *Dls_data_TEMPO;
    GSList *Dls_data_VISUEL;
    GSList *Dls_data_REGISTRE;
    GSList *Dls_data_WATCHDOG;
    GSList *Dls_data_MESSAGE;
    GSList *Dls_data_HORLOGE;
    GSList *Agent_tech_ids;                                            /* Liste des Agents en input ou output du plugin D.L.S */

    time_t start_date;
    void *handle;
    void (*go)(struct DLS_PLUGIN *);
    gdouble conso;
    gchar *(*version)(void);
    void (*remap_all_alias)(struct DLS_PLUGIN *);
    void (*init)(struct DLS_PLUGIN *);
    GSList *Arbre_Comm;

    gboolean restart;                                   /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    gboolean debug;                                                 /* TRUE si le plugin doit logguer ses changements de bits */
    guint    monitor_until_top;                     /* Date d'extinction automatique du monitoring si l'API ne le relance pas */
    GHashTable *monitor_changed;          /* Bits modifiés depuis le dernier flush: clé=pointeur du bit, valeur=classe du bit */
    gint     num_ligne;                                                         /* N° de ligne du plugin en cours d'execution */
    struct DLS_MONO *dls_comm;
    gboolean dls_comm_old;
    struct DLS_MONO *dls_memsa_ok;
    gboolean dls_memsa_ok_old;
    struct DLS_MONO *dls_memsa_defaut;
    gboolean dls_memsa_defaut_old;
    struct DLS_MONO *dls_memsa_defaut_fixe;
    gboolean dls_memsa_defaut_fixe_old;
    struct DLS_MONO *dls_memsa_alarme;
    gboolean dls_memsa_alarme_old;
    struct DLS_MONO *dls_memsa_alarme_fixe;
    gboolean dls_memsa_alarme_fixe_old;
    struct DLS_MONO *dls_memssb_veille;
    gboolean dls_memssb_veille_old;
    struct DLS_MONO *dls_memssb_alerte;
    gboolean dls_memssb_alerte_old;
    struct DLS_MONO *dls_memssb_alerte_fixe;
    gboolean dls_memssb_alerte_fixe_old;
    struct DLS_MONO *dls_memssp_ok;
    gboolean dls_memssp_ok_old;
    struct DLS_MONO *dls_memssp_derangement;
    gboolean dls_memssp_derangement_old;
    struct DLS_MONO *dls_memssp_derangement_fixe;
    gboolean dls_memssp_derangement_fixe_old;
    struct DLS_MONO *dls_memssp_danger;
    gboolean dls_memssp_danger_old;
    struct DLS_MONO *dls_memssp_danger_fixe;
    gboolean dls_memssp_danger_fixe_old;
    struct DLS_DI   *dls_osyn_acquit;
    struct DLS_MESSAGE *dls_msg_comm_ok;
    struct DLS_MESSAGE *dls_msg_comm_hs;
  };

 extern struct DLS_BI *Dls_data_BI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_BI_get        ( struct DLS_BI *bit );
 extern gboolean Dls_data_BI_get_up     ( struct DLS_BI *bit );
 extern gboolean Dls_data_BI_get_down   ( struct DLS_BI *bit );
 extern void     Dls_data_BI_set        ( struct DLS_PLUGIN *plugin, struct DLS_BI *bit, gboolean valeur );

 extern struct DLS_MONO *Dls_data_MONO_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_MONO_get      ( struct DLS_MONO *bit );
 extern gboolean Dls_data_MONO_get_up   ( struct DLS_MONO *bit );
 extern gboolean Dls_data_MONO_get_down ( struct DLS_MONO *bit );
 extern void     Dls_data_MONO_set      ( struct DLS_PLUGIN *plugin, struct DLS_MONO *bit, gboolean valeur );

 extern struct DLS_DI *Dls_data_DI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_DI_get        ( struct DLS_DI *bit );
 extern gboolean Dls_data_DI_get_up     ( struct DLS_DI *bit );
 extern gboolean Dls_data_DI_get_down   ( struct DLS_DI *bit );
 extern void Dls_data_DI_set_pulse ( struct DLS_PLUGIN *plugin, struct DLS_DI *bit );

 extern struct DLS_DO *Dls_data_DO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_DO_set        ( struct DLS_PLUGIN *plugin, struct DLS_DO *bit, gboolean valeur );
 extern gboolean Dls_data_DO_get        ( struct DLS_DO *bit );
 extern gboolean Dls_data_DO_get_up     ( struct DLS_DO *bit );
 extern gboolean Dls_data_DO_get_down   ( struct DLS_DO *bit );

 extern struct DLS_AO *Dls_data_AO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_AO_set        ( struct DLS_PLUGIN *plugin, struct DLS_AO *bi, gdouble valeur );
 extern gdouble  Dls_data_AO_get        ( struct DLS_AO *bit );

 extern struct DLS_WATCHDOG *Dls_data_WATCHDOG_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_WATCHDOG_get ( struct DLS_WATCHDOG *bit );
 extern gint     Dls_data_WATCHDOG_get_time ( struct DLS_WATCHDOG *bit );
 extern void     Dls_data_WATCHDOG_set ( struct DLS_PLUGIN *plugin, struct DLS_WATCHDOG *bit, gint consigne );

 extern void Dls_data_set_bus ( struct DLS_PLUGIN *plugin, gchar *agent_tech_id, gchar *commande );

 extern struct DLS_AI *Dls_data_AI_lookup ( gchar *tech_id, gchar *acronyme );
 extern gdouble  Dls_data_AI_get        ( struct DLS_AI *bit );
 extern gboolean Dls_data_AI_get_inrange ( struct DLS_AI *bit );

 extern struct DLS_CI *Dls_data_CI_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_CI_set ( struct DLS_PLUGIN *plugin, struct DLS_CI *bit, gboolean etat );
 extern void Dls_data_CI_set_pulse ( struct DLS_PLUGIN *plugin, struct DLS_CI *bit );
 extern gint Dls_data_CI_get ( struct DLS_CI *bit );
 extern void Dls_data_CI_reset ( struct DLS_PLUGIN *plugin, struct DLS_CI *bit );

 extern struct DLS_CH *Dls_data_CH_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_CH_set ( struct DLS_PLUGIN *plugin, struct DLS_CH *bit, gboolean etat );
 extern gint Dls_data_CH_get ( struct DLS_CH *cpt_h );
 extern void Dls_data_CH_reset ( struct DLS_PLUGIN *plugin, struct DLS_CH *bit );

 extern struct DLS_REGISTRE *Dls_data_REGISTRE_lookup ( gchar *tech_id, gchar *acronyme );
 extern void    Dls_data_REGISTRE_set ( struct DLS_PLUGIN *plugin, struct DLS_REGISTRE *reg, gdouble valeur );
 extern gdouble Dls_data_REGISTRE_get ( struct DLS_REGISTRE *reg );

 extern struct DLS_VISUEL *Dls_data_VISUEL_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_VISUEL_set ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu,
                                   gdouble valeur, gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_badge ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, gchar *badge );
 extern void Dls_data_VISUEL_set_mode ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, gchar *mode );
 extern void Dls_data_VISUEL_set_color ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, gchar *color );
 extern void Dls_data_VISUEL_set_libelle ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, gchar *libelle );
 extern void Dls_data_VISUEL_set_for_WATCHDOG ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_WATCHDOG *src,
                                                gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_REGISTRE ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_REGISTRE *src,
                                                gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_TEMPO ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_TEMPO *src,
                                             gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_CI ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_CI *src,
                                          gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_CH ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_CH *src,
                                          gboolean cligno, gboolean noshow, gboolean disable );
 extern void Dls_data_VISUEL_set_for_AI ( struct DLS_PLUGIN *plugin, struct DLS_VISUEL *visu, struct DLS_AI *src,
                                          gboolean cligno, gboolean noshowe, gboolean disable );
 extern struct DLS_HORLOGE *Dls_data_HORLOGE_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_HORLOGE_get ( struct DLS_HORLOGE *bit );

 extern struct DLS_MESSAGE *Dls_data_MESSAGE_lookup ( gchar *tech_id, gchar *acronyme );
 extern void Dls_data_MESSAGE_set ( struct DLS_PLUGIN *plugin, struct DLS_MESSAGE *msg );

 extern struct DLS_TEMPO *Dls_data_TEMPO_lookup ( gchar *tech_id, gchar *acronyme );
 extern void     Dls_data_TEMPO_set     ( struct DLS_PLUGIN *plugin, struct DLS_TEMPO *bit, gboolean etat,
                                          gint delai_on, gint min_on, gint max_on, gint delai_off, gint random);
 extern gboolean Dls_data_TEMPO_get     ( struct DLS_TEMPO *bit );
 extern gint     Dls_data_TEMPO_get_time ( struct DLS_TEMPO *bit );

 extern void Dls_PID_reset ( struct DLS_PLUGIN *plugin, struct DLS_REGISTRE *r_input );
 extern void Dls_PID ( struct DLS_PLUGIN *plugin, struct DLS_REGISTRE *input, struct DLS_REGISTRE *consigne,
                       struct DLS_REGISTRE *kp,struct DLS_REGISTRE *ki, struct DLS_REGISTRE *kd,
                       struct DLS_REGISTRE *outputmin, struct DLS_REGISTRE *outputmax, struct DLS_REGISTRE *output
                     );

 extern gint Dls_get_top( void );                                                                             /* donne le top */

 extern gboolean Dls_Jour_semaine ( int jour );
 extern gboolean Dls_Heure ( int heure, int minute );
 extern gboolean Dls_Heure_apres ( int heure, int minute );
 extern gboolean Dls_Heure_avant ( int heure, int minute );
 extern gboolean Dls_Heure_apres_egal ( int heure, int minute );
 extern gboolean Dls_Heure_avant_egal ( int heure, int minute );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/

//
// Created by flassabe on 26/10/22.
//

#include "reducers.h"

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "global_defs.h"
#include "utility.h"

/*!
 * @brief add_source_to_list adds an e-mail to the sources list. If the e-mail already exists, do not add it.
 * @param list the list to update
 * @param source_email the e-mail to add as a string
 * @return a pointer to the updated beginning of the list
 */
sender_t *add_source_to_list(sender_t *list, char *source_email) {
    sender_t* element= (sender_t*) malloc(sizeof(sender_t)); // création d'un élément sender
    strcpy(element->recipient_address,source_email); // attribution du sourcemail à l'element  strcpy(la structure, source);
    sender_t* searcher = find_source_in_list(list,source_email) ;
    if(list->recipient_address == NULL){
        return element;}
    else if (searcher == NULL){
        element->next = list;
        list->prev = element; // on place l'element en tete de liste;
        return element;
    }
    else{
        return list;
    }

}

/*!
 * @brief clear_sources_list clears the list of e-mail sources (therefore clearing the recipients of each source)
 * @param list a pointer to the list to clear
 */
void clear_sources_list(sender_t *list) {
    sender_t* p = list;
    while(p->next != NULL){
        recipient_t* q = p->head;
        while(q->next != NULL){
            q = q->next;
            free(q->prev);
        }
        free(q);
        p=p->next;
        free(p->prev);
    }
}

/*!
 * @brief find_source_in_list looks for an e-mail address in the sources list and returns a pointer to it.
 * @param list the list to look into for the e-mail
 * @param source_email the e-mail as a string to look for
 * @return a pointer to the matching source, NULL if none exists
 */
sender_t *find_source_in_list(sender_t *list, char *source_email) {
// partie de code pour régler le probleme de strmcp
    char *adr_retour_ligne;
    adr_retour_ligne = strpbrk(source_email, "\n");/* Recherche de l'adresse d'un \n dans la variable chaine */
    if (adr_retour_ligne != NULL)/* Adresse trouvée ? */
    {
        *adr_retour_ligne = 0; /* Remplacement du caractère par un octet nul (fin de chaîne en C) */
    }
    sender_t *q = list;
    while(q->next != NULL && (strcmp(q->recipient_address,source_email) != 0) ){ // tant que on a pas fait toute la liste et qu'on a pas trouvé de correspondance entre source mail et un sender de la chaine
        q = q->next;
    }
    int a = strcmp(q->recipient_address,source_email);
    if(a == 0 ){
        return q;
    }else{
        return NULL;
        }
}

/*!
 * @brief find_source_in_list_of_recipients looks for an e-mail address in the recipient list and returns a pointer to it.
 * @param list the list to look into for the e-mail
 * @param source_email the e-mail as a string to look for
 * @return a pointer to the matching source, NULL if none exists
 */
recipient_t *find_source_in_list_of_recipients(recipient_t *list, char *source_email) {
// partie de code pour régler le probleme de strmcp
    char *adr_retour_l;
    adr_retour_l = strpbrk(source_email, "\n");/* Recherche de l'adresse d'un \n dans la variable chaine */
    if(adr_retour_l != NULL)/* Adresse trouvée ? */
    {
        *adr_retour_l = 0; /* Remplacement du caractère par un octet nul (fin de chaîne en C) */
    }
    recipient_t *q = list;
    while(q->next != NULL && (strcmp(q->recipient_address,source_email) != 0) ){ // tant que on a pas fait toute la liste et qu'on a pas trouvé de correspondance entre source mail et un sender de la chaine
        q = q->next;
    }
    if(strcmp(q->recipient_address,source_email) == 0 ){
        return q;
    }else{
        return NULL;}
}


/*!
 * @brief add_recipient_to_source adds or updates a recipient in the recipients list of a source. It looks for
 * the e-mail in the recipients list: if it is found, its occurrences is incremented, else a new recipient is created
 * with its occurrences = to 1.
 * @param source a pointer to the source to add/update the recipient to
 * @param recipient_email the recipient e-mail to add/update as a string
 */
void add_recipient_to_source(sender_t *source, char *recipient_email) {
    recipient_t* q = source->head;
    if (q != NULL){
        q=find_source_in_list_of_recipients(q,recipient_email);
        if(q == NULL){
            recipient_t* elementR= (recipient_t*) malloc(sizeof(recipient_t)); // création d'un élément receiver
            elementR->next = source->head;
            source->head->prev = elementR;
            source->head = elementR;
            elementR->occurrences = 1;
            strcpy(elementR->recipient_address,recipient_email);
        }else{
            q->occurrences +=1;
        }
    }else{
        recipient_t* elementR= (recipient_t*) malloc(sizeof(recipient_t)); // création d'un élément receiver
        elementR->occurrences = 1;
        strcpy(elementR->recipient_address,recipient_email);
        source->head = elementR;
    }
}

/*!
 * @brief push_chain, allows to send the one line on the output file, which corresponds to a sender and all its receivers
 * format : sender n:receiver ...with n the number of mail that the sender sent to the receiver. 
 * @param l pointer to a sender
 * @param f_out output file 
 */
 sender_t * push_chain(sender_t* l,FILE *f_out){

    sender_t* q = l;
    recipient_t* p = l->head;
    fprintf(f_out,"%s",q->recipient_address);

    while(p->next != NULL){
        fprintf(f_out," %d:%s",p->occurrences,p->recipient_address);
        p=p->next;
    }
    fprintf(f_out," %d:%s\n",p->occurrences,p->recipient_address);

    q=q->next;
    return q;
}

/*!
 * @brief files_list_reducer is the first reducer. It uses concatenates all temporary files from the first step into
 * a single file. Don't forget to sync filesystem before leaving the function.
 * @param data_source the data source directory (its directories have the same names as the temp files to concatenate)
 * @param temp_files the temporary files directory, where to read files to be concatenated
 * @param output_file path to the output file (default name is step1_output, but we'll keep it as a parameter).
 */
void files_list_reducer(char *data_source, char *temp_files, char *output_file) {
      DIR *dir = opendir(temp_files);
    FILE *output = fopen(output_file, "w+");
    struct dirent *entry ;
    char add[500];
    char buffer[500];
    int compteur=0;
    if(output){
        if(dir){
            entry = readdir(dir);
            while(entry != NULL){
                if(entry->d_type==DT_REG && strcmp(entry->d_name,"step1_output")){
                    strcpy(add, temp_files);
                    strcat(add, "/");
                    strcat(add, entry->d_name);
                    FILE *fichier = fopen(add, "r");
                    if (fichier){
                        while(fgets(buffer,499,fichier) != NULL){
                            fprintf(output, "%s" ,buffer);
                            compteur++;
                        }
                    }
                    fclose(fichier);
                    remove(add);
                }
                entry=readdir(dir);
            }
        }
        closedir(dir);
    }
    fclose(output);

    printf("nb ligne saisie : %d \n", compteur);
}

/*!
 * @brief files_reducer opens the second temporary output file (default step2_output) and collates all sender/recipient
 * information as defined in the project instructions. Stores data in a double level linked list (list of source e-mails
 * containing each a list of recipients with their occurrences).
 * @param temp_file path to temp output file
 * @param output_file final output file to be written by your function
 */
void files_reducer(char *temp_file, char *output_file) {
// Première partie qui consiste à mettre toute les informations dans la double liste chainé.

    FILE *f_in = NULL;
    f_in = fopen(temp_file,"r");
    sender_t* listS = (sender_t*) malloc(sizeof(sender_t)); // on initialise la liste chainée

    if(f_in != NULL){
        char buffer[STR_MAX_LEN]; // on initialise le buffer car on le vide à chaque itération
       while(fgets(buffer,STR_MAX_LEN,f_in) != NULL){
            char d[] = " ";
            char *p = strtok(buffer, d);  // on découpe la string et on fait pointé p sur  le premier mail
            listS = add_source_to_list(listS,p); // on ajoute le sender à la liste chainée
            sender_t* senderFound = find_source_in_list(listS,p); // retourne un pointeur vers l'élément de la liste sender qui contient le mail
            if(senderFound != NULL){
                p = strtok(NULL, d); // on recupère le premier receiver
                while(p != NULL)
                {
                    add_recipient_to_source(senderFound,p); // ajoute le receiver à la liste chainé relié au sender.
                    p = strtok(NULL, d); // mettre dans p le prochain receiver

                }
            }
        }
        fclose(f_in);

        printf("\npremier element: %s",listS->next->recipient_address);

        FILE* f_out = NULL;
        f_out = fopen(output_file,"w");
        if(f_out != NULL){
            sender_t* q = listS;
            while(q->next != NULL){
                q=push_chain(q,f_out);
            }
            clear_sources_list(listS);
           printf("\n--> data transferred to the output document.");

        }
        else {
            printf("invalid PATH to f_out\n");}
        fclose(f_out);
    }
    else {
        printf("invalid PATH to f_in\n");
        fclose(f_in);}
}


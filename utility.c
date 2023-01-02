//
// Created by flassabe on 26/10/22.
//

#include "utility.h"

#include <string.h>
#include <dirent.h>
#include <libgen.h>
#include <unistd.h>

#include "global_defs.h"

/*!
 * @brief cat_path concatenates two file system paths into a result. It adds the separation /  if required.
 * @param prefix first part of the complete path
 * @param suffix second part of the complete path
 * @param full_path resulting path
 * @return pointer to full_path if operation succeeded, NULL else
 */
char *concat_path(char *prefix, char *suffix, char *full_path) {
    strcpy(full_path,prefix);
    strcat(full_path, "/");
    strcat(full_path,suffix);
    return full_path;
}

/*!
 * @brief directory_exists tests if directory located at path exists
 * @param path the path whose existence to test
 * @return true if directory exists, false else
 */
bool directory_exists(char *path) {
    struct dirent *d;
    DIR *dir=opendir(path);
    if(dir){
        if((d=readdir(dir))!=NULL){
            return true;
        }
    }
    return false;
}

/*!
 * @brief path_to_file_exists tests if a path leading to a file exists. It separates the path to the file from the
 * file name itself. For instance, path_to_file_exists("/var/log/Xorg.0.log") will test if /var/log exists and is a
 * directory.
 * @param path the path to the file
 * @return true if path to file exists, false else
 */
bool path_to_file_exists(char *path) {
    char *file_name=strtok(strrchr(path,'/'),"/");
    int tail=strlen(path)-strlen(file_name);
    char dir_path[tail];
    for(int i=tail-1;i>-1;i--){
        dir_path[i]=path[i];
    }
    dir_path[strlen(dir_path)-6]='\0';    
    if(directory_exists(dir_path)){
        struct dirent *d;
        DIR *dir=opendir(dir_path);
        while((d=readdir(dir))!=NULL){
            if(strstr(d->d_name,file_name)!=NULL){
                return true;
            }
        }
    }
    return false;
}

/*!
 * @brief sync_temporary_files waits for filesystem syncing for a path // on attend que tout soit reellement ecrit 
 * @param temp_dir the path to the directory to wait for
 * Use fsync and dirfd
 */
void sync_temporary_files(char *temp_dir) {
       fsync(dirfd(temp_dir));
}


/*!
 * @brief next_dir returns the next directory entry that is not . or ..
 * @param entry a pointer to the current struct dirent in caller
 * @param dir a pointer to the already opened directory
 * @return a pointer to the next not . or .. directory, NULL if none remain
 */
struct dirent *next_dir(struct dirent *entry, DIR *dir) {
        if (!dir){
        return NULL;
    }

	entry = readdir(dir);

	if (!entry){
        return NULL;
    }

    while (entry) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_type != DT_DIR){
            entry = readdir(dir);
        }
        else {break;}
    }

	return entry;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>   
#include <io.h>   
#include <windows.h> 
#include <shlobj.h>   
#include <errno.h>    

// Function declarations
void move();
void compress();
void name();
void execute_command(const char *cmd);
void delete_non_LRF_files_safe();

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <command>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--help") == 0) {
        printf("This program provides the following functions to organize and process LRF files:\n");
        printf("    -m: Create a directory named 'LRF' in the current directory and move all .LRF files into it.\n");
        printf("    -t: Rename all LRF files in the 'LRF' directory to have an .MP4 extension.\n");
        printf("    -c: Compress all MP4 videos in the 'LRF' directory using the H265 codec. After compression, delete the original videos that do not contain '_LRF' in their names.\n");
    } else if (strcmp(argv[1], "-m") == 0) {
        printf("Moving LRF files...\n");
        move();
    } else if (strcmp(argv[1], "-c") == 0) {
        printf("Compressing files in the LRF directory using H265...\n");
        compress();
    } else if (strcmp(argv[1], "-t") == 0) {
        printf("Changing file extensions of all LRF files to MP4...\n");
        name();
    } else {
        printf("Unknown command: %s\n", argv[1]);
    }

    return 0;
}

void execute_command(const char *cmd) {
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    // Create a modifiable copy of cmd
    char *cmd_copy = _strdup(cmd);
    if (cmd_copy == NULL) {
        printf("Unable to allocate memory for the command.\n");
        return;
    }

    if (CreateProcess(NULL, cmd_copy, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        printf("Unable to execute command: %s\n", cmd);
    }
    free(cmd_copy);
}

void move() {
    if (_mkdir("LRF") == -1 && errno != EEXIST) {
        printf("Failed to create directory 'LRF'.\n");
        return;
    }

    // Find .LRF and .lrf files and move them
    struct _finddata_t file_info;
    intptr_t handle = _findfirst("*.LRF", &file_info);
    if (handle == -1) {
        handle = _findfirst("*.lrf", &file_info);
        if (handle == -1) {
            printf("No .LRF files found.\n");
            return;
        }
    }

    do {
        const char *ext = strrchr(file_info.name, '.');
        if (ext && (_stricmp(ext, ".LRF") == 0)) {
            char src_path[MAX_PATH];
            char dest_path[MAX_PATH];
            snprintf(src_path, sizeof(src_path), "%s", file_info.name);
            snprintf(dest_path, sizeof(dest_path), "LRF\\%s", file_info.name);
            if (rename(src_path, dest_path) == 0) {
                printf("Moved file: %s -> %s\n", src_path, dest_path);
            } else {
                printf("Failed to move file: %s\n", src_path);
            }
        }
    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
}

void name() {
    struct _finddata_t file_info;
    intptr_t handle = _findfirst("LRF\\*", &file_info);
    if (handle == -1) {
        printf("No files found in the 'LRF' directory.\n");
        return;
    }

    do {
        char old_path[MAX_PATH];
        char new_path[MAX_PATH];

        const char *ext = strrchr(file_info.name, '.');
        if (!ext || _stricmp(ext, ".mp4") != 0) {
            // Remove existing extension (if any)
            char file_name_no_ext[MAX_PATH];
            if (ext) {
                size_t len = ext - file_info.name;
                strncpy(file_name_no_ext, file_info.name, len);
                file_name_no_ext[len] = '\0';
            } else {
                strcpy(file_name_no_ext, file_info.name);
            }

            snprintf(old_path, sizeof(old_path), "LRF\\%s", file_info.name);
            snprintf(new_path, sizeof(new_path), "LRF\\%s.mp4", file_name_no_ext);

            if (rename(old_path, new_path) == 0) {
                printf("Renamed file: %s -> %s\n", old_path, new_path);
            } else {
                printf("Failed to rename file: %s\n", old_path);
            }
        }
    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
}

void compress() {
    struct _finddata_t file_info;
    intptr_t handle = _findfirst("LRF\\*.mp4", &file_info);
    if (handle == -1) {
        printf("No .mp4 files found in the 'LRF' directory.\n");
        return;
    }

    do {
        char input_file[MAX_PATH];
        char output_file[MAX_PATH];
        char ffmpeg_cmd[1024];

        snprintf(input_file, sizeof(input_file), "LRF\\%s", file_info.name);

        if (strstr(file_info.name, "_LRF") != NULL) {
            printf("File '%s' already processed, skipping.\n", file_info.name);
            continue;
        }

        // Remove extension
        char file_name_no_ext[MAX_PATH];
        strcpy(file_name_no_ext, file_info.name);
        char *dot = strrchr(file_name_no_ext, '.');
        if (dot != NULL) {
            *dot = '\0';
        }

        snprintf(output_file, sizeof(output_file), "LRF\\%s_LRF.mp4", file_name_no_ext);

        snprintf(ffmpeg_cmd, sizeof(ffmpeg_cmd),
            "ffmpeg -i \"%s\" -c:v libx265 -b:v 3M -threads 4 -c:a copy \"%s\"", input_file, output_file);

        printf("Compressing: %s -> %s\n", input_file, output_file);
        execute_command(ffmpeg_cmd);

    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
    delete_non_LRF_files_safe();
}

void delete_non_LRF_files_safe() {
    struct _finddata_t file_info;
    intptr_t handle = _findfirst("LRF\\*", &file_info);
    if (handle == -1) {
        printf("No files found in the 'LRF' directory.\n");
        return;
    }

    do {
        if (strstr(file_info.name, "_LRF") == NULL) {
            char file_path[MAX_PATH + 2] = {0}; // +2 for double null terminator
            snprintf(file_path, sizeof(file_path) - 2, "LRF\\%s", file_info.name);

            // Ensure double null terminator
            size_t len = strlen(file_path);
            file_path[len + 1] = '\0'; // Second null character

            SHFILEOPSTRUCT file_op = {0};
            file_op.wFunc = FO_DELETE;
            file_op.pFrom = file_path;
            file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION;

            if (SHFileOperation(&file_op) == 0) {
                printf("Moved to recycle bin: %s\n", file_path);
            } else {
                printf("Failed to move to recycle bin: %s\n", file_path);
            }
        }
    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
}
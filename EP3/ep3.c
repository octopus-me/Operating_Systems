#include "ep3.h"

void load_directory(FILE *fs_file,Directory *dir){
    fread(&dir->file_count,sizeof(int),1,fs_file);
    for(int i=0;i<dir->file_count;i++){
        fread(&dir->files[i],sizeof(File),1,fs_file);
        if(dir->files[i].is_directory){
            dir->files[i].directory = (Directory *)malloc(sizeof(Directory));
            load_directory(fs_file,dir->files[i].directory);
        }
    }
}

void load_filesystem(FileSystem *fs, FILE *fs_file){
    fseek(fs_file,0,SEEK_SET);
    fread(fs->fat,sizeof(fs->fat),1,fs_file);
    fread(fs->bitmap,sizeof(fs->bitmap),1,fs_file);
    load_directory(fs_file,&fs->root);
}

void save_directory(FILE *fs_file, Directory *dir){
    fwrite(&dir->file_count,sizeof(int),1,fs_file);
    for(int i=0;i<dir->file_count;i++){
        fwrite(&dir->files[i],sizeof(File),1,fs_file);
        if(dir->files[i].is_directory){
            save_directory(fs_file,dir->files[i].directory);
        }
    }
}

void save_filesystem(FileSystem *fs, FILE *fs_file){
    fseek(fs_file,0,SEEK_SET);
    fwrite(fs->fat,sizeof(fs->fat),1,fs_file);
    fwrite(fs->bitmap,sizeof(fs->bitmap),1,fs_file);
    save_directory(fs_file,&fs->root);
}

void init_fs(FileSystem *fs){
    memset(fs->fat,-1,sizeof(fs->fat));
    memset(fs->bitmap,0,sizeof(fs->bitmap));
    memset(&fs->root,0,sizeof(fs->root));

    for(int i=0;i < FILESYSTEM_METADATA_BLOCKS;i++){
        fs->bitmap[i] = 1;
        fs->fat[i] = -1; //Metadados não sao encadeados
    }
}

void print_directory_tree(Directory *dir, int level) {
    for (int i = 0; i < level; i++) {
        if (i == level - 1) {
            printf("|-- ");
        } else {
            printf("|   ");
        }
    }
    printf("%s/\n", dir->name);

    for (int i = 0; i < dir->file_count; i++) {
        for (int j = 0; j < level + 1; j++) {
            if (j == level) {
                printf("|-- ");
            } else {
                printf("|   ");
            }
        }
        if (dir->files[i].is_directory) {
            printf("%s/\n", dir->files[i].name);
            print_directory_tree(dir->files[i].directory, level + 1);
        } else {
            printf("%s\n", dir->files[i].name);
        }
    }
}

FILE * mount_fs(FileSystem *fs, const char *fs_path){
    FILE *fs_file = fopen(fs_path, "r+b");

    if (fs_file) {
        load_filesystem(fs,fs_file);
        printf("Sistema de arquivos montado de %s: \n", fs_path);
        print_directory_tree(&fs->root,0);
    } else {
        fs_file = fopen(fs_path, "w+b");
        if(!fs_file) {
            printf("Erro ao criar o arquivo do sistema de arquivos. \n");
            return NULL;
        }
        init_fs(fs);
        save_filesystem(fs,fs_file);
        printf("Novo sistema de arquivos criado %s. \n", fs_path);
    }
    return fs_file;
}

// O arquivo do sistema de arquivos simulado ("fs_file") é passado como parâmetro.
// fseek(fs_file, sizeof(FileSystem) + block_index * BLOCK_SIZE, SEEK_SET);
// "fseek" reposiciona o ponteiro do arquivo para o início do bloco que desejamos escrever.
// O ponteiro do arquivo é movido para a posição sizeof(FileSystem)+ block_index * BLOCK_SIZE;
// Isso é nescessário porque os primeiros sizeof(FileSystem) bytes do arquivo são reservados para armazenar
// as estruturas de dados do sistema de arquivos (FAT, Bitmap, e diretórios);
// Após esses bytes, os blocos de dados reais começam;

// fwrite(data, sizeof(char), BLOCK_SIZE, fs_file);
// fwrite escreve BLOCK_SIZE bytes do buffer data no arquivo a partir da posição atual do ponteiro do arquivo.

void write_block(FILE *fs_file, int block_index, const char *data){
    // Posiciono o ponteiro do file no local exato onde desejo fazer a escrita
    fseek(fs_file, block_index * BLOCK_SIZE, SEEK_SET);
    // Escrevo exatamente 1 bloco naquele local
    fwrite(data, sizeof(unsigned char),BLOCK_SIZE, fs_file);
}

void read_block(FILE *fs_file, int block_index, char *data){
    // Posiciono com a fseek o local onde está a informação que desejo ler;
    fseek(fs_file, block_index*BLOCK_SIZE,SEEK_SET);
    // Leio exatamente 1 bloco e salvo na variável data;
    fread(data, sizeof(unsigned char), BLOCK_SIZE, fs_file); 
}

int find_free_block(FileSystem *fs){
    for(int i=0;i<TOTAL_BLOCKS;i++){
        if(fs->bitmap[i]==0){
            return i;
        }
    }
    return -1;
}

void copy_file(FileSystem *fs, const char *src_path, const char *dest_path, FILE *fs_file){
    // Abri o arquivo de origem
    FILE *src_file = fopen(src_path, "rb");
    if(src_file==NULL){
        printf("Erro ao abrir o arquivo de origem. \n");
        return;
    }

    // Extrair o nome do arquivo e o caminho do diretório de destino
    char parent_path[MAX_PATH_LEN];
    char file_name[MAX_FILENAME_LEN];

    strcpy(parent_path, dest_path);
    char *last_slash = strrchr(parent_path,'/');
    if(last_slash != NULL){
        strcpy(file_name, last_slash+1);
        *last_slash = '\0';
    } else {
        strcpy(parent_path,"/");
        strcpy(file_name, dest_path);
    }

    //Navegar até o diretório de destino
    Directory *dest_dir = navigate_to_directory(&fs->root, parent_path);
    if(dest_dir == NULL){
        printf("Diretorio de destino nao encontrado. \n");
        fclose(src_file);
        return;
    }

    // Verificar se já existe um arquivo com o mesmo nome
    for(int i=0;i<dest_dir->file_count;i++){
        if(strcmp(dest_dir->files[i].name, file_name)==0){
            printf("Um arquivo com o mesmo nome já existe. \n");
            fclose(src_file);
            return;
        }
    }

    // Ler o arquivo de origem e copiar para o sistema de arquivos simulado
    File new_file;
    strcpy(new_file.name, file_name);
    time(&new_file.creation_time);
    new_file.modification_time = new_file.creation_time;
    new_file.access_time = new_file.creation_time;
    new_file.is_directory = 0;
    new_file.directory = NULL;

    // Façamos agora:
    // Copiar os bytes do arquivo original no sistema de arquivos
    // Atualizar o FAT e o Bitmap
    // Definir o ponto inicial do arquivo

    int prev_block = -1;
    int first_block = -1;
    char buffer[BLOCK_SIZE];

    // Computo o tamanho o arquivo em bytes
    fseek(src_file,0,SEEK_END);
    int size = ftell(src_file);
    fseek(src_file,0,SEEK_SET);

    new_file.size = size;

    if(size > MAX_FILE_SIZE){
        printf("Rquivo muito grande para ser copiado. \n");
        fclose(src_file);
        return;
    }

    while(size > 0) {
        int bytes_to_write = (size < BLOCK_SIZE) ? size : BLOCK_SIZE;
        fread(buffer, sizeof(unsigned char), bytes_to_write, src_file);

        // ATENÇÂO: O que fazer quando tem menos bytes que o tamanho do bloco?
        // Inicialmente irei preencher tudo com zeros
        if(bytes_to_write < BLOCK_SIZE) {
            memset(buffer+bytes_to_write,0,BLOCK_SIZE-bytes_to_write);
        }
        int free_block = find_free_block(fs);

        if (free_block == -1) {
            printf("Erro: Espaço insuficiente no sistema de arquivos. \n");
            fclose(src_file);
            return;
        }

        fs->bitmap[free_block] = 1;
        if(prev_block != -1){
            fs->fat[prev_block] = free_block;
        } else {
            first_block = free_block;
        }
        prev_block = free_block;

        write_block(fs_file,free_block,buffer);

        size -= bytes_to_write;

    }

    if (prev_block != -1){
        fs->fat[prev_block] = -1;
    }

    new_file.start_block = first_block;

    //Adicionar o novo arquivo ao diretório de destino
    dest_dir->files[dest_dir->file_count++] = new_file;

    fclose(src_file);

}

// FUNÇÕES PARA OS DIRETÓRIOS
// Antes de criar um diretório, precisamos navegar pela estrutura de diretórios para encontrar o local correto e adicionar o novo diretório lá.
Directory* navigate_to_directory(Directory *root, const char *path){
    char *token;
    char path_copy[MAX_PATH_LEN];
    strcpy(path_copy,path);

    Directory *current_dir = root;
    token = strtok(path_copy,"/");

    while(token != NULL){
        int found = 0;
        for(int i=0;i < current_dir->file_count;i++){
            if(current_dir->files[i].is_directory && strcmp(current_dir->files[i].name,token)==0){
                current_dir = current_dir->files[i].directory;
                found = 1;
                break;
            }
        }
        if (!found) return NULL;
        token = strtok(NULL,"/");
    }
    return current_dir;
}

// FUNÇÃO PARA CRIAÇÃO DE DIRETÓRIO
void create_dir(FileSystem *fs, const char *path) {
    char temp_path[MAX_PATH_LEN];
    Directory *current_dir = &fs->root;

    // Copiar o caminho para a string temporária para manipulação
    strcpy(temp_path, path);

    // Dividir o caminho por '/'
    char *token = strtok(temp_path, "/");

    while (token != NULL) {
        int found = 0;

        // Verificar se o subdiretório já existe
        for (int i = 0; i < current_dir->file_count; i++) {
            if (strcmp(current_dir->files[i].name, token) == 0 && current_dir->files[i].is_directory) {
                current_dir = current_dir->files[i].directory;
                found = 1;
                break;
            }
        }

        // Se o subdiretório não existe, criar um novo
        if (!found) {
            Directory *new_dir = malloc(sizeof(Directory));
            strcpy(new_dir->name, token);
            new_dir->file_count = 0;

            File new_dir_file;
            strcpy(new_dir_file.name, token);
            new_dir_file.size = 0;
            time(&new_dir_file.creation_time);
            new_dir_file.modification_time = new_dir_file.creation_time;
            new_dir_file.access_time = new_dir_file.creation_time;
            new_dir_file.start_block = -1; // Diretórios não têm bloco inicial
            new_dir_file.is_directory = 1;
            new_dir_file.directory = new_dir;

            current_dir->files[current_dir->file_count++] = new_dir_file;
            current_dir = new_dir;
        }

        // Próximo token
        token = strtok(NULL, "/");
    }
}



void unmount_fs(FileSystem *fs, FILE *fs_file) {
    // Fechar o arquivo de sistema de arquivos
    fclose(fs_file);
    
    // Não há necessidade de liberar `files` dentro de `Directory` pois é estático
    // A função ainda vai percorrer os diretórios para garantir que estamos liberando corretamente

    void free_directory(Directory *dir) {
        for (int i = 0; i < dir->file_count; i++) {
            if (dir->files[i].is_directory) {
                free_directory(dir->files[i].directory);
                free(dir->files[i].directory);  // Liberar o ponteiro do diretório
            }
        }
    }

    free_directory(&fs->root);
}




void list_directory(const char *path, FileSystem *fs) {
    Directory *dir = navigate_to_directory(&fs->root, path);
    if (dir == NULL) {
        printf("Diretório não encontrado.\n");
        return;
    }

    // Print header
    printf("%-20s | %-10s | %-25s | %-25s | %-25s |\n", "Nome", "Tamanho", "Criado em", "Modificado em", "Acessado em");
    printf("----------------------------------------------------------------------------------------------------------------------\n");

    // Print files and directories
    for (int i = 0; i < dir->file_count; i++) {
        File *file = &dir->files[i];
        // Remove newline character from ctime output
        char creation_time[25], modification_time[25], access_time[25];
        strncpy(creation_time, ctime(&file->creation_time), 24);
        creation_time[24] = '\0';
        strncpy(modification_time, ctime(&file->modification_time), 24);
        modification_time[24] = '\0';
        strncpy(access_time, ctime(&file->access_time), 24);
        access_time[24] = '\0';

        printf("%-20s | %-10d | %-25s | %-25s | %-25s%s\n",
               file->name, file->size, creation_time, modification_time, access_time,
               file->is_directory ? "| Directory |" : "| File      |");
    }
}



void delete_file(FileSystem *fs, File *file){
    int current_block = file->start_block;
    while(current_block != -1) {
        int next_block = fs->fat[current_block];
        fs->bitmap[current_block] = 0;
        fs->fat[current_block] = -1;
        current_block = next_block;
    }
}

void delete_directory(FileSystem *fs, Directory *dir){
    for (int i=0;i<dir->file_count;i++){
        if(dir->files[i].is_directory){
            delete_directory(fs,dir->files[i].directory);
        } else {
            delete_file(fs,&dir->files[i]);
        }
        printf("Deletado: %s \n", dir->files[i].name);
    }
    free(dir);
}

void delete_dir(FileSystem *fs, const char *path){
    char parente_path[MAX_PATH_LEN];
    char dir_name[MAX_FILENAME_LEN];


    strcpy(parente_path,path);
    char *last_slash = strrchr(parente_path,'/');
    if(last_slash != NULL){
        strcpy(dir_name,last_slash+1);
        *last_slash = '\0';
    } else {
        strcpy(parente_path,"/");
        strcpy(dir_name,path);
    }

    Directory *parente_dir = navigate_to_directory(&fs->root,parente_path);
    if(parente_path == NULL){
        printf("Diretorio pai não encontrado. \n");
        return;
    }

    for(int i=0;i <parente_dir->file_count;i++){
        if(strcmp(parente_dir->files[i].name,dir_name)==0 && parente_dir->files[i].is_directory){
            delete_directory(fs,parente_dir->files[i].directory);
            for(int j=i;j<parente_dir->file_count-1;j++){
                parente_dir->files[j] = parente_dir->files[j+1];
            }
            parente_dir->file_count--;
            return;
        }
    }
    printf("Diretório não encontrado. \n");
}

void delete_file_by_name(FileSystem *fs, const char *path){
    char parente_path[MAX_PATH_LEN];
    char file_name[MAX_FILENAME_LEN];

    strcpy(parente_path, path);
    char *last_slash = strrchr(parente_path,'/');
    if (last_slash != NULL){
        strcpy(file_name, last_slash+1);
        *last_slash = '\0';
    } else {
        strcpy(parente_path,"/");
        strcpy(file_name,path);
    }

    Directory *parent_dir = navigate_to_directory(&fs->root, parente_path);
    if (parent_dir == NULL){
        printf("Diretório pai não encontrado. \n");
        return;
    }

    // Encontrar o arquivo no diretório pai
    for(int i=0; i < parent_dir->file_count;i++){
        if(strcmp(parent_dir->files[i].name,file_name)==0 && !parent_dir->files[i].is_directory){
            // Deletar o arquivo e liberar os blocos
            delete_file(fs,&parent_dir->files[i]);
            // Deletar a entrada do arquivo nos metadados do diretório
            for(int j = i;j<parent_dir->file_count-1;j++){
                parent_dir->files[j] = parent_dir->files[j+1];
            }
            parent_dir->file_count--;
            printf("Arquivo deletado %s\n", file_name);
            return;
        }
    }
    printf("Arquivo não encontrado. \n");
}

void toca_arquivo(FileSystem *fs, const char *path) {
    // Extrair o caminho do diretório pai e o nome do arquivo
    char parent_path[MAX_PATH_LEN];
    char file_name[MAX_FILENAME_LEN];

    strcpy(parent_path, path);
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash != NULL) {
        strcpy(file_name, last_slash + 1);
        *last_slash = '\0';
    } else {
        strcpy(parent_path, "/");
        strcpy(file_name, path);
    }

    // Navegar até o diretório pai
    Directory *parent_dir = navigate_to_directory(&fs->root, parent_path);
    if (parent_dir == NULL) {
        printf("Diretório pai não encontrado.\n");
        return;
    }

    // Verificar se o arquivo já existe
    for (int i = 0; i < parent_dir->file_count; i++) {
        if (strcmp(parent_dir->files[i].name, file_name) == 0 && !parent_dir->files[i].is_directory) {
            // Atualizar o tempo de acesso
            time(&parent_dir->files[i].access_time);
            printf("Arquivo %s acessado. Tempo de acesso atualizado.\n", file_name);
            return;
        }
    }

    // Se o arquivo não existir, criar um novo arquivo vazio
    File new_file;
    strcpy(new_file.name, file_name);
    new_file.size = 0;
    time(&new_file.creation_time);
    new_file.modification_time = new_file.creation_time;
    new_file.access_time = new_file.creation_time;
    new_file.start_block = -1;
    new_file.is_directory = 0;
    new_file.directory = NULL;

    parent_dir->files[parent_dir->file_count++] = new_file;
    printf("Arquivo %s criado.\n", file_name);
}

File *find_file(Directory *dir, const char *file_name){
    for(int i=0;i<dir->file_count;i++){
        if(strcmp(dir->files[i].name, file_name)==0 && !dir->files[i].is_directory){
            return &dir->files[i];
        }
    }
    return NULL;
}

void mostra_arquivo(FileSystem *fs,  const char *file_path, FILE *fs_file){
    char parent_path[MAX_PATH_LEN];
    char file_name[MAX_FILENAME_LEN];

    strcpy(parent_path, file_path);
    char *last_slash = strrchr(parent_path, '/');
    if(last_slash != NULL){
        strcpy(file_name, last_slash+1);
        *last_slash ='\0';
    } else {
        strcpy(parent_path, "/");
        strcpy(file_name, file_path);
    }

    Directory *parent_dir = navigate_to_directory(&fs->root, parent_path);
    if(parent_dir == NULL){
        printf("Diretorio pai não encontrado. \n");
        return;
    }

    File *file = find_file(parent_dir, file_name);
    if(file==NULL){
        printf("Arquivo não encontrado. \n");
        return;
    }

    // Ler e exibir o conteúdo do arquivo
    int current_block = file->start_block;
    char buffer[BLOCK_SIZE+1];
    buffer[BLOCK_SIZE] = '\0';

    while(current_block != -1) {
        //ATENCAO
        read_block(fs_file,current_block,buffer);
        printf("%s", buffer);
        current_block = fs->fat[current_block];
    }

    time(&file->access_time);

}

void init_db(Database *db) {
    db->count = 0;
}

void fill_database(Database *db, Directory *dir, char *path) {
    // Adicionar o diretório atual ao banco de dados
    if (db->count < MAX_FILES) {
        strcpy(db->paths[db->count], path);
        strcat(db->paths[db->count], "/");
        db->count++;
    }

    // Navegar pelos arquivos e subdiretórios
    for (int i = 0; i < dir->file_count; i++) {
        File *file = &dir->files[i];
        char new_path[MAX_PATH_LEN];
        strcpy(new_path, path);
        strcat(new_path, "/");
        strcat(new_path, file->name);

        if (file->is_directory) {
            fill_database(db, file->directory, new_path);
        } else {
            if (db->count < MAX_FILES) {
                strcpy(db->paths[db->count], new_path);
                db->count++;
            }
        }
    }
}

void atualizadb(FileSystem *fs, Database *db) {
    init_db(db);
    fill_database(db, &fs->root, "");
}


void print_database(Database *db) {
    printf("Árvore de Diretórios e Arquivos:\n");
    for (int i = 0; i < db->count; i++) {
        int level = 0;
        char *path = db->paths[i];

        // Contar o nível de profundidade baseado nos '/' no caminho
        for (char *p = path; *p; p++) {
            if (*p == '/') {
                level++;
            }
        }

        // Imprimir com indentação baseada no nível
        for (int j = 0; j < level; j++) {
            printf("    ");
        }
        printf("|-- %s\n", path);
    }
}

void busca(Database *db, const char *search_string) {
    printf("Resultados da busca para '%s':\n", search_string);
    int found = 0;
    for (int i = 0; i < db->count; i++) {
        if (strstr(db->paths[i], search_string) != NULL) {
            printf("%s\n", db->paths[i]);
            found = 1;
        }
    }
    if (!found) {
        printf("Nenhum arquivo ou diretório encontrado contendo '%s' no nome.\n", search_string);
    }
}

void status(FileSystem *fs) {
    int total_directories = 0;
    int total_files = 0;
    int free_space = 0;
    int wasted_space = 0;

    // Calcular espaço livre
    for (int i = 0; i < MAX_FILE_SIZE / BLOCK_SIZE; i++) {
        if (fs->bitmap[i] == 0) {
            free_space += BLOCK_SIZE;
        }
    }

    // Função auxiliar para percorrer o sistema de arquivos
    void traverse_directory(Directory *dir) {
        total_directories++;
        for (int i = 0; i < dir->file_count; i++) {
            if (dir->files[i].is_directory) {
                traverse_directory(dir->files[i].directory);
            } else {
                total_files++;
                int file_blocks = (dir->files[i].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
                int actual_size = file_blocks * BLOCK_SIZE;
                wasted_space += (actual_size - dir->files[i].size);
            }
        }
    }

    // Percorrer a estrutura de diretórios começando do root
    traverse_directory(&fs->root);

    // Imprimir informações
    printf("Status do Sistema de Arquivos:\n");
    printf("Quantidade de diretórios: %d\n", total_directories);
    printf("Quantidade de arquivos: %d\n", total_files);
    printf("Espaço livre: %d bytes\n", free_space);
    printf("Espaço desperdiçado: %d bytes\n", wasted_space);
}


void prompt() {
    FileSystem fs;
    int mounted = 0;

    Database db;

    char comando[500];

    char *fs_path = NULL; //Vou querer esse variável para guardar o caminho do f

    FILE *fs_file = NULL;

    while (1) {
        printf("\n{ep3}: ");
        fgets(comando, sizeof(comando), stdin);

        // Remover nova linha do comando
        comando[strcspn(comando, "\n")] = 0;

        // Tokenizar o comando
        char *cmd = strtok(comando, " ");

        if (cmd == NULL) {
            continue;
        }

        if (strcmp(cmd, "monta") == 0) {
            fs_path = strtok(NULL, " ");

            printf("%s \n", fs_path);
            if (fs_path == NULL) {
                printf("Erro: Caminho do arquivo não fornecido.\n");
                continue;
            }
            fs_file = mount_fs(&fs, fs_path);
            mounted = 1;

        } else if (!mounted) {
            printf("Erro: Nenhum sistema de arquivos montado.\n");
            continue;

        } else if (strcmp(cmd, "copia") == 0) {
            char *origem = strtok(NULL, " ");
            char *destino = strtok(NULL, " ");
            if (origem == NULL || destino == NULL) {
                printf("Erro: Caminhos de origem e destino não fornecidos.\n");
                continue;
            }
            copy_file(&fs, origem, destino, fs_file);

        } else if (strcmp(cmd, "criadir") == 0) {
            char *diretorio = strtok(NULL, " ");
            if (diretorio == NULL) {
                printf("Erro: Caminho do diretório não fornecido.\n");
                continue;
            }
            create_dir(&fs,diretorio);

        } else if (strcmp(cmd, "apagadir") == 0) {
            char *diretorio = strtok(NULL, " ");
            if (diretorio == NULL) {
                printf("Erro: Caminho do diretório não fornecido.\n");
                continue;
            }
            delete_dir(&fs, diretorio);

        } else if (strcmp(cmd, "mostra") == 0) {
            // NAO HA NADA AINDA
            char *arquivo = strtok(NULL, " ");
            if (arquivo == NULL) {
                printf("Erro: Caminho do arquivo não fornecido.\n");
                continue;
            }
            mostra_arquivo(&fs,arquivo,fs_file);

        } else if (strcmp(cmd, "apaga") == 0) {
            char *arquivo = strtok(NULL, " ");
            if (arquivo == NULL) {
                printf("Erro: Caminho do arquivo não fornecido.\n");
                continue;
            }
            delete_file_by_name(&fs, arquivo);

        } else if (strcmp(cmd, "lista") == 0) {
            char *diretorio = strtok(NULL, " ");
            if (diretorio == NULL) {
                printf("Erro: Caminho do diretório não fornecido.\n");
                continue;
            }
            list_directory(diretorio,&fs);

        }  else if (strcmp(cmd, "toca") == 0) {
            char *arquivo = strtok(NULL, " ");
            if (arquivo == NULL) {
                printf("Erro: Caminho do arquivo não fornecido.\n");
                continue;
            }
            printf("%s \n", arquivo);
            toca_arquivo(&fs, arquivo);

        } else if(strcmp(cmd, "atualizadb")==0){
            atualizadb(&fs,&db);
            print_database(&db);

        } else if(strcmp(cmd, "desmonta") ==0){

            if (mounted){
                save_filesystem(&fs, fs_file);
                unmount_fs(&fs,fs_file);
                mounted = 0;
                printf("Sistema de arquivos desmontado. \n");
            } else {
                printf("Erro: Nenhum sistema de arquivos montado. \n");
            }
        } else if (strcmp(cmd, "busca") == 0) {
            char *search_string = strtok(NULL, " ");
            if (search_string == NULL) {
                printf("Erro: String de busca não fornecida.\n");
                continue;
            }
            busca(&db, search_string);

        } else if (strcmp(cmd, "status") == 0) {
            status(&fs);

        } else if(strcmp(cmd, "sai") == 0){

            if (mounted){
                save_filesystem(&fs, fs_file);
                unmount_fs(&fs,fs_file);
                mounted = 0;
                printf("Saindo do simulador. \n");
            }

            return;

        } else {
            printf("Comando desconhecido.\n");
        } 

        if (mounted) {
            save_filesystem(&fs, fs_file);
        }        
    }
}

int main() {
    prompt();
    return 0;
}

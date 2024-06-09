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

void mount_fs(FileSystem *fs, const char *fs_path){
    FILE *fs_file = fopen(fs_path, "r+b");

    if (fs_file) {
        load_filesystem(fs,fs_file);
        fclose(fs_file);
        printf("Sistema de arquivos montado de %s: \n", fs_path);
        print_directory_tree(&fs->root,0);
    } else {
        fs_file = fopen(fs_path, "w+b");
        if(!fs_file) {
            printf("Erro ao criar o arquivo do sistema de arquivos. \n");
            exit(EXIT_FAILURE);
        }
        init_fs(fs);
        save_filesystem(fs,fs_file);
        fclose(fs_file);
        printf("Novo sistema de arquivos criado %s. \n", fs_path);

    }

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

void copy_file(FileSystem *fs, const char *src_path, const char *dest_path){
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
    FILE *fs_file = fopen("arquivo", "r+b"); //Abro o sistema de arquivo simulado
    
    if (fs_file == NULL) {
        printf("Erro ao abrir o sistema de arquivos simulado.\n");
        fclose(src_file);
        return;
    }

    // Computo o tamanho o arquivo em bytes
    fseek(src_file,0,SEEK_END);
    int size = ftell(src_file);
    fseek(src_file,0,SEEK_SET);

    new_file.size = size;

    if(size > MAX_FILE_SIZE){
        printf("Rquivo muito grande para ser copiado. \n");
        fclose(src_file);
        fclose(fs_file);
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
    fclose(fs_file);

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
void create_dir(FileSystem *fs, const char *path){
    char parent_path[MAX_PATH_LEN];
    char dir_name[MAX_FILENAME_LEN];

    // Extrair o nome do diretório e o caminho do diretório pai
    strcpy(parent_path,path);
    char *last_slash = strrchr(parent_path,'/');

    if(last_slash != NULL){
        strcpy(dir_name,last_slash+1);
        *last_slash = '\0';
    } else {
        strcpy(parent_path,"/");
        strcpy(dir_name,path);
    }

    // Navegar até o diretorio pai
    Directory *parent_dir = navigate_to_directory(&fs->root,parent_path);
    if(parent_dir == NULL){
        printf("Diretorio não encontrado. \n");
        // ATENÇÃO: Em tese, ele precisa criar os diretórios caso não existam;
        return;
    }

    // Verifica se já existe um arquivo no diretório com o mesmo nome
    for(int i=0;i<parent_dir->file_count;i++){
        if(strcmp(parent_dir->files[i].name,dir_name)==0){
            printf("Um arquivo ou diretorio com o mesmo nome já existe \n");
            return;
        }
    }

    //Cria o novo diretorio

    Directory *new_dir = malloc(sizeof(Directory));
    strcpy(new_dir->name,dir_name);
    new_dir->file_count = 0;
    new_dir->parent = parent_dir;

    File new_dir_file;
    strcpy(new_dir_file.name,dir_name);
    new_dir_file.size = 0;
    time(&new_dir_file.creation_time);
    new_dir_file.modification_time = new_dir_file.creation_time;
    new_dir_file.access_time = new_dir_file.creation_time;
    new_dir_file.start_block = -1; // Diretorios nao tem bloco inicial
    new_dir_file.is_directory = 1;
    new_dir_file.directory = new_dir;

    parent_dir->files[parent_dir->file_count++] = new_dir_file;
}


void unmount_fs(const char *file_path, FileSystem *fs) {
    FILE *file = fopen(file_path, "wb");
    fwrite(fs, sizeof(FileSystem), 1, file);
    fclose(file);
}



void list_directory(const char *path, FileSystem *fs) {
	printf("LISTAR \n");
    Directory *dir = navigate_to_directory(&fs->root, path);
    if (dir == NULL) {
        printf("Diretório não encontrado.\n");
        return;
    }

    for (int i = 0; i < dir->file_count; i++) {
        File *file = &dir->files[i];
        printf("Nome: %s, Tamanho: %d, Criado em: %s, Modificado em: %s, Acessado em: %s%s\n",
               file->name, file->size, ctime(&file->creation_time), ctime(&file->modification_time), ctime(&file->access_time),
               file->is_directory ? "/" : "");
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

// int main() {
//     FileSystem fs;
//     mount_fs(&fs, "arquivo");

//     create_dir(&fs, "/novo_dir");
//     copy_file(&fs, "source.txt", "/novo_dir/copiadest.txt");

//     delete_dir(&fs, "/novo_dir");

//     FILE *fs_file = fopen("arquivo", "r+b");
//     save_filesystem(&fs, fs_file);
//     fclose(fs_file);

//     return 0;
// }


// int main() {
//     FileSystem fs;

//     // Montar o sistema de arquivos
//     mount_fs(&fs, "arquivo");

//     // Criar um diretório
//     create_dir(&fs, "/novo_dir");

//     //list_directory("/",&fs);
    

//     // Copiar um arquivo
//     copy_file(&fs, "source.txt", "/novo_dir/copiadest.txt");
//     copy_file(&fs, "source.txt", "/novo_dir/novacopiadest.txt");

//     create_dir(&fs,"/novo_dir/novo_dir2");
//     create_dir(&fs,"/segundodir");

//     copy_file(&fs, "source.txt","/segundodir/segundacopia.txt");

//     create_dir(&fs,"/novo_dir/novo_dir2/wallace");
//     create_dir(&fs,"/novo_dir/novo_dir2/wallace/beatriz");

//     copy_file(&fs, "source2.txt","/novo_dir/novo_dir2/wallace/beatriz/bia.txt");
//     //list_directory("/novo_dir",&fs);

//     delete_dir(&fs,"/novo_dir/novo_dir2");

//     copy_file(&fs,"TESTE/e.txt","/novo.txt");

//     // Salvar o sistema de arquivos
//     FILE *fs_file = fopen("arquivo", "r+b");
//     save_filesystem(&fs, fs_file);
//     fclose(fs_file);

//     printf("CHEGOU AQUI \n");

//     // Reabrir o sistema de arquivos
//     mount_fs(&fs, "arquivo");

//     list_directory("/",&fs);

//     char comando[500];
//     while(1){
//         printf("{ep3}: ");
//         scanf("%s",comando);

//         if(strcmp (strtok(comando," "),"monta")==0) {
//             printf("%s", strtok(comando, "o"));
//             printf("SIM \n");
//         }


//     }


//     // // Verificar se a cópia do arquivo persistiu
//     // Directory *dir = navigate_to_directory(&fs.root, "/novo_dir");
//     // if (dir != NULL) {
//     //     for (int i = 0; i < dir->file_count; i++) {
//     //         printf("Arquivo no diretório: %s\n", dir->files[i].name);
//     //     }
//     // }

//     // // Deletar o diretório
//     // delete_dir(&fs, "/novo_dir");

//     // //list_directory("/",&fs);

//     // // Salvar novamente após deleção
//     // fs_file = fopen("filesystem.dat", "r+b");
//     // save_filesystem(&fs, fs_file);
//     // fclose(fs_file);

//     // // Reabrir novamente o sistema de arquivos
//     // mount_fs(&fs, "filesystem.dat");

//     //list_directory("/",&fs);

//     return 0;
// }




void prompt() {
    FileSystem fs;
    int mounted = 0;

    char comando[500];
    while (1) {
        printf("{ep3}: ");
        fgets(comando, sizeof(comando), stdin);

        // Remover nova linha do comando
        comando[strcspn(comando, "\n")] = 0;

        // Tokenizar o comando
        char *cmd = strtok(comando, " ");
        if (cmd == NULL) {
            continue;
        }

        if (strcmp(cmd, "monta") == 0) {
            char *arquivo = strtok(NULL, " ");
            if (arquivo == NULL) {
                printf("Erro: Caminho do arquivo não fornecido.\n");
                continue;
            }
            mount_fs(&fs, arquivo);
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
            copy_file(&fs, origem, destino);

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
            //NAO HA NADA AINDA
            // char *arquivo = strtok(NULL, " ");
            // if (arquivo == NULL) {
            //     printf("Erro: Caminho do arquivo não fornecido.\n");
            //     continue;
            // }
            // show_file(&fs, arquivo);

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

        } else {
            printf("Comando desconhecido.\n");
        }

        FILE *fs_file = fopen("arquivo", "r+b");
        save_filesystem(&fs, fs_file);
        fclose(fs_file);
    }
}

int main() {
    prompt();
    return 0;
}
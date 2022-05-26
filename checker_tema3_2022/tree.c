#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "utils.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."


FileTree createFileTree(char* rootFolderName) {
    // TODO
    FileTree *tree = malloc(sizeof(FileTree));
    DIE(NULL == tree, "Malloc failed for tree\n");
    tree->root = malloc(sizeof(TreeNode));
    DIE(NULL == tree->root, "Malloc failed for tree->root\n");

    tree->root->name = malloc(strlen(rootFolderName) + 1);
	DIE(NULL == tree->root->name, "Malloc() failed for tree->root->name\n");
    memcpy(tree->root->name, rootFolderName, strlen(rootFolderName) + 1);

    tree->root->parent = NULL;
    tree->root->type = FOLDER_NODE;
    FolderContent content;
    content.children = malloc(sizeof(List));
    content.children->head = NULL;
    memcpy(tree->root->content, &content, sizeof(content));
    return *tree;
}

void freeTree(FileTree fileTree) {
    // TODO
    
}


void ls(TreeNode* currentNode, char* arg) {
    // TODO
}


void pwd(TreeNode* treeNode) {
    // TODO
    
}


TreeNode* cd(TreeNode* currentNode, char* path) {
    // TODO
}


void tree(TreeNode* currentNode, char* arg) {
    // TODO
}


void mkdir(TreeNode* currentNode, char* folderName) {
    // TODO
}


void rmrec(TreeNode* currentNode, char* resourceName) {
    // TODO
}


void rm(TreeNode* currentNode, char* fileName) {
    // TODO
}


void rmdir(TreeNode* currentNode, char* folderName) {
    // TODO
}


void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
    // TODO
}


void cp(TreeNode* currentNode, char* source, char* destination) {
    // TODO
}

void mv(TreeNode* currentNode, char* source, char* destination) {
    // TODO
}


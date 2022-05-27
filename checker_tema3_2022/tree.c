#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "utils.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."
#define NO_PRINT 0


FileTree *createFileTree(char* rootFolderName) {
	FileTree *tree = malloc(sizeof(FileTree));
	DIE(NULL == tree, "Malloc failed for tree\n");
	tree->root = malloc(sizeof(TreeNode));
	DIE(NULL == tree->root, "Malloc failed for tree->root\n");

	tree->root->name = malloc(strlen(rootFolderName) + 1);
	DIE(NULL == tree->root->name, "Malloc() failed for tree->root->name\n");
	memcpy(tree->root->name, rootFolderName, strlen(rootFolderName) + 1);

	tree->root->parent = NULL;
	tree->root->type = FOLDER_NODE;

	tree->root->content = malloc(sizeof(FolderContent));
	FolderContent content;
	content.children = malloc(sizeof(List));
	content.children->head = NULL;
	memcpy(tree->root->content, &content, sizeof(content));

	free(rootFolderName);
	return tree;
}

void ls(TreeNode* currentNode, char* arg) {
	if (!currentNode || currentNode->type == FILE_NODE)
		return;

	FolderContent content = *(FolderContent *)(currentNode->content);
	ListNode *it = content.children->head;

	if (!strcmp(arg, NO_ARG)) {
		while (it) {
			printf("%s\n", it->info->name);
			it = it->next;
		}
		return;
	}

	while (it) {
		if (!strcmp(it->info->name, arg))
			break;
		it = it->next;
	}

	if (!it) {
		printf("ls: cannot access '%s': No such file or directory\n", arg);
		return;
	}

	if (it->info->type == FILE_NODE) {
		printf("%s: %s\n", it->info->name,
			(*(FileContent *)(it->info->content)).text);
		return;
	}

	if (it->info->type == FOLDER_NODE) {
		FolderContent it_content = *(FolderContent *)(it->info->content);
		ListNode *curr = it_content.children->head;

		while (curr) {
			printf("%s\n", curr->info->name);
			curr = curr->next;
		}
		return;
	}
}


void pwd(TreeNode* treeNode) {
	if (!treeNode || treeNode->type == FILE_NODE)
		return;

	int path_size = 1;
	char **path = malloc(sizeof(char *));

	TreeNode *it = treeNode;
	while (it) {
		path[path_size - 1] = it->name;

		++path_size;
		char **new_path = realloc(path, path_size * sizeof(char *));
		path = new_path;

		it = it->parent;
	}

	for (int i = path_size - 2; i >= 1; --i)
		printf("%s/", path[i]);
	printf("%s\n", path[0]);

	free(path);
}


TreeNode* cd(TreeNode* currentNode, char* path, int print) {
	if (!currentNode || currentNode->type == FILE_NODE || !path)
		return currentNode;

	TreeNode *ogNode = currentNode;
	char *tok = strtok(path, "/\n");

	while (tok) {
		if (!strcmp(tok , PARENT_DIR)) {
			currentNode = currentNode->parent;
		} else {
			FolderContent content = *(FolderContent *)(currentNode->content);
			ListNode *it = content.children->head;

			while (it) {
				if (!strcmp(it->info->name, tok))
					break;
				it = it->next;
			}

			if (it) {
				currentNode = it->info;
			} else {
				if (print)
					printf("cd: no such file or directory: %s\n", path);
				return ogNode;
			}
		}

		tok = strtok(NULL, "/\n");
	}
	return currentNode;
}


void tree(TreeNode* currentNode, char* arg) {
	if (!currentNode || currentNode->type == FILE_NODE)
		return;

	static int files = 0, folders = 0, level = 0;

	TreeNode *workingNode = currentNode;
	if (strcmp(arg, NO_ARG)) {
		workingNode = cd(currentNode, arg, NO_PRINT);

		if (workingNode == currentNode || workingNode->type == FILE_NODE) {
			printf("%s [error opening dir]\n\n0 directories, 0 files\n", arg);
			return;
		}
	}

	FolderContent content = *(FolderContent *)(workingNode->content);
	ListNode *it = content.children->head;

	while (it) {
		for (int i = 0; i < level; ++i)
			printf("    ");
		printf("%s\n", it->info->name);

		if (it->info->type == FOLDER_NODE) {
			++level;
			tree(it->info, NO_ARG);
			--level;
			++folders;
		} else {
			++files;
		}

		it = it->next;
	}

	if (!level) {
		printf("%d directories, %d files\n", folders, files);
		files = folders = 0;
	}
}


void mkdir(TreeNode* currentNode, char* folderName) {
	if (!currentNode || currentNode->type == FILE_NODE || !folderName)
		return;

	FolderContent curr_content = *(FolderContent *)(currentNode->content);
	ListNode *it = curr_content.children->head;

	while (it) {
		if (!strcmp(it->info->name, folderName))
			break;
		it = it->next;
	}

	if (it) {
		printf("mkdir: cannot create directory '%s': File exists\n",
			folderName);

		return;
	}

	TreeNode *node = malloc(sizeof(TreeNode));

	node->name = malloc(strlen(folderName) + 1);
	DIE(NULL == node->name, "Malloc() failed for tree->root->name\n");
	memcpy(node->name, folderName, strlen(folderName) + 1);

	node->parent = currentNode;
	node->type = FOLDER_NODE;

	node->content = malloc(sizeof(FolderContent));
	FolderContent content;
	content.children = malloc(sizeof(List));
	content.children->head = NULL;
	memcpy(node->content, &content, sizeof(content));

	ListNode *listNode = malloc(sizeof(ListNode));
	listNode->info = node;
	listNode->next = ((FolderContent *)currentNode->content)->children->head;
	((FolderContent *)currentNode->content)->children->head = listNode;

	free(folderName);
}


void rmrec(TreeNode* currentNode, char* resourceName) {
	if (!currentNode || currentNode->type == FILE_NODE || !resourceName)
		return;

	FolderContent curr_content = *(FolderContent *)(currentNode->content);
	ListNode *it = curr_content.children->head;

	while (it) {
		if (!strcmp(it->info->name, resourceName))
			break;
		it = it->next;
	}

	if (!it) {
		printf("rmrec: failed to remove '%s': No such file or directory",
			resourceName);

		return;
	}

	if (it->info->type == FILE_NODE) {
		rm(currentNode, resourceName);

		return;
	} else {
		FolderContent it_content = *(FolderContent *)(it->info->content);
		ListNode *i = it_content.children->head, *next;

		while (i) {
			next = i->next;

			if (i->info->type == FILE_NODE) {
				rm(it->info, i->info->name);
			} else {
				if (((FolderContent *)i->info->content)->children->head)
					rmrec(it->info, i->info->name);
				else
					rmdir(it->info, i->info->name);
			}

			i = next;
		}

		rmdir(currentNode, it->info->name);
	}
}


void rm(TreeNode* currentNode, char* fileName) {
	if (!currentNode || currentNode->type == FILE_NODE || !fileName)
		return;

	FolderContent curr_content = *(FolderContent *)(currentNode->content);
	ListNode *it = curr_content.children->head, *prev = NULL;

	while (it) {
		if (!strcmp(it->info->name, fileName))
			break;
		prev = it;
		it = it->next;
	}

	if (!it) {
		printf("rm: failed to remove '%s': No such file or directory",
			fileName);
		return;
	}

	if (it->info->type == FOLDER_NODE) {
		printf("rm: cannot remove '%s': Is a directory", fileName);
		return;
	}

	if (prev)
		prev->next = it->next;
	else
		curr_content.children->head = it->next;

	free(it->info->name);
	free(((FileContent *)it->info->content)->text);
	free(it->info->content);
	free(it->info);
	free(it);
}


void rmdir(TreeNode* currentNode, char* folderName) {
	if (!currentNode || currentNode->type == FILE_NODE || !folderName)
		return;

	FolderContent curr_content = *(FolderContent *)(currentNode->content);
	ListNode *it = curr_content.children->head, *prev = NULL;

	while (it) {
		if (!strcmp(it->info->name, folderName))
			break;
		prev = it;
		it = it->next;
	}

	if (!it) {
		printf("rmdir: failed to remove '%s': No such file or directory",
			folderName);
		return;
	}

	if (it->info->type == FILE_NODE) {
		printf("rmdir: failed to remove '%s': Not a directory", folderName);
		return;
	}

	if (((FolderContent *)it->info->content)->children->head) {
		printf("rmdir: failed to remove '%s': Directory not empty", folderName);
		return;
	}

	if (prev)
		prev->next = it->next;
	else
		curr_content.children->head = it->next;

	free(it->info->name);
	free(((FolderContent *)it->info->content)->children);
	free(it->info->content);
	free(it->info);
	free(it);
}


void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
	FolderContent curr_content = *(FolderContent *)(currentNode->content);
	ListNode *it = curr_content.children->head;

	while (it) {
		if (!strcmp(it->info->name, fileName))
			break;
		it = it->next;
	}

	if (it)
		return;

	TreeNode *node = malloc(sizeof(TreeNode));

	node->name = malloc(strlen(fileName) + 1);
	DIE(NULL == node->name, "Malloc() failed for tree->root->name\n");
	memcpy(node->name, fileName, strlen(fileName) + 1);

	node->parent = currentNode;
	node->type = FILE_NODE;

	node->content = malloc(sizeof(FileContent));
	FileContent content;
	content.text = malloc(strlen(fileContent) + 1);
	memcpy(content.text, fileContent, strlen(fileContent) + 1);
	memcpy(node->content, &content, sizeof(content));

	ListNode *listNode = malloc(sizeof(ListNode));
	listNode->info = node;
	listNode->next = ((FolderContent *)currentNode->content)->children->head;
	((FolderContent *)currentNode->content)->children->head = listNode;

	free(fileName);
	free(fileContent);
}


void cp(TreeNode* currentNode, char* source, char* destination) {
	char *dest_copy = malloc(strlen(destination) + 1);
	memcpy(dest_copy, destination, strlen(destination) + 1);
	TreeNode* src = cd(currentNode, source, NO_PRINT);
	TreeNode* dest = cd(currentNode, dest_copy, NO_PRINT);
	free(dest_copy);

	if (src->type == FOLDER_NODE) {
		printf("cp: -r not specified; omitting directory '%s'\n", source);
		return;
	}

	if (dest != currentNode) {
		if (dest->type == FOLDER_NODE) {
			touch(dest, strdup(src->name),
				strdup(((FileContent *)src->content)->text));

			return;
		} else if (dest->type == FILE_NODE) {
			free(((FileContent *)dest->content)->text);
			((FileContent *)dest->content)->text =
				malloc(strlen(((FileContent *)src->content)->text) + 1);

			memcpy(((FileContent *)dest->content)->text,
				((FileContent *)src->content)->text,
					strlen(((FileContent *)src->content)->text) + 1);

			return;
		}
	} else {
		char *lastSlash = strrchr(destination, '/');

		if (lastSlash) {
			char *newFileName = malloc(strlen(lastSlash + 1) + 1);

			memcpy(newFileName, lastSlash + 1, strlen(lastSlash + 1) + 1);

			char *newDestination = malloc(strlen(destination)
				- strlen(lastSlash) + 1);

			memcpy(newDestination, destination, strlen(destination)
				- strlen(lastSlash) + 1);

			newDestination[strlen(destination) - strlen(lastSlash)] = '\0';

			dest = cd(currentNode, newDestination, NO_PRINT);

			if (dest == currentNode) {
				printf("cp: failed to access '%s': Not a directory\n",
					destination);
			} else {
				touch(dest, strdup(newFileName),
					strdup(((FileContent *)src->content)->text));
			}
			free(newDestination);
			free(newFileName);
		}
	}
}

void mv(TreeNode* currentNode, char* source, char* destination) {
	char *dest_copy = malloc(strlen(destination) + 1);
	memcpy(dest_copy, destination, strlen(destination) + 1);
	TreeNode* src = cd(currentNode, source, NO_PRINT);
	TreeNode* dest = cd(currentNode, dest_copy, NO_PRINT);
	free(dest_copy);

	if (src->type == FOLDER_NODE && dest->type == FOLDER_NODE) {
		mkdir(dest, strdup(src->name));
		FolderContent curr_content = *(FolderContent *)(dest->content);
		ListNode *it = curr_content.children->head;

		((FolderContent *)it->info->content)->children->head =
			((FolderContent *)src->content)->children->head;

		((FolderContent *)src->content)->children->head = NULL;

		rmdir(src->parent, src->name);
		return;
	}

	if (dest != currentNode) {
		if (dest->type == FOLDER_NODE) {
			touch(dest, strdup(src->name),
				strdup(((FileContent *)src->content)->text));

			rm(src->parent, src->name);
			return;
		} else if (dest->type == FILE_NODE) {
			free(((FileContent *)dest->content)->text);
			((FileContent *)dest->content)->text =
				malloc(strlen(((FileContent *)src->content)->text) + 1);

			memcpy(((FileContent *)dest->content)->text,
				((FileContent *)src->content)->text,
					strlen(((FileContent *)src->content)->text) + 1);

			rm(src->parent, src->name);
			return;
		}
	} else {
		char *lastSlash = strrchr(destination, '/');

		if (lastSlash) {
			char *newFileName = malloc(strlen(lastSlash + 1) + 1);

			memcpy(newFileName, lastSlash + 1, strlen(lastSlash + 1) + 1);

			char *newDestination = malloc(strlen(destination)
				- strlen(lastSlash) + 1);

			memcpy(newDestination, destination, strlen(destination)
				- strlen(lastSlash) + 1);

			newDestination[strlen(destination) - strlen(lastSlash)] = '\0';

			dest = cd(currentNode, newDestination, NO_PRINT);

			if (dest == currentNode) {
				printf("mv: failed to access '%s': Not a directory\n",
					destination);
			} else {
				touch(dest, strdup(newFileName),
					strdup(((FileContent *)src->content)->text));
			}
			free(newDestination);
			free(newFileName);
		}
	}
	rm(src->parent, src->name);
}

void freeTree(FileTree* fileTree) {
	FolderContent curr_content = *(FolderContent *)(fileTree->root->content);
	ListNode *it = curr_content.children->head, *next;

	while (it) {
		next = it->next;
		rmrec(fileTree->root, it->info->name);
		it = next;
	}

	free(fileTree->root->name);
	free(((FolderContent *)fileTree->root->content)->children);
	free(fileTree->root->content);
	free(fileTree->root);
	free(fileTree);
}

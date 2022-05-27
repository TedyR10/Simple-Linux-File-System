// Copyright 2022 Mitran Andrei-Gabriel & Rolea Theodor-Ioan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"
#include "utils.h"
#define TREE_CMD_INDENT_SIZE 4
#define NO_ARG ""
#define PARENT_DIR ".."
#define NO_PRINT 0

// Creates the root directory
FileTree *createFileTree(char* rootFolderName) {
	// Allocs memory for the structure
	FileTree *tree = malloc(sizeof(FileTree));
	// Defensive programming xd
	DIE(NULL == tree, "Malloc failed for tree\n");
	tree->root = malloc(sizeof(TreeNode));
	DIE(NULL == tree->root, "Malloc failed for tree->root\n");

	tree->root->name = malloc(strlen(rootFolderName) + 1);
	DIE(NULL == tree->root->name, "Malloc() failed for tree->root->name\n");
	memcpy(tree->root->name, rootFolderName, strlen(rootFolderName) + 1);

	tree->root->parent = NULL;
	tree->root->type = FOLDER_NODE;

	// Allocs memory for content in advance
	tree->root->content = malloc(sizeof(FolderContent));
	DIE(NULL == tree->root->content,
		"Malloc() failed for tree->root->content\n");
	FolderContent content;
	// Allocs memory for the future files and directories
	content.children = malloc(sizeof(List));
	DIE(NULL == content.children,
		"Malloc() failed for content.children\n");
	content.children->head = NULL;
	memcpy(tree->root->content, &content, sizeof(content));

	free(rootFolderName);
	return tree;
}

// Lists every file in a given directory
void ls(TreeNode* currentNode, char* arg) {
	// Checks if the given node doesn't exist or is a file
	if (!currentNode || currentNode->type == FILE_NODE)
		return;

	FolderContent content = *(FolderContent *)(currentNode->content);
	ListNode *it = content.children->head;

	// If no file is specified, lists the content in the current directory
	if (!strcmp(arg, NO_ARG)) {
		while (it) {
			printf("%s\n", it->info->name);
			it = it->next;
		}
		return;
	}

	// Checks if the specified file or directory exists
	while (it) {
		if (!strcmp(it->info->name, arg))
			break;
		it = it->next;
	}

	// Error if the file or directory was not found
	if (!it) {
		printf("ls: cannot access '%s': No such file or directory\n", arg);
		return;
	}

	// If ls is used on a file, prints the content of the file
	if (it->info->type == FILE_NODE) {
		printf("%s: %s\n", it->info->name,
			(*(FileContent *)(it->info->content)).text);
		return;
	}

	// If ls is user on a directory, prints the contents of the directory
	if (it->info->type == FOLDER_NODE) {
		FolderContent itContent = *(FolderContent *)(it->info->content);
		ListNode *curr = itContent.children->head;

		while (curr) {
			printf("%s\n", curr->info->name);
			curr = curr->next;
		}
		return;
	}
}

// Prints the directory we are currently in
void pwd(TreeNode* treeNode) {
	// Checks if the given node doesn't exist or is a file
	if (!treeNode || treeNode->type == FILE_NODE)
		return;

	// We are going to keep in mind the path size for printing
	int pathSize = 1;
	// We are going to have a vector of directories we've been through
	char **path = malloc(sizeof(char *));
	DIE(NULL == path, "Malloc failed for path\n");

	TreeNode *it = treeNode;
	while (it) {
		// Adds every directory we go into into the vector
		path[pathSize - 1] = it->name;

		// Increments the pathSize
		++pathSize;
		// Realloc the new path with an increased size
		char **newPath = realloc(path, pathSize * sizeof(char *));
		DIE(NULL == newPath, "Malloc failed for newPath\n");

		path = newPath;

		it = it->parent;
	}

	// Prints the path
	for (int i = pathSize - 2; i >= 1; --i)
		printf("%s/", path[i]);
	printf("%s\n", path[0]);

	free(path);
}

// Goes into another directory
TreeNode* cd(TreeNode* currentNode, char* path, int print) {
	// Checks if the given node doesn't exist, is a file, or the path is wrong
	if (!currentNode || currentNode->type == FILE_NODE || !path)
		return currentNode;

	// Keeps in mind the node we started from
	TreeNode *ogNode = currentNode;
	// We break apart the given path into tokens
	char *tok = strtok(path, "/\n");

	while (tok) {
		// If the given path has ".." in it, it goes back a directory
		if (!strcmp(tok , PARENT_DIR)) {
			currentNode = currentNode->parent;
		} else {
			// Else, seraches for the specified path in the given directory
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
				// If the specified path is not correct, an error is displayed
				if (print)
					printf("cd: no such file or directory: %s\n", path);
				return ogNode;
			}
		}

		tok = strtok(NULL, "/\n");
	}
	return currentNode;
}

// Prints every file and directory from the given node
void tree(TreeNode* currentNode, char* arg) {
	// Checks if the given node doesn't exist or is a file
	if (!currentNode || currentNode->type == FILE_NODE)
		return;

	// Keeps in mind the number of files and folders plus the level for spaces
	static int files = 0, folders = 0, level = 0;

	TreeNode *workingNode = currentNode;
	// Checks if the given node is the one we are currently in
	if (strcmp(arg, NO_ARG)) {
		workingNode = cd(currentNode, arg, NO_PRINT);

		if (workingNode == currentNode || workingNode->type == FILE_NODE) {
			printf("%s [error opening dir]\n\n0 directories, 0 files\n", arg);
			return;
		}
	}

	FolderContent content = *(FolderContent *)(workingNode->content);
	ListNode *it = content.children->head;

	// Prints every file and directory in the current node
	while (it) {
		// We are using level here to print out the spaces accordingly
		for (int i = 0; i < level; ++i)
			printf("    ");
		printf("%s\n", it->info->name);

		// If the node we are going into is a directory, we will call tree on it
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

// Creates a new directory in the current directory
void mkdir(TreeNode* currentNode, char* folderName) {
	// Checks if the given node doesn't exist, is a file or we are not given
	// A folder name
	if (!currentNode || currentNode->type == FILE_NODE || !folderName)
		return;

	FolderContent currContent = *(FolderContent *)(currentNode->content);
	ListNode *it = currContent.children->head;

	// Checks if the directory already exists
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

	// Allocs memory for the new directory the same way as the file tree
	TreeNode *node = malloc(sizeof(TreeNode));
	DIE(NULL == node, "Malloc failed for node\n");

	node->name = malloc(strlen(folderName) + 1);
	DIE(NULL == node->name, "Malloc() failed for tree->root->name\n");
	memcpy(node->name, folderName, strlen(folderName) + 1);

	node->parent = currentNode;
	node->type = FOLDER_NODE;

	node->content = malloc(sizeof(FolderContent));
	DIE(NULL == node->content, "Malloc failed for node->content\n");

	FolderContent content;
	content.children = malloc(sizeof(List));
	DIE(NULL == content.children, "Malloc failed for content.children\n");

	content.children->head = NULL;
	memcpy(node->content, &content, sizeof(content));

	ListNode *listNode = malloc(sizeof(ListNode));
	DIE(NULL == listNode, "Malloc failed for listNode\n");
	listNode->info = node;
	listNode->next = ((FolderContent *)currentNode->content)->children->head;
	((FolderContent *)currentNode->content)->children->head = listNode;

	free(folderName);
}

// Deletes everything in a given directory
void rmrec(TreeNode* currentNode, char* resourceName) {
	// Checks if the given node doesn't exist, is a file or the resourceName is
	// Not given
	if (!currentNode || currentNode->type == FILE_NODE || !resourceName)
		return;

	FolderContent currContent = *(FolderContent *)(currentNode->content);
	ListNode *it = currContent.children->head;

	// Checks if the given resourceName exists
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

	// If the specified resource is a file, simply use rm function
	if (it->info->type == FILE_NODE) {
		rm(currentNode, resourceName);

		return;
	} else {
		// Else, remove everything inside of it using rm function
		FolderContent itContent = *(FolderContent *)(it->info->content);
		ListNode *i = itContent.children->head, *next;

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
		// Lastly, remove the current node we are in
		rmdir(currentNode, it->info->name);
	}
}

// Deletes the given file
void rm(TreeNode* currentNode, char* fileName) {
	// Checks if the given node doesn't exist, is a file or the filename is not
	// Given
	if (!currentNode || currentNode->type == FILE_NODE || !fileName)
		return;

	FolderContent currContent = *(FolderContent *)(currentNode->content);
	ListNode *it = currContent.children->head, *prev = NULL;

	// Checks if the specified file exists
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

	// If the specified file is a directory, an error is displayed
	if (it->info->type == FOLDER_NODE) {
		printf("rm: cannot remove '%s': Is a directory", fileName);
		return;
	}

	// Remove the node from the list
	if (prev)
		prev->next = it->next;
	else
		currContent.children->head = it->next;

	// Free the removed node
	free(it->info->name);
	free(((FileContent *)it->info->content)->text);
	free(it->info->content);
	free(it->info);
	free(it);
}

// Deletes the given directory only if it is empty
void rmdir(TreeNode* currentNode, char* folderName) {
	// Checks if the given node doesn't exist, is a file or the folderName
	// Is not given
	if (!currentNode || currentNode->type == FILE_NODE || !folderName)
		return;

	FolderContent currContent = *(FolderContent *)(currentNode->content);
	ListNode *it = currContent.children->head, *prev = NULL;

	// Checks if the given folder exists
	while (it) {
		if (!strcmp(it->info->name, folderName))
			break;
		prev = it;
		it = it->next;
	}

	// If it doesn't exist, an error is displayed
	if (!it) {
		printf("rmdir: failed to remove '%s': No such file or directory",
			folderName);
		return;
	}

	// If it is a file, an error is displayed
	if (it->info->type == FILE_NODE) {
		printf("rmdir: failed to remove '%s': Not a directory", folderName);
		return;
	}

	// If it is not empty, an error is displayed
	if (((FolderContent *)it->info->content)->children->head) {
		printf("rmdir: failed to remove '%s': Directory not empty", folderName);
		return;
	}

	// Remove the node from the list
	if (prev)
		prev->next = it->next;
	else
		currContent.children->head = it->next;

	// Free the removed node
	free(it->info->name);
	free(((FolderContent *)it->info->content)->children);
	free(it->info->content);
	free(it->info);
	free(it);
}

// Creates a new file in the directory
void touch(TreeNode* currentNode, char* fileName, char* fileContent) {
	// Checks if the given node doesn't exist, is a file or the fileName
	// Is not given
	if (!currentNode || currentNode->type == FILE_NODE || !fileName)
		return;

	FolderContent currContent = *(FolderContent *)(currentNode->content);
	ListNode *it = currContent.children->head;

	// Checks if the file already exists
	while (it) {
		if (!strcmp(it->info->name, fileName))
			break;
		it = it->next;
	}

	if (it)
		return;

	// Allocs memory for the new file and add it into the children list
	TreeNode *node = malloc(sizeof(TreeNode));
	DIE(NULL == node, "Malloc failed for node\n");

	node->name = malloc(strlen(fileName) + 1);
	DIE(NULL == node->name, "Malloc() failed for tree->root->name\n");
	memcpy(node->name, fileName, strlen(fileName) + 1);

	node->parent = currentNode;
	node->type = FILE_NODE;

	node->content = malloc(sizeof(FileContent));
	DIE(NULL == node->content, "Malloc failed for node->content\n");

	FileContent content;
	content.text = malloc(strlen(fileContent) + 1);
	DIE(NULL == content.text, "Malloc failed for content.text\n");
	memcpy(content.text, fileContent, strlen(fileContent) + 1);
	memcpy(node->content, &content, sizeof(content));

	ListNode *listNode = malloc(sizeof(ListNode));
	DIE(NULL == listNode, "Malloc failed for listNode\n");

	listNode->info = node;
	listNode->next = ((FolderContent *)currentNode->content)->children->head;
	((FolderContent *)currentNode->content)->children->head = listNode;

	free(fileName);
	free(fileContent);
}

// Copies a file from the current directory to another directory
void cp(TreeNode* currentNode, char* source, char* destination) {
	// We are going to use a copy of the destination, because we are going to
	// Break it up into tokens
	char *destCopy = malloc(strlen(destination) + 1);
	DIE(NULL == destCopy, "Malloc failed for destCopy\n");
	memcpy(destCopy, destination, strlen(destination) + 1);
	TreeNode* src = cd(currentNode, source, NO_PRINT);
	TreeNode* dest = cd(currentNode, destCopy, NO_PRINT);
	free(destCopy);

	// Checks if the source is a folder
	if (src->type == FOLDER_NODE) {
		printf("cp: -r not specified; omitting directory '%s'\n", source);
		return;
	}

	if (dest != currentNode) {
		// If the destination is a directory, we are going to create a new file
		// And paste the contents from src into it
		if (dest->type == FOLDER_NODE) {
			touch(dest, strdup(src->name),
				strdup(((FileContent *)src->content)->text));

			return;
		} else if (dest->type == FILE_NODE) {
			// If the destination is a file, we are going to free its contents
			// And then realloc the required memory and copy the new contents
			free(((FileContent *)dest->content)->text);
			((FileContent *)dest->content)->text =
				malloc(strlen(((FileContent *)src->content)->text) + 1);
				DIE(NULL == ((FileContent *)dest->content)->text,
				"Malloc failed for (FileContent *)dest->content)->text\n");
			memcpy(((FileContent *)dest->content)->text,
				((FileContent *)src->content)->text,
					strlen(((FileContent *)src->content)->text) + 1);

			return;
		}
	} else {
		// We are going to search for the last directory in the path
		char *lastSlash = strrchr(destination, '/');

		// We are going to read the path from finish to start to find where the
		// Destination is
		if (lastSlash) {
			char *newFileName = malloc(strlen(lastSlash + 1) + 1);
			DIE(NULL == newFileName, "Malloc failed for newFileName\n");

			memcpy(newFileName, lastSlash + 1, strlen(lastSlash + 1) + 1);

			char *newDestination = malloc(strlen(destination)
				- strlen(lastSlash) + 1);
			DIE(NULL == newDestination, "Malloc failed for newDestination\n");


			memcpy(newDestination, destination, strlen(destination)
				- strlen(lastSlash) + 1);

			newDestination[strlen(destination) - strlen(lastSlash)] = '\0';

			dest = cd(currentNode, newDestination, NO_PRINT);

			// If we are trying to copy something into the directory we are
			// Already in, an error is displayed
			if (dest == currentNode) {
				printf("cp: failed to access '%s': Not a directory\n",
					destination);
			} else {
				// Else, the file is created at the destination
				touch(dest, strdup(newFileName),
					strdup(((FileContent *)src->content)->text));
			}
			free(newDestination);
			free(newFileName);
		}
	}
}

// Moves a file from the current directory into another
void mv(TreeNode* currentNode, char* source, char* destination) {
	// We are going to use a copy of the destination, because we are going to
	// Break it up into tokens
	char *destCopy = malloc(strlen(destination) + 1);
	DIE(NULL == destCopy, "Malloc failed for destCopy\n");
	memcpy(destCopy, destination, strlen(destination) + 1);
	TreeNode* src = cd(currentNode, source, NO_PRINT);
	TreeNode* dest = cd(currentNode, destCopy, NO_PRINT);
	free(destCopy);

	if (src->type == FOLDER_NODE && dest->type == FOLDER_NODE) {
		// If the src is a directory, it creates a new one at the destination
		// And deletes the one from the currentNode
		mkdir(dest, strdup(src->name));
		FolderContent currContent = *(FolderContent *)(dest->content);
		ListNode *it = currContent.children->head;

		((FolderContent *)it->info->content)->children->head =
			((FolderContent *)src->content)->children->head;

		((FolderContent *)src->content)->children->head = NULL;

		rmdir(src->parent, src->name);
		return;
	}

	// Assumes that src is a file, not a directory
	if (dest != currentNode) {
		if (dest->type == FOLDER_NODE) {
			// if dest is a directory, creates a new file and delete the old one
			touch(dest, strdup(src->name),
				strdup(((FileContent *)src->content)->text));

			rm(src->parent, src->name);
			return;
		} else if (dest->type == FILE_NODE) {
			// If dest is a file, replaces it contents and deletes the file from
			// the currentNode
			free(((FileContent *)dest->content)->text);
			((FileContent *)dest->content)->text =
				malloc(strlen(((FileContent *)src->content)->text) + 1);
				DIE(NULL == ((FileContent *)dest->content)->text,
					"Malloc failed for (FileContent *)dest->content)->text\n");

			memcpy(((FileContent *)dest->content)->text,
				((FileContent *)src->content)->text,
					strlen(((FileContent *)src->content)->text) + 1);

			rm(src->parent, src->name);
			return;
		}
	} else {
		// We are going to search for the last directory in the path
		char *lastSlash = strrchr(destination, '/');

		if (lastSlash) {
			// We are going to read the path from finish to start to find where
			// The destination is
			char *newFileName = malloc(strlen(lastSlash + 1) + 1);
			DIE(NULL == newFileName, "Malloc failed for newFileName\n");
			memcpy(newFileName, lastSlash + 1, strlen(lastSlash + 1) + 1);

			char *newDestination = malloc(strlen(destination)
				- strlen(lastSlash) + 1);
			DIE(NULL == newDestination, "Malloc failed for newDestination\n");

			memcpy(newDestination, destination, strlen(destination)
				- strlen(lastSlash) + 1);

			newDestination[strlen(destination) - strlen(lastSlash)] = '\0';

			dest = cd(currentNode, newDestination, NO_PRINT);

			// If we are trying to copy something into the directory we are
			// Already in, an error is displayed
			if (dest == currentNode) {
				printf("mv: failed to access '%s': Not a directory\n",
					destination);
			} else {
				// Else, the file is created at the destination
				touch(dest, strdup(newFileName),
					strdup(((FileContent *)src->content)->text));
			}
			free(newDestination);
			free(newFileName);
		}
	}
	// Removes the original file from currentNode
	rm(src->parent, src->name);
}

// Frees the whole file system
void freeTree(FileTree* fileTree) {
	FolderContent currContent = *(FolderContent *)(fileTree->root->content);
	ListNode *it = currContent.children->head, *next;

	// Uses rmrec to free every single file and directory
	while (it) {
		next = it->next;
		rmrec(fileTree->root, it->info->name);
		it = next;
	}

	// Frees the file tree and root
	free(fileTree->root->name);
	free(((FolderContent *)fileTree->root->content)->children);
	free(fileTree->root->content);
	free(fileTree->root);
	free(fileTree);
}

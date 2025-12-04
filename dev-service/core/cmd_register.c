#include "cmd_register.h"
#include "linked_list.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static ll_list_t *command_list = NULL;

int command_init()
{
    command_list = ll_create();
    if (command_list == NULL)
    {
        LOGE("Failed to create command list");
        return -1; // Return error if linked list creation fails
    }

    return 0;
}

int command_register(command_t *cmd)
{
    if (cmd == NULL || command_list == NULL) 
    {
        LOGE("Invalid command or command list not initialized");
        return -1; // Return error if command is NULL or command list is not initialized
    }

    // Allocate memory for command name and copy it
    char *cmd_name_copy = strdup(cmd->name);
    if (cmd_name_copy == NULL) 
    {
        LOGE("Failed to allocate memory for command name");
        return -1; // Return error if memory allocation fails
    }

    // Allocate memory for command structure
    command_t *new_cmd = (command_t *)malloc(sizeof(command_t));
    if (new_cmd == NULL) 
    {
        free(cmd_name_copy);
        LOGE("Failed to allocate memory for command structure");
        return -1; // Return error if memory allocation fails
    }

    new_cmd->name = cmd_name_copy;
    new_cmd->handler = cmd->handler;

    // Add the new command to the linked list
    if (ll_add(command_list, (void *)new_cmd) != 0) 
    {
        free((void *)new_cmd->name);
        free(new_cmd);
        LOGE("Failed to add command to the command list");
        return -1; // Return error if adding to linked list fails
    }

    LOGI("Registered command: %s", new_cmd->name);

    return 0; // Return 0 on success
}

void command_free_all()
{
    if (command_list == NULL) 
    {
        return; // Nothing to free
    }

    ll_node_t *current = command_list->head;
    while (current != NULL) 
    {
        command_t *cmd = (command_t *)current->data;
        if (cmd != NULL) 
        {
            free((void *)cmd->name); // Free command name
            free(cmd);               // Free command structure
        }
        current = current->next;
    }

    ll_free(command_list); // Free the linked list itself
    command_list = NULL;   // Set to NULL after freeing
}

void command_print_list()
{
    if (command_list == NULL) 
    {
        LOGE("Command list is not initialized.");
        return;
    }

    ll_node_t *current = command_list->head;
    printf("Registered commands:\n");
    while (current != NULL) 
    {
        command_t *cmd = (command_t *)current->data;
        printf(" - %s\n", cmd->name);
        current = current->next;
    }
}

command_t *command_find(const char *name)
{
    if (command_list == NULL || name == NULL) 
    {
        return NULL; // Command list not initialized or name is NULL
    }

    ll_node_t *current = command_list->head;
    while (current != NULL) 
    {
        command_t *cmd = (command_t *)current->data;
        if (strcmp(cmd->name, name) == 0) 
        {
            return cmd; // Found the command
        }
        current = current->next;
    }

    return NULL; // Command not found
}
#include "linked_list.h"

node_t *node_create(void *data, int priority)
{
	// allocate memory for the node
	node_t *node = calloc(1, sizeof(node_t));

	node->data = data;
	node->priority = priority;
	node->next = NULL;

	return node;
}

void node_destroy(node_t *node, void (*free_data)(void *))
{
	free_data(node->data);
	free(node);
}

linked_list_t *linked_list_create(void)
{
	// allocate memory for the list - calloc initializes the memory to 0
	linked_list_t *list = calloc(1, sizeof(linked_list_t));

	return list;
}

void linked_list_destroy(linked_list_t **list, void (*free_data)(void *))
{
	node_t *current = (*list)->head;
	node_t *next = NULL;

	// iterate through the list and free all nodes
	while (current != NULL) {
		next = current->next;
		node_destroy(current, free_data);
		current = next;
	}

	free(*list);
	*list = NULL;
}

void linked_list_add(linked_list_t **list, void *data)
{
	if (*list == NULL)
		return;

	// allocate memory for the new node
	node_t *new_node = node_create(data, 0);

	if (new_node == NULL)
		return;

	// if the list is empty, set the new node as head
	if ((*list)->head == NULL) {
		(*list)->head = new_node;
		return;
	}

	// insert the new node
	new_node->next = (*list)->head;
	(*list)->head = new_node;
}

void *linked_list_remove(linked_list_t **list, int pos)
{
	if (*list == NULL)
		return NULL;

	// check if the list is empty
	if ((*list)->head == NULL)
		return NULL;

	void *data = NULL;

	// if the position is 0, remove the head
	if (pos == 0) {
		node_t *head = (*list)->head;
		data = head->data;
		(*list)->head = head->next;
		free(head);
		(*list)->size -= 1;
		return data;
	}

	// iterate through the list until the position is reached
	node_t *current = (*list)->head;
	int i = 0;

	while (current != NULL && i < pos - 1) {
		current = current->next;
		i++;
	}

	// check if the position is not valid
	if (current == NULL || current->next == NULL)
		return NULL;

	// remove the node
	node_t *next = current->next->next;

	// decrement the list size
	(*list)->size -= 1;

	// save the data
	data = current->next->data;

	// free the memory
	free(current->next);
	current->next = next;

	return data;
}

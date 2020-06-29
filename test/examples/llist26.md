# [Single-Linked List in C - beginner](https://codereview.stackexchange.com/questions/217145)
### tags: ['beginner', 'c', 'linked-list']

I'm a python dev by day trying to learn C. 

This is a simple implementation of a singly linked list. As a noob I would like comments on style and C conventions as well as functional remarks on memory management etc.  

    #include <stdio.h>
    #include <stdlib.h>
    
    /*
            --------linkedList---------
            |                          |
            |                          |
            |                          |
            |                          | 
            *head  -->  nodes  -->   *end   
    */
    
    struct linkedList {
        struct node * head;
        struct node * end;
        int len;
    };
    struct node {
        int id;
        int val;
        struct node * next;
    };
    struct linkedList * createList() {
        struct linkedList * l_list = (struct linkedList * ) malloc(sizeof(struct linkedList));
    
        l_list->head = NULL;
        l_list->end = NULL;
        l_list->len = 0;
        printf("created list\n");
        return l_list;
    }
    
    struct node * createNode(int id, int val) {
        struct node * n_node = (struct node * ) malloc(sizeof(struct node));
    
        n_node->id = id;
        n_node->val = val;
        n_node->next = NULL;
        printf("created node\n");
        return n_node;
    }
    
    void addNode(struct linkedList * ptr, int id, int val) {
        struct node * new_node = createNode(id, val);
        if (ptr->len == 0) {
            ptr->head = new_node;
            ptr->end = new_node;
            ptr->len += 1;
            printf("created a list and added a new value\n");
        } else {
            // update next of previous end
            // make new end this node 
            struct node * temp;
            temp = ptr->end;
            temp->next = new_node;
            ptr->end = new_node;
            ptr->len += 1;
            // printf("updated a preexisting list\n");
        }
    }
    
    void printListWithFor(struct linkedList * someList) {
        struct node currentNode = * someList->head;
        printf("current length of list is %d\n", someList->len);
        printf("first item is %d, last item is %d\n", someList->head->val, someList->end->val);
        if (currentNode.next == NULL) {
            printf("current node id is %d, with a value of %d\n", currentNode.id, currentNode.val);
        }
        for (int i = 0; i < ( * someList).len; i++) {
            printf("current node id is %d, with a value of %d\n", currentNode.id, currentNode.val);
            currentNode = * currentNode.next;
        }
    }
    
    void printListWithWhile(struct linkedList * someList) {
        struct node currentNode = * someList->head;
        struct node endNode = * someList->end;
        printf("current length of list is %d\n", someList->len);
        printf("first item is %d, last item is %d\n", someList->head->val, someList->end->val);
        if (currentNode.next == NULL) {
            printf("current node id is %d, with a value of %d\n", currentNode.id, currentNode.val);
        }
        while (currentNode.id != endNode.id) {
            printf("current node id is %d, with a value of %d\n", currentNode.id, currentNode.val);
            currentNode = * currentNode.next;
        }
        printf("current node id is %d, with a value of %d\n", currentNode.id, currentNode.val);
    }
    struct node * findNode(struct linkedList * someList, int id) {
        struct node headNode = * someList->head;
        struct node endNode = * someList->end;
        struct node * nullNode = createNode(-1, -1);
    
        if (headNode.id == id) {
            free(nullNode);
            return someList->head;
        }
        if (endNode.id == id) {
            free(nullNode);
            return someList->end;
        }
        struct node * currentNode = headNode.next;
        while (currentNode-> id != endNode.id) {
            if (currentNode->id == id) {
                free(nullNode);
                return currentNode;
            }
            currentNode = currentNode->next;
        }
        return nullNode;
    }
    
    int delNode(struct linkedList * someList, int id) {
        struct node * headNode = someList->head;
        struct node * endNode = someList->end;
    
        if (headNode->id == id) {
            // remove node, replace it with next node, free memory
            struct node * temp = headNode->next;
            someList->head = temp;
            printf("removed a node with id of %d and value of %d\n", headNode->id, headNode->val);
            free(headNode);
            someList->len -= 1;
            return 0;
        }
        if (endNode->id == id) {
            printf("removed a node with id of %d and value of %d\n", endNode->id, endNode->val);
            free(endNode);
            someList->len -= 1;
            return 0;
        }
        struct node * currentNode = headNode->next;
        struct node * prevNode = headNode;
        while (prevNode->id != endNode->id) {
            if (currentNode->id == id) {
                struct node * temp = currentNode->next;
                prevNode->next = temp;
                printf("removed a node with id %d and value of %d\n", currentNode->id, currentNode->val);
                free(currentNode);
                someList->len -= 1;
                return 0;
            }
            prevNode = currentNode;
            currentNode = currentNode->next;
        }
        return -1;
    
    }
    int main() {
    
        struct linkedList * list = createList();
        addNode(list, 1, 7);
        addNode(list, 2, 6);
        addNode(list, 3, 11);
        addNode(list, 5, 92);
        addNode(list, 18, 6);
        addNode(list, 10, 3);
        addNode(list, 50, 9);
    
        // printListWithWhile(list);
        // printListWithFor(list);
        printf("\n");
        struct node * foundNode = findNode(list, 1);
        printf("Node id : %d\n", foundNode->id);
        printf("Node val : %d\n", foundNode->val);
    
        printf("\n");
        // printListWithWhile(list);
        delNode(list, 2);
        printListWithWhile(list);
        delNode(list, 18);
        printf("\n");
        printListWithWhile(list);
        return 0;
    }
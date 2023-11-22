#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

#define MAX_STRING_LENGTH 100

typedef struct _Node {
    char value[MAX_STRING_LENGTH];
    struct _Node* next;
    pthread_mutex_t sync;
} Node;

typedef struct _Storage {
    Node *first;
} Storage;


Storage* initializeStorage() {
    Storage* storage = (Storage*)malloc(sizeof(Storage));
    if (storage == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    storage->first = NULL;
    return storage;
}

void destroyStorage(Storage* storage) {
    Node* current = storage->first;
    while (current != NULL) {
        Node* temp = current;
        current = current->next;
        free(temp);
    }
    free(storage);
}

void addNode(Storage* storage, const char* value) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        perror("Failed to allocate memory for Node");
        exit(EXIT_FAILURE);
    }

    strncpy(newNode->value, value, MAX_STRING_LENGTH);
    newNode->next = NULL;
    pthread_mutex_init(&(newNode->sync), NULL);

    if (storage->first == NULL) {
        storage->first = newNode;
    } else {
        Node* current = storage->first;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
}

void printList(Storage* storage) {
    Node* current = storage->first;
    while (current != NULL) {
        printf("%s\n", current->value);
        current = current->next;
    }
}

void* ascendingLength(void* arg) {
    Storage* storage = (Storage*)arg;
    int iterationCount = 0;

    while (1) {
        Node* current = storage->first;
        int pairCount = 0;

        pthread_mutex_lock(&(current->sync));
        Node* nextNode = current->next;

        while (nextNode != NULL) {
            pthread_mutex_lock(&(nextNode->sync));
            if (strlen(current->value) < strlen(nextNode->value)) {
                pairCount++;
            }
            pthread_mutex_unlock(&(current->sync));
            current = nextNode;
            nextNode = current->next;
        }

        iterationCount++;
        printf("Ascending: Iteration %d, Pair count: %d\n", iterationCount, pairCount);
        sleep(1);

        pthread_mutex_unlock(&(current->sync));
    }

    return NULL;
}

void* descendingLength(void* arg) {
    Storage* storage = (Storage*)arg;
    int iterationCount = 0;

    while (1) {
        Node* current = storage->first;
        int pairCount = 0;

        pthread_mutex_lock(&(current->sync));
        Node* nextNode = current->next;

        while (nextNode != NULL) {
            pthread_mutex_lock(&(nextNode->sync));
            if (strlen(current->value) > strlen(nextNode->value)) {
                pairCount++;
            }
            pthread_mutex_unlock(&(current->sync));
            current = nextNode;
            nextNode = current->next;
        }

        iterationCount++;
        printf("Descending: Iteration %d, Pair count: %d\n", iterationCount, pairCount);
        sleep(1);

        pthread_mutex_unlock(&(current->sync)); 
    }

    return NULL;
}

void* equalLength(void* arg) {
    Storage* storage = (Storage*)arg;
    int iterationCount = 0;

    while (1) {
        Node* current = storage->first;
        int pairCount = 0;

        pthread_mutex_lock(&(current->sync)); 
        Node* nextNode = current->next;

        while (nextNode != NULL) {
            pthread_mutex_lock(&(nextNode->sync)); 
            if (strlen(current->value) == strlen(nextNode->value)) {
                pairCount++;
            }
            pthread_mutex_unlock(&(current->sync)); 
            current = nextNode;
            nextNode = current->next;
        }

        iterationCount++;
        printf("Equal: Iteration %d, Pair count: %d\n", iterationCount, pairCount);
        sleep(1); 

        pthread_mutex_unlock(&(current->sync)); 
    }

    return NULL;
}

int swapNeeded(Node* first, Node* second, Node* third) {
    if (first == NULL || second == NULL || third == NULL) {
        return 0; 
    }

    srand(time(NULL));

    return rand() % 2;
}

void* swapElements(void* arg) {
    Storage* storage = (Storage*)arg;
    int swapCount = 0;

    while (1) {
        Node* current = storage->first;

        while (current != NULL && current->next != NULL) {
            // printf("9\n");
            Node* nextNode = current->next;

            pthread_mutex_lock(&(current->sync)); 
            pthread_mutex_lock(&(nextNode->sync)); 
            Node* temp = nextNode->next;

            if (temp != NULL) pthread_mutex_lock(&(temp->sync)); 

            if (swapNeeded(current, nextNode, temp)) {
                nextNode->next = temp->next;
                temp->next = nextNode;
                current->next = temp;

                swapCount++;
                printf("Swap count: %d\n", swapCount);
                // printList(storage);
            }

            if (temp != NULL) pthread_mutex_unlock(&(temp->sync)); 

            pthread_mutex_unlock(&(nextNode->sync)); 
            pthread_mutex_unlock(&(current->sync)); 

            current = current->next;
        }

        sleep(1); 
    }

    return NULL;
}




int main() {
    Storage* storage = initializeStorage();


    for (int i = 0; i < 10000; i++) {
        addNode(storage, "String" + i);
    }
    // addNode(storage, "String 1");
    // addNode(storage, "String 2");
    // addNode(storage, "Strings 3");
    // addNode(storage, "Strings2 4");
    // addNode(storage, "String 5");

    pthread_t ascendingThread, descendingThread, sameLengthThread, swapElementsThread1, swapElementsThread2, swapElementsThread3;

    pthread_create(&ascendingThread, NULL, ascendingLength, storage);
    pthread_create(&descendingThread, NULL, descendingLength, storage);
    pthread_create(&sameLengthThread, NULL, equalLength, storage);
    pthread_create(&swapElementsThread1, NULL, swapElements, storage);
    pthread_create(&swapElementsThread2, NULL, swapElements, storage);
    pthread_create(&swapElementsThread3, NULL, swapElements, storage);

    pthread_join(ascendingThread, NULL);
    pthread_join(descendingThread, NULL);
    pthread_join(sameLengthThread, NULL);
    pthread_join(swapElementsThread1, NULL);
    pthread_join(swapElementsThread2, NULL);
    pthread_join(swapElementsThread3, NULL);

    destroyStorage(storage);

    return EXIT_SUCCESS;
}

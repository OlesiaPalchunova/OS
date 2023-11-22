#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h>
#include <syscall.h>
#include <string.h>

#define SPINLOCK_FREE 0
#define SPINLOCK_LOCKED 1


typedef struct _QueueNode {
    int val;
    struct _QueueNode *next;
} qnode_t;

typedef struct _Queue {
    qnode_t *first;
    qnode_t *last;

    int count;
    int max_count;

    long add_attempts;
    long get_attempts;
    long add_count;
    long get_count;

    volatile uint32_t lock;

} queue_t;

void spin_lock(volatile uint32_t *lock) {
    while (__sync_lock_test_and_set(lock, SPINLOCK_LOCKED)) {
        while (*lock == SPINLOCK_LOCKED) {
            sched_yield();
        }
    }
}

void spin_unlock(volatile uint32_t *lock) {
    __sync_lock_release(lock);
}

void mutex_lock(volatile uint32_t *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        syscall(SYS_futex, lock, FUTEX_WAIT_PRIVATE, 1, NULL, NULL, 0);
    }
}

void mutex_unlock(volatile uint32_t *lock) {
    *lock = 0;
    syscall(SYS_futex, lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

queue_t* queue_init(int max_count) {
    queue_t *q = (queue_t*)malloc(sizeof(queue_t));
    if (!q) {
        printf("Cannot allocate memory for a queue\n");
        abort();
    }

    q->first = NULL;
    q->last = NULL;
    q->max_count = max_count;
    q->count = 0;
    q->lock = SPINLOCK_FREE;

    return q;
}

void queue_destroy(queue_t *q) {
    if (q == NULL) {
        return;
    }

    while (q->first != NULL) {
        qnode_t *tmp = q->first;
        q->first = q->first->next;
        free(tmp);
    }

    free(q);
}

int queue_add(queue_t *q, int val) {
    spin_lock(&q->lock);

    q->add_attempts++;
    printf("Запись: попытки %ld\n", q->add_attempts);

    if (q->count >= q->max_count) {
        spin_unlock(&q->lock);
        return 0;
    }
    qnode_t *new_node = (qnode_t*)malloc(sizeof(qnode_t));

    if (!new_node) {
        spin_unlock(&q->lock);
        printf("Cannot allocate memory for new node\n");
        abort();
    }

    new_node->val = val;
    new_node->next = NULL;

    if (!q->first)
        q->first = q->last = new_node;
    else {
        q->last->next = new_node;
        q->last = q->last->next;
    }

    q->count++;
    q->add_count++;
    printf("Запись: успешно %ld\n", q->add_count);

    spin_unlock(&q->lock);

    return 1;
}


int queue_get(queue_t *q, int *val) {
    spin_lock(&q->lock);

    q->get_attempts++;
    printf("Чтение: попытки %ld\n", q->get_attempts);

    if (q->count == 0) {
        spin_unlock(&q->lock);
        return 0;
    }

    qnode_t *tmp = q->first;

    *val = tmp->val;
    q->first = q->first->next;

    free(tmp);
    q->count--;
    q->get_count++;
    printf("Чтение: успешно %ld\n", q->get_count);

    spin_unlock(&q->lock);

    return 1;
}


void *reader(void *arg) {
	int expected = 0;
	queue_t *q = (queue_t *)arg;
	printf("reader [%d %d %d]\n", getpid(), getppid(), gettid());


	while (1) {

		int val = -1;
		int ok = queue_get(q, &val);

		expected = val + 1;
        usleep(100);
	}

	return NULL;
}

void *writer(void *arg) {
	int i = 0;
	queue_t *q = (queue_t *)arg;
	printf("writer [%d %d %d]\n", getpid(), getppid(), gettid());


	while (1) {

		int ok = queue_add(q, i);
		if (!ok)
			continue;
		// printf("writer [%d]\n", i);

		i++;

		usleep(100);
	}

	return NULL;
}

int main() {
	pthread_t tid;
	queue_t *q;
	int err;

	printf("main [%d %d %d]\n", getpid(), getppid(), gettid());

	q = queue_init(10000);

	err = pthread_create(&tid, NULL, reader, q);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}

	sched_yield();

	err = pthread_create(&tid, NULL, writer, q);
	if (err) {
		printf("main: pthread_create() failed: %s\n", strerror(err));
		return -1;
	}
	

	pthread_join(tid, NULL);

	pthread_exit(NULL);

	return 0;
}

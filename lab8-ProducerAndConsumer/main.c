#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/time.h>

#define I 5  // Number of producer
#define J 4  // Number of consumer
#define N 3  // size of buffer
#define K 15 // Number of the end of process

double getTime()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000) / 1000.0;
}
typedef struct Producer
{
    int id;
} Producer;
typedef struct Consumer
{
    int id;
} Consumer;
typedef struct item
{
    int producerId;
    int consumerId;
    int id;
    double produceTime; // time from process start.
    double consumeTime; 
    int status; // 0 has not been produced, 1 has been produced, 2 has been in buffer, 3 has been consumed.
} Item;
typedef struct Buffer
{
    int id;
    int itemId;
} Buffer;
typedef int semaphore;

Item item[K];
Buffer buffer[N];
int itemCount, consumeCount;
double timeStart;
semaphore empty;
pthread_cond_t prod = PTHREAD_COND_INITIALIZER, cons = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;
FILE* pFile;

void *produce(void*  arg) {
    Producer *producer = (Producer*)arg;
    char str[80];
    while (1)
    {   
        if (itemCount >= 15) break;
        if (empty == 0)
        {
            pthread_cond_signal(&cons);
            pthread_cond_wait(&prod, &mutex);
        }
        // printf("Try get buffer\tProducer:%d\tTime:%.3lfs\n", producer->id, getTime()-timeStart);
        sprintf(str,"Try get buffer\tProducer:%d\tTime:%.3lfs\n", producer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);

        if(!pthread_mutex_trylock(&mutex)) continue;
        sprintf(str, "Get the buffer\tProducer:%d\tTime:%.3lfs\n", producer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);

        int i, opnum;
        for(i = 0; i<N; i++)
        {
            if(buffer[i].itemId == -1)
            {
                sprintf(str, "%d[ ]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
                opnum = i;
            }
            else
            {
                sprintf(str, "%d[x]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
        }
        sprintf(str, "\n");
        printf("%s", str);
        fputs(str, pFile);

        item[itemCount].producerId = producer->id;
        item[itemCount].produceTime = getTime()-timeStart;
        item[itemCount].status = 1;
        sprintf(str, "Produce an item%d\tProducer:%d\tTime:%.3lfs\n", itemCount, producer->id, item[itemCount].produceTime);
        printf("%s", str);
        fputs(str, pFile);
        buffer[opnum].itemId = itemCount;
        sprintf(str, "Put item%d in buffer%d\tProducer:%d\tTime:%.3lfs\n", itemCount, buffer[opnum].id, producer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        itemCount++;
        empty--;
        // Finish produce and item ++ 
        for(i = 0; i<N; i++)
        {
            if(buffer[i].itemId == -1)
            {
                sprintf(str, "%d[ ]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
            else
            {
                sprintf(str, "%d[x]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
        }
        sprintf(str, "\n");
        printf("%s", str);
        fputs(str, pFile);
        sprintf(str, "Leave the buffer\tProducer:%d\tTime:%.3lfs\n\n", producer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        pthread_cond_signal(&cons);
        pthread_mutex_unlock(&mutex);
        if (itemCount >= 15) break;

        sleep(1 + rand() % 4);
    }
    printf("生产线程[%d]返回 with item count:%d\n", producer->id, itemCount);
    return;
}

void *consume(void* arg) {
    Consumer *consumer = (Consumer*)arg;
    char str[80];
    while (1)
    {
        if(consumeCount>=15) break;
        if(empty == N) {
            pthread_cond_signal(&prod);
            pthread_cond_wait(&cons, &mutex);
        }

        sprintf(str, "Try get buffer\tConsumer:%d\tTime:%.3lfs\n", consumer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        if(!pthread_mutex_trylock(&mutex))
        sprintf(str, "Get the buffer\tConsumer:%d\tTime:%.3lfs\n", consumer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        int i, opnum, itemid;
        for(i = 0; i<N; i++)
        {
            if(buffer[i].itemId == -1)
            {
                sprintf(str, "%d[ ]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
            else
            {
                sprintf(str, "%d[x]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
                opnum = i;
            }
        }
        sprintf(str, "\n");
        printf("%s", str);
        fputs(str, pFile);
        itemid = buffer[opnum].itemId;
        item[itemid].consumerId = consumer->id;
        item[itemid].consumeTime = getTime()-timeStart;
        item[itemid].status = 3;
        sprintf(str, "Consume an item%d\tConsumer:%d\tTime:%.3lfs\n", itemid, consumer->id, item[itemid].consumeTime);
        printf("%s", str);
        fputs(str, pFile);

        buffer[opnum].itemId = -1;
        sprintf(str, "Delete item%d from buffer%d\tConsumer:%d\tTime:%.3lfs\n", itemid, buffer[opnum].id, consumer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        consumeCount++;
        empty++;
        for(i = 0; i<N; i++)
        {
            if(buffer[i].itemId == -1)
            {
                sprintf(str, "%d[ ]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
            else
            {
                sprintf(str, "%d[x]\t", buffer[i].id);
                printf("%s", str);
                fputs(str, pFile);
            }
        }
        sprintf(str, "\n");
        printf("%s", str);
        fputs(str, pFile);
        pthread_cond_signal(&prod);
        pthread_mutex_unlock(&mutex);
        sprintf(str, "Leave the buffer\tConsumer:%d\tTime:%.3lfs\n\n", consumer->id, getTime()-timeStart);
        printf("%s", str);
        fputs(str, pFile);
        if(consumeCount>=15) return;
        sleep(1 + rand() % 4);
    }
    printf("消费线程[%d]返回 consume count:%d\n", consumer->id, consumeCount);
    return;
}

int main() {
    pFile = fopen("./log.txt", "w");
    Consumer con[J];
    Producer pro[I];
    timeStart = getTime();
    srand(timeStart);
    itemCount = 0;
    consumeCount = 0;
    empty = N;
    pthread_t proThread[I], conThread[J];
    int i;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&prod, NULL);
    pthread_cond_init(&cons, NULL);
    for (i = 0; i < J; i++) {
        con[i].id = i;
    }
    for (i = 0; i < I; i++) {
        pro[i].id = i;
    }
    for (i = 0; i < K; i++) {
        item[i].id = i;
        item[i].status = 0;
    }
    for (i = 0; i < N; i++) {
        buffer[i].id = i;
        buffer[i].itemId = -1; // means the buffer is empty
    }
    for (i = 0; i < I; i++) {
        pthread_create(&proThread[i], NULL, produce, &pro[i]);
    }
    for (i = 0; i < J; i++) {
        pthread_create(&conThread[i], NULL, consume, &con[i]);
    }
    for (i = 0; i < I; i++) {
        pthread_join(proThread[i], NULL);
    }
    for (i = 0; i < J; i++) {
        pthread_join(conThread[i], NULL);
    }
    pthread_cond_destroy(&prod);
    pthread_cond_destroy(&cons);
    pthread_mutex_destroy(&mutex);
    fclose(pFile);
    return 0;
}
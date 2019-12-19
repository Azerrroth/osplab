#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>  
#include <unistd.h>  


pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex;

void *ThreadFunc(void *arg) {  
  
  pthread_mutex_lock(&mutex);  
  /*此处等待唤醒*/
  int i = -1;
  printf("Start to sleep, i = %d From:%d\n", i, pthread_self());
  i = pthread_cond_wait(&cond, &mutex); // 进入wait后就会释放锁
  printf("Thread sleeping...Return:%d\n",i);
    
  /*唤醒成功*/    
  printf("Thread awakened! Return:%d From:%d\n", pthread_mutex_unlock(&mutex), pthread_self());

  printf("Thread awakened! Return:%d From:%d\n",pthread_mutex_unlock(&mutex), pthread_self());

  return NULL;
}

int main(void) {

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  pthread_t tid;
  pthread_create(&tid, NULL, ThreadFunc, NULL);  

  /*等待5秒再唤醒，方便观察*/
  sleep(5);

  printf("Main thread Lock mutex. Return:%d From:%d\n",  pthread_mutex_lock(&mutex), pthread_self());
  /*唤醒*/
    printf("Before unlock \n");
  pthread_mutex_unlock(&mutex);
  printf("After Unlock\n");

  pthread_cond_signal(&cond);
  pthread_join(tid, NULL);
  return 0;
}

#ifndef __DASTRS__
#define __DASTRS__

#include <hiredis.h>/*hiredis*/

#define DASTRS_MAJOR 0
#define DASTRS_MINOR 1
#define DASTRS_PATCH 1
#define DASTRS_SONAME 0.1

/*dastrsContext flags definition*/
#define DASTRS_BLOCK 0x1


/*dastrs type*/
typedef enum{
    DASTRS_QUEUE_E,/*queue*/
    DASTRS_HMAP_E,/*hash map set*/
    DASTRS_SET_E,/*set*/
    DASTRS_SSET_E,/*sorted set*/
    DASTRS_TYPE_NUM
}dastrsDataType;

/*Async Reply callback function*/
typedef void (*dastrsCallbackfn)(void *handler, void *reply, void *privdata);

typedef struct dastrsContext {
    void* rc;/*Async or Sync*/
    dastrsDataType data_type;
    int flags;/*bitmap*/
}dastrsContext;

/*Share*/
int dastrsInit(
        dastrsContext *dastrs, 
        dastrsDataType data_type, 
        bool is_block, 
        dastrsCallbackfn fn
        );

int dastrsDestroy(dastrsContext *ds);

typedef dastrsData {
    char *data;
    int dalen;
}dastrsData;

typedef multiDastrData {
    int num;
    dastrsData *multi;
}multiDastrData;

/*Queue*/

/*
 * Get Data from Queue by Blocking mode
 */ 
int dastrsBLGetQueue(dastrsContext *ds, int timeout, char *tbname, char *data, int dalen);/*BLPOP*/
int dastrsBRGetQueue(dastrsContext *ds, int timeout, char *tbname, char *data, int dalen);/*BRPOP*/

/*
 * Get Data from Queue by non-Blocking mode
 *
 * NOTE:ds MUST in non-Blocking mode, 
 * AND callbackFunc MUST supported 
 * by Call dastrsIinit()
 */
int dastrsLGetQueue(dastrsContext *ds, char *tbname, char* data, int dalen);/*LPOP*/
int dastrsRGetQueue(dastrsContext *ds, char *tbname, char* data, int dalen);/*RPOP*/


int dastrsLAddQueue(dastrsContext *ds, char *tbname, char *data, int dalen);/*LPUSH*/
int dastrsRAddQueue(dastrsContext *ds, char *tbname, char *data, int dalen);/*RPUSH*/

int dastrsMultiLAddQueue(dastrsContext *ds, char* tbname, multiDastrData *data);/*LPUSH*/
int dastrsMultiRAddQueue(dastrsContext *ds, char* tbname, multiDastrData *data);/*RPUSH*/

int dastrsGetQueueSize(dastrsContext *ds);/*LLEN*/

int dastrsResetQueue(dastrsContext *ds);/*LTRIM*/

typedef enum {
    FILEDTYPE_INT,
    FILEDTYPE_STRING,
}HmSetFiledType;

typedef struct {
    char filed[FILED_LEN + 1];/*MUst end of '\0'*/
    HmSetFiledType filedType;
    char* data;
}HmSetFiled;

typedef struct {
    int filedNum;
    HmSetFiled *array;
}HmSetItem;

typedef struct {
    HmSetFiled is_lock;/*1:lock, 0:unlock*/
    HmSetFiled lock_timestamp;/*上锁时间戳，预防锁被长时间占用*/
    HmSetFiled lock_expired;/*锁过期时间*/
    HmSetFiled data_timestamp;/*数据上次更新时间*/
    HmSetFiled data_expired;/*数据过期时间*/

    HmSetFiled data;/*业务数据*/
}HmSetItemTemplate;

/*HMset*/
int dastrsAddHMset(dastrsContext *ds, char* tbname, HmSetItem* item);/*HMSET*/

/*NOTE:
 *
 * int setWLock//MULTI:HEXISTS:HSET:EXEC
 *
 * 写加锁步骤:
 *
 * 2.HSETNX:WLock filed
 *      2.1:Succ:数据块加锁
 *      2.2:Fail:数据快加锁失败,自旋，尝试获得写锁，或者超时退出
 *
 * 3.HDEL:WLock filed,解锁
 */
int dastrsAddFiledHMset(dastrsContext *ds, char* tbname, HmSetFiled *filed, int WLock);/*HSET*/

/*
 *
 *NOTE:
 *1.读取数据之前需要测试WLock是否有效，如果存在写锁则不能读，反之可以.
 *
 *2.item如何释放内存
 */
int dastrsGetHMset(
        dastrsContext *ds, 
        char* tbname, 
        HmSetItem* item, 
        int getAll, 
        );/*Part:HMGET, All:HVALS or HGETALL*/

int dastrsDelHMset(dastrsContext *ds, char* tbname);/*DEL*/

/*Pub/Sub*/
#endif


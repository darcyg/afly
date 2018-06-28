#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#include "linkkit.h"
#include "nxp.h"
#include "cJSON.h"

#define NELEMS(x)   (sizeof(x) / sizeof((x)[0]))

#define DPRINT(...)                                      \
do {                                                     \
    printf("\033[1;31;40m%s.%d: ", __func__, __LINE__);  \
    printf(__VA_ARGS__);                                 \
    printf("\033[0m");                                   \
} while (0)

#define LOCK_MAX_NUM         (1)

typedef struct {
    int  devid;
    char productKey[32];
    char deviceName[64];
    char deviceSecret[64];

    int battery;
    int online;
    char type[8];
    char version[8];
    char model[8];
} lock_t;

typedef struct {
    char *productKey;
    char *deviceName;
    char *deviceSecret;
} lock_conf_t;

static const lock_conf_t lock_maps[] = {
    {"a1eOQheUHpW", "00158d0000b4716f", "Lmh7xC0kdnHGsk4KHxq96voRLRJBUfg3"},
};

static lock_t *locks[LOCK_MAX_NUM];

static int lock_get_property(char *in, char *out, int out_len, void *ctx)
{
    DPRINT("in: %s\n", in);

    lock_t *lock = ctx;

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    int iSize = cJSON_GetArraySize(rJson);
    if (iSize <= 0) {
        cJSON_Delete(rJson);
        return -1;
    }

    cJSON *pJson = cJSON_CreateObject();
    if (!pJson) {
        cJSON_Delete(rJson);
        return -1;
    }

    int i;
    for (i = 0; i < iSize; i++) {
        cJSON *pSub = cJSON_GetArrayItem(rJson, i);

        if (strcmp(pSub->valuestring, "LockType") == 0) {
            cJSON_AddStringToObject(pJson, "LockType", lock->type);
        } else if (strcmp(pSub->valuestring, "LockVersion") == 0) {
            cJSON_AddStringToObject(pJson, "LockVersion", lock->version);
        } else if (strcmp(pSub->valuestring, "LockModel") == 0) {
            cJSON_AddStringToObject(pJson, "LockModel", lock->model);
        }
    }

    char *p = cJSON_PrintUnformatted(pJson);
    if (!p) {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        return -1;
    }

    if (strlen(p) >= out_len) {
        cJSON_Delete(rJson);
        cJSON_Delete(pJson);
        free(p);
        return -1;
    }

    strcpy(out, p);

    DPRINT("out: %s\n", out);

    cJSON_Delete(rJson);
    cJSON_Delete(pJson);
    free(p);

    return 0;
}

static int lock_set_property(char *in, void *ctx)
{
    lock_t *lock = ctx;

    DPRINT("%s.%s: in %s\n", lock->productKey, lock->deviceName, in);

    cJSON *rJson = cJSON_Parse(in);
    if (!rJson)
        return -1;

    cJSON *LockType = cJSON_GetObjectItem(rJson, "LockType");
    if (LockType)
        strcpy(lock->type, LockType->valuestring);

    cJSON *LockVersion = cJSON_GetObjectItem(rJson, "LockVersion");
    if (LockVersion)
        strcpy(lock->version, LockVersion->valuestring);

    cJSON *LockModel = cJSON_GetObjectItem(rJson, "LockModel");
    if (LockModel)
        strcpy(lock->model, LockModel->valuestring);

    cJSON_Delete(rJson);

    linkkit_gateway_post_property_json_sync(lock->devid, in, 10000);

    return 0;
}

static int lock_call_service(char *identifier, char *in, char *out, int out_len, void *ctx)
{
    lock_t *lock = ctx;

    DPRINT("%s.%s: in %s\n", lock->productKey, lock->deviceName, in);
    
    linkkit_gateway_post_property_json_sync(lock->devid, "{\"SetTimer\": \"hello, world!\"}", 5000);

    return 0;
}

static linkkit_cbs_t lock_cbs = {
    .get_property = lock_get_property,
    .set_property = lock_set_property,
    .call_service = lock_call_service,
};

int lock_init(void)
{
    int i;
    for (i = 0; i < LOCK_MAX_NUM; i++) {
        lock_t *lock = malloc(sizeof(lock_t));
        if (!lock)
            break;
        
        memset(lock, 0, sizeof(lock_t));

        const lock_conf_t *conf = &lock_maps[i];

        strncpy(lock->productKey,   conf->productKey,   sizeof(lock->productKey) - 1);
        strncpy(lock->deviceName,   conf->deviceName,   sizeof(lock->deviceName) - 1);
        strncpy(lock->deviceSecret, conf->deviceSecret, sizeof(lock->deviceSecret) - 1);

        if (linkkit_gateway_subdev_register(lock->productKey, lock->deviceName, lock->deviceSecret) < 0) {
            free(lock);
            break;
        }

        lock->devid = linkkit_gateway_subdev_create(lock->productKey, lock->deviceName, &lock_cbs, lock);
        if (lock->devid < 0) {
            DPRINT("linkkit_gateway_subdev_create %s<%s> failed\n", lock->deviceName, lock->productKey);
            linkkit_gateway_subdev_unregister(lock->productKey, lock->deviceName);
            free(lock);
            break;
        }

        if (linkkit_gateway_subdev_login(lock->devid) < 0) {
            DPRINT("linkkit_gateway_subdev_login %s<%s> failed\n", lock->deviceName, lock->productKey);
            linkkit_gateway_subdev_destroy(lock->devid);
            linkkit_gateway_subdev_unregister(lock->productKey, lock->deviceName);
            free(lock);
            break;
        }

        locks[i] = lock;
    }

    return 0;
}

int lock_exit(void)
{
    int i;
    for (i = 0; i < LOCK_MAX_NUM; i++) {
        lock_t *lock = locks[i];
        if (!lock)
            continue;

        linkkit_gateway_subdev_logout(lock->devid);
        linkkit_gateway_subdev_destroy(lock->devid);
        linkkit_gateway_subdev_unregister(lock->productKey, lock->deviceName);
        free(lock);

        locks[i] = NULL;
    }

    return 0;
}


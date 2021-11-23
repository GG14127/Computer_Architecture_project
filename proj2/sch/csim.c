
/*
 *
 * Student Name: Chenhao Shi
 * Student ID: 519021910434
 *
 */

/*
 *
 * BLOCK[I]->SET  :  S
 * BLOCK[I][J]->A LINE  :  E
 *
 * 刚开始使用CACHE->SET->LINE三重结构,但由于SET中LINELIST的大小随E值改变,未能找到较为方便的初始化方法
 * 所以改为使用二维数组,每行代表一个set
 * 避免了原先在CACHE初始化时由于 SET大小补丁引起的SEGMENTATION FAULT
 *
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

static int miss = 0, hit = 0, eviction = 0;
struct Block {
    int valid;
    int tag;
    int LRU;
};
struct Cache {
    int linenum;
    int setnum;
    int blockbyte;
    struct Block **cache;
};

int str2int(char *str) {
    int len = strlen(str);
    int res = 0;
    for (int i = 0; i < len; i++)
        res = res * 10 + str[i] - '0';
    return res;
}

long hex2dec(char *str) {
    int len = strlen(str);
    long res = 0;
    for (int i = 0; i < len; i++) {
        if (str[i] >= '0' && str[i] <= '9')
            res = res * 16 + str[i] - '0';
        if (str[i] >= 'a' && str[i] <= 'f')
            res = res * 16 + str[i] - 'a' + 10;
        if (str[i] >= 'A' && str[i] <= 'F')
            res = res * 16 + str[i] - 'A' + 10;
    }
    return res;
}

void initiate_cache(int S, int E, int B, struct Cache *c) {
    c->linenum = E;
    c->setnum = S;
    c->blockbyte = B;
    c->cache = (struct Block **) malloc(sizeof(struct Block *) * S);
    for (int i = 0; i < S; i++) {
        c->cache[i] = (struct Block *) malloc(sizeof(struct Block) * E);
        for (int j = 0; j < E; j++) {
            struct Block *p = (struct Block *) malloc(sizeof(struct Block));
            c->cache[i][j] = *p;
            c->cache[i][j].valid = 0;
            c->cache[i][j].tag = 0;
            c->cache[i][j].LRU = 0;
        }
    }
}

//记录命令中的变量
void analysis(char *record, char *op, char *add, char *size) {
    int i = 0, j = 0;
    while (record[i] == ' ') i++;
    *op = record[i];
    i++;//option
    for (; record[i] != ','; i++, j++)
        add[j] = record[i];
    add[j] = '\0';
    j = 0;
    i++;
    for (; record[i] != '\n'; i++, j++)
        size[j] = record[i];
    size[j] = '\0';
}

// 执行命令
void operation(struct Cache *c, char *add, char *size, int rec) {
    long address = hex2dec(add);
    int sign = (address / c->blockbyte) % (c->setnum); //第几组
    long curtag = (address / c->blockbyte) / (c->setnum); //第几行
    int empty = -1;
    for (int i = 0; i < c->linenum; i++) {
        if (c->cache[sign][i].valid == 0)
            empty = i;
        if (c->cache[sign][i].valid == 1 && c->cache[sign][i].tag == curtag) { //找到匹配行,处理结束
            hit++;
            c->cache[sign][i].LRU = rec;
            return;
        }
    }
    miss++;
    if (empty >= 0 && empty < c->linenum) { //miss但有空行, cold miss
        c->cache[sign][empty].tag = curtag;
        c->cache[sign][empty].valid = 1;
        c->cache[sign][empty].LRU = rec;
        return;
    } else {  //无空行 eviction
        eviction++;
        int minrec = c->cache[sign][0].LRU;
        int del = 0;

        //找到最小的操作次数
        for (int i = 1; i < c->linenum; i++)
            if (c->cache[sign][i].LRU < minrec) {
                minrec = c->cache[sign][i].LRU;
                del = i;
            }
        c->cache[sign][del].tag = curtag; //load
        c->cache[sign][del].LRU = rec;  //更新当前操作的次数
    }
    return;
}


int main(int argc, char *argv[]) {
    struct Cache cache;
    int S, E, B;
    int s, b, count = 0;
    char buf[50];
    FILE *fp = NULL;
    for (int i = 0; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 's') {
                i++;
                s = str2int(argv[i]);
                S = 1 << s;
                i++;
            }
            if (argv[i][1] == 'E') {
                i++;
                E = str2int(argv[i]);
                i++;
            }
            if (argv[i][1] == 'b') {
                i++;
                b = str2int(argv[i]);
                B = 1 << b;
                i++;
            }
            if (argv[i][1] == 't') {
                i++;
                if ((fp = fopen(argv[i], "r")) == NULL) {
                    printf("openfile %s error", argv[i]);
                    exit(1);
                }
            }
        }
    }
    if (s <= 0 || E <= 0 || b <= 0) return -1;
    initiate_cache(S, E, B, &cache);
    while (fgets(buf, sizeof(buf), fp)) {
        count++;
        char op;
        char add[50];
        char size[50];
        analysis(buf, &op, add, size);
        if (op == 'I') continue;
        operation(&cache, add, size, count);
        if (op == 'M') operation(&cache, add, size, count); //"M" 操作做两次
    }
    printSummary(hit, miss, eviction);
    return 0;
}
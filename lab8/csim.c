/*
 *
 * Student Name: 黎君
 * Student ID: 516030910233
 *
 */
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define SET(addr, B, S) ((addr/B)%S);
#define TAG(addr, B, S) ((addr/B)/S);

typedef struct line_t {
	int valid;
	int tag;
} Line;

void initLine(Line* line)
{
	line->valid = 0;
	line->tag = 0;
}

void load(Line* line, int tag)
{
	line->valid = 1;
	line->tag = tag;
}

int lineMatch(Line* line, int tag)
{
	if (line->valid == 0) return 0;
	if (line->tag == tag) {
		return 1;
	} else {
		return 0;
	}
}

typedef struct LRU_t {
	int index;
	struct LRU_t* next;
} LRUNode;

LRUNode* newNode(int idx)
{
	LRUNode* p = (LRUNode*) malloc(sizeof(LRUNode));
	p->index = idx;
	p->next = NULL;
	return p;
}

typedef struct LRUList_t {
	LRUNode* head;
	LRUNode* tail;
} LRUList;

void initLRUList(LRUList* lru)
{
	lru->head = NULL;
	lru->tail = NULL;
}

void insert(LRUList* lru, int idx)
{
	LRUNode* p = newNode(idx);
	if (lru->head == NULL) {
		lru->head = lru->tail = p;
	} else {
		lru->tail->next = p;
		lru->tail = p;
	}
}

void deleteNode(LRUList* lru, int idx)
{
	LRUNode* prev = lru->head;
	LRUNode* p = prev->next;

	if (lru->head->index == idx) {
		if (lru->head == lru->tail) {
			lru->tail = p;
		}
		free(lru->head);
		lru->head = p;
	}

	while (p != NULL) {
		if (p->index == idx) {
			if (p == lru->tail) {
				lru->tail = prev;
			}
			prev->next = p->next;
			free(p);
		}
		prev = p;
		p = p->next;
	}
}

int LRUidx(LRUList* lru)
{
	if (lru->head == NULL) {
		return -1;
	}
	int idx = lru->head->index;
	LRUNode* nextHead = lru->head->next;
	if (lru->head == lru->tail) {
		lru->tail = nextHead;
	}
	free(lru->head);
	lru->head = nextHead;
	return idx;
}

typedef struct set_t {
	int E;
	Line* lines;
	int isFull;
	LRUList* lru;
} Set;

void initSet(Set* set, int E)
{
	Line* lines = (Line*)malloc(sizeof(Line)*E);
	for (int i = 0; i < E; i++) {
		initLine(lines + i);
	}
	LRUList* lru = (LRUList*)malloc(sizeof(LRUList));
	initLRUList(lru);
	set->E = E;
	set->lines = lines;
	set->lru = lru;
	set->isFull = 0;
}

/*
 * findLine - find next invalid line in the set
 */
int findLine(Set* set)
{
	int n = set->E;
	Line* lines = set->lines;
	for (int i = 0; i < n; i++) {
		if ((lines+i)->valid == 0) {
			return i;
		}
	}
	return -1;
}

typedef enum Msg_t {HIT, COLD, CONFLICT} Msg;

Msg selectSet(Set* set, int tag)
{
	int n = set->E;
	Line* lines = set->lines;
	LRUList* lru = set->lru;

	/* hit */
	for (int i = 0; i < n; i++) {
		if (lineMatch(lines+i, tag)) {
			deleteNode(lru, i);
			insert(lru, i);
			return HIT;
		}
	}

	/* cold miss */
	if (set->isFull == 0) { // not full
		int idx = findLine(set);
		load(lines+idx, tag);
		insert(lru, idx);
		if (idx == n - 1) {
			set->isFull = 1;
		}
		return COLD;
	}

	/* conflict miss */
	/* evict LRU */
	int idx = LRUidx(lru);
	load(lines+idx, tag);
	insert(lru, idx);
	return CONFLICT;
}

typedef struct cache_t {
	int B;
	int S;
	Set* sets;
} Cache;

void initCache(Cache* cache, int B, int E, int S)
{
	Set* sets = (Set*) malloc(sizeof(Set) * S);
	for (int i = 0; i < S; i++) {
		initSet(sets + i, E);
	}
	cache->sets = sets;
	cache->B = B;
	cache->S = S;
}

int locate(Cache* cache, unsigned int addr)
{
	int set = SET(addr, cache->B, cache->S);
	int tag = TAG(addr, cache->B, cache->S);
	return selectSet(cache->sets + set, tag);
}

int main(int argc,char *argv[])
{
	FILE* trace_file;
    const char* optstring = "s:E:b:t:";
    char opt;
	int S, E, B;
	int hasOpt = 0;
	while((opt = getopt(argc,argv,optstring)) != -1) {
		hasOpt = 1;
        switch (opt) {
            case 's':
                S = 1 << atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                B = 1 << atoi(optarg);
                break;
            case 't':
                trace_file = fopen(optarg, "r");
                break;
            default:
                return 0;
        }
    }    

	if (hasOpt == 0) {
		return 0;
	}

	Cache* cache = (Cache*) malloc(sizeof(Cache));
	initCache(cache, B, E, S);

	int hitCount = 0;
	int missCount = 0;
	int evictionCount = 0;

	char id;
	unsigned int addr;
	int size;
	while (fscanf(trace_file, " %c %x,%d", &id, &addr, &size) > 0) {
		int t;
		if (id == 'L' || id == 'S') {
			t = 1;
		} else if (id == 'M') {
			t = 2;
		} else {
			continue;
		}
		for (int i = 0; i < t; i++) {
			Msg m = locate(cache, addr);
			switch (m) {
				case HIT:
					hitCount += 1;
					break;
				case COLD:
					missCount += 1;
					break;
				case CONFLICT:
					missCount += 1;
					evictionCount += 1;
					break;
				default:
					break;
			}
		}
	}

    printSummary(hitCount, missCount, evictionCount);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DEBUG
#define COMMAND_NUM 320 // 虚拟页装满的指令个数
#define PAGE_CONTENT 10 // 虚拟页存放的指令个数

// 指令集合，假设一虚拟页中存放10条指令，虚拟页数为32，物理页数4-32
typedef struct commandList {
    int command; // 10条指令的集合代表所在的虚拟页页号
    int offset; // 指令偏移地址
    int lastTimeHit; // 最后一次命中的时间
    int enterNum; // 模拟进入的顺序
}commandList;

// 物理内存页
typedef struct physicalPage {
    int command; // 装入的指令集，-1代表没有装入
}physicalPage;

// 物理页数
int physicalPageNum;
// 虚拟页，int存储的是虚拟页中的指令，该指令随机产生
int* mVirtualPages;
// 物理页
physicalPage* mPhysicalPage;
// 指令集
commandList* mCommandList;
// 缺页次数
int disaffect = 0;
// 缺页率
float disaffectRate;

// 初始化虚拟页
void initVirtualPage(int* vp) {
    srand((unsigned)time(NULL));

    for (int i = 0; i < COMMAND_NUM; i++) {
        vp[i] = rand() % COMMAND_NUM;
    }
}

// 初始化指令集合
void initCommandList(commandList* cl) {
    for (int i = 0; i < COMMAND_NUM; i++) {
        (cl + i)->lastTimeHit = -1;
        (cl + i)->command = i / PAGE_CONTENT;
        (cl + i)->offset = i % PAGE_CONTENT;
        (cl + i)->enterNum = 0;
    }
}

// 初始化物理内存页
void initPhysicalPage(physicalPage* pp) {
    for (int i = 0; i < physicalPageNum; i++) {
        (pp + i)->command = -1;
    }
}

// 是否命中，即是否存在物理页中
int isExistInPhysicalPage(int command, physicalPage* pp, int length) {
    for (int i = 0; i < length; i++) {
        if ((pp + i)->command == command)
            return i;
    }
    return -1;
}

// FIFO策略淘汰的页号
int whichToSelectFIFO(physicalPage *pp, int length) {
    int returnValue = 0;
    int target = (mCommandList + pp->command * PAGE_CONTENT)->enterNum;

    for (int i = 0; i < length; i++) {
        // 若物理内存为空，则直接返回该位置
        if ((pp + i)->command == 1) {
            return i;
        } else {
            // 若不为空，则返回最先进入内存的页面
            int temp = (mCommandList + (pp + i)->command * PAGE_CONTENT)->enterNum;
            if (temp < target) {
                target = temp;
                returnValue = i;
            }
        }
    }
    return returnValue;
}

// 模拟FIFO访问内存
void FIFO() {
    for (int i = 0; i < COMMAND_NUM; i++) {
        int command = *(mVirtualPages + i) / PAGE_CONTENT;
#ifdef DEBUG
        printf("---------------------------------------------------------------------------\n");
        printf("指令%d请求分配内存,", *(mVirtualPages + i));
#endif
        // 是否存在物理页，若存在，返回所在位置，否则返回-1
        int index = isExistInPhysicalPage(command, mPhysicalPage, physicalPageNum);
        // 若不存在
        if (index == -1) {
            disaffect++;
            // 模拟执行装入内存
            int targetIndex = whichToSelectFIFO(mPhysicalPage, physicalPageNum);
            // 写入
            (mPhysicalPage + targetIndex)->command = command;
#ifdef DEBUG
            printf("物理内存%d上的指令被淘汰,取而代之的是%d\n", targetIndex, *(mVirtualPages + i));
#endif 
            //更新使用情况
            (mCommandList + command * PAGE_CONTENT)->enterNum = i + 1;
            (mCommandList + command * PAGE_CONTENT)->lastTimeHit = i;
        } else {
            // 直接更新
#ifdef DEBUG
            printf("该指令已经存在内存中！\n");
#endif
            (mCommandList + command * PAGE_CONTENT)->lastTimeHit = i;
        }
#ifdef DEBUG
        printf("内存使用情况:\n");
        //输出内存使用情况
        for (int k = 0; k < physicalPageNum; k++) {
            printf(" %d ", (mPhysicalPage + k)->command);
        }
        printf("\n");
#endif
    }
}

// LRU淘汰策略
int whichToSelectLRU(physicalPage* pp, int length) {
    int returnValue = 0;
    int target = (mCommandList + pp->command * PAGE_CONTENT)->lastTimeHit;
    for (int i = 0; i < length; i++) {
        // 若物理位置为空，则直接返回该位置
        if ((pp + i)->command == -1) {
            return i;
        } else {
            // 若不为空，则返回最先进入内存的页面
            int temp = (mCommandList + (pp + i)->command * PAGE_CONTENT)->lastTimeHit;
            if (temp < target) {
                target = temp;
                returnValue = i;
            }
        }
    }
    return returnValue;
}

// 模拟LRU分配内存
void LRU() {
    for (int i = 0; i < COMMAND_NUM; i++) {
        int command = *(mVirtualPages + i) / PAGE_CONTENT;
#ifdef DEBUG
        printf("---------------------------------------------------------------------------\n");
        printf("指令%d请求分配内存,", *(mVirtualPages + i));
#endif
        // 是否存在物理页，若存在，返回所在位置，否则返回-1
        int index = isExistInPhysicalPage(command, mPhysicalPage, physicalPageNum);
        // 若不存在
        if (index == -1) {
            disaffect++;
            // 模拟执行装入内存
            int targetIndex = whichToSelectLRU(mPhysicalPage, physicalPageNum);
            // 写入
            (mPhysicalPage + targetIndex)->command = command;
#ifdef DEBUG
            printf("物理内存%d上的指令被淘汰,取而代之的是%d\n", targetIndex, *(mVirtualPages + i));
#endif
            //更新使用情况
            (mCommandList + command * PAGE_CONTENT)->enterNum = i + 1;
            (mCommandList + command * PAGE_CONTENT)->lastTimeHit = i;
        } else {
#ifdef DEBUG
            printf("该指令已经存在内存中！\n");
#endif
            (mCommandList + command*PAGE_CONTENT)->lastTimeHit = i;
        }
#ifdef DEBUG
        printf("内存使用情况:\n");
        //输出内存使用情况
        for (int k = 0; k < physicalPageNum; k++) {
            printf(" %d ", (mPhysicalPage + k)->command);
        }
        printf("\n");
#endif
    }
}

int main() {
    while (1) {
        while (1) {
            printf("请输入物理页数(范围4~32):");
            scanf("%d", &physicalPageNum);
            if (physicalPageNum < 4 ||  physicalPageNum > 32) {
                printf("请输入大于4 小于32的整数");
            } else {
                break;
            }
        }

        mPhysicalPage = (physicalPage*)malloc(physicalPageNum * sizeof(physicalPage));
        // 模拟32个不同的指令集合
        mCommandList = (commandList*)malloc(COMMAND_NUM * sizeof(commandList));
        mVirtualPages = (int*)malloc(COMMAND_NUM * sizeof(int));
        // FIFO
        initCommandList(mCommandList);
        initPhysicalPage(mPhysicalPage);
        initVirtualPage(mVirtualPages);
        disaffect = 0;
        disaffect = 0;
        printf("FIFO策略:\n");
        FIFO();
        disaffectRate = (float)disaffect / (float)COMMAND_NUM;
        printf("命中率为%f\n", 1 - disaffectRate);

        // LRU
        initCommandList(mCommandList);
        initPhysicalPage(mPhysicalPage);
        disaffect = 0;
        printf("\n\nLRU策略:\n");
        LRU();
        disaffectRate = (float)disaffect / (float)COMMAND_NUM;
        printf("命中率为%f\n", 1 - disaffectRate);
        printf("\n");
    }
}
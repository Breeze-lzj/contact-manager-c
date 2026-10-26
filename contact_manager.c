#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INIT_CAPACITY 20
#define GROUP_COUNT 6
#define INITIAL_COUNT 27
#define BUF_LEN 256

// 联系人结构体
typedef struct {
    char name[50];        // 姓名
    char phone[20];       // 手机号
    char tel[20];         // 固话
    char email[50];       // 邮箱
    char company[50];     // 公司
    char position[30];    // 职位
    int groupId;          // 1亲属 2同事 3同学 4领导 5朋友
    char initial;         // 拼音首字母
    char remark[100];     // 备注
} Contact;

// 个人名片结构体
typedef struct {
    char name[50];
    char phone[20];
    char tel[20];
    char email[50];
    char company[50];
    char position[30];
    char address[100];
    char wechat[30];
    char qq[20];
    char bio[200];
} MyCard;

// 链表节点
typedef struct Node {
    Contact data;
    struct Node *next;
} Node, *LinkList;

// 通讯录总管理结构
typedef struct {
    Contact* contacts;
    int count;
    int capacity;
    LinkList initialIndex[INITIAL_COUNT]; // 声母索引 # A-Z
    LinkList groupIndex[GROUP_COUNT];     // 群组索引 1~5
} ContactList;

// ==================== 通用工具 ====================
void clearEnter() {
    char ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
}

void pauseReturn(void) {
    printf("\n按回车键返回上一级菜单...");
    clearEnter();
    getchar();
}

void InitLinkList(LinkList* head) {
    *head = (Node*)malloc(sizeof(Node));
    if (*head == NULL) { printf("链表内存分配失败！\n"); exit(1); }
    (*head)->next = NULL;
}

void addToLinkList(LinkList* head, Contact* c) {
    Node* p = *head;
    while (p->next != NULL) p = p->next;
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) { printf("节点内存分配失败！\n"); exit(1); }
    newNode->data = *c;
    newNode->next = NULL;
    p->next = newNode;
}

void freeLinkList(LinkList* head) {
    if (*head == NULL) return;
    Node* p = (*head)->next;
    while (p != NULL) { Node* temp = p; p = p->next; free(temp); }
    free(*head);
    *head = NULL;
}

int getInitialIndex(char ch) {
    if (ch == '#') return 0;
    ch = toupper(ch);
    if (ch >= 'A' && ch <= 'Z') return ch - 'A' + 1;
    return 0;
}

const char* getGroupName(int gid) {
    switch (gid) {
        case 1: return "亲属";
        case 2: return "同事";
        case 3: return "同学";
        case 4: return "领导";
        case 5: return "朋友";
        default: return "未分类";
    }
}

char getPinyinInitial(const char* name) {
    if (name == NULL || name[0] == '\0') return '#';
    if ((name[0] >= 'a' && name[0] <= 'z') || (name[0] >= 'A' && name[0] <= 'Z'))
        return toupper(name[0]);
    unsigned char c1 = name[0];
    unsigned char c2 = name[1];
    int code = (c1 - 160) * 100 + (c2 - 160);
    if (code >= 1601 && code <= 1636) return 'A';
    if (code >= 1637 && code <= 1832) return 'B';
    if (code >= 1833 && code <= 2077) return 'C';
    if (code >= 2078 && code <= 2273) return 'D';
    if (code >= 2274 && code <= 2301) return 'E';
    if (code >= 2302 && code <= 2431) return 'F';
    if (code >= 2432 && code <= 2593) return 'G';
    if (code >= 2594 && code <= 2786) return 'H';
    if (code >= 2787 && code <= 2801) return 'J';
    if (code >= 2802 && code <= 2948) return 'K';
    if (code >= 2949 && code <= 3085) return 'L';
    if (code >= 3086 && code <= 3304) return 'M';
    if (code >= 3305 && code <= 3491) return 'N';
    if (code >= 3492 && code <= 3633) return 'O';
    if (code >= 3634 && code <= 3722) return 'P';
    if (code >= 3723 && code <= 3825) return 'Q';
    if (code >= 3826 && code <= 3963) return 'R';
    if (code >= 3964 && code <= 4169) return 'S';
    if (code >= 4170 && code <= 4323) return 'T';
    if (code >= 4324 && code <= 4357) return 'W';
    if (code >= 4358 && code <= 4439) return 'X';
    if (code >= 4440 && code <= 4549) return 'Y';
    if (code >= 4550 && code <= 5693) return 'Z';
    return '#';
}

void rebuildIndex(ContactList* list) {
    for (int i = 0; i < INITIAL_COUNT; i++) freeLinkList(&list->initialIndex[i]);
    for (int i = 0; i < GROUP_COUNT; i++) freeLinkList(&list->groupIndex[i]);
    for (int i = 0; i < INITIAL_COUNT; i++) InitLinkList(&list->initialIndex[i]);
    for (int i = 0; i < GROUP_COUNT; i++) InitLinkList(&list->groupIndex[i]);
    for (int i = 0; i < list->count; i++) {
        int ini = getInitialIndex(list->contacts[i].initial);
        addToLinkList(&list->initialIndex[ini], &list->contacts[i]);
        int g = list->contacts[i].groupId;
        if (g >= 1 && g <= 5) addToLinkList(&list->groupIndex[g], &list->contacts[i]);
    }
}

// ==================== 通讯录功能 ====================
void InitContactList(ContactList* list) {
    list->count = 0;
    list->capacity = INIT_CAPACITY;
    list->contacts = (Contact*)malloc(list->capacity * sizeof(Contact));
    if (list->contacts == NULL) { printf("内存分配失败！\n"); exit(1); }
    for (int i = 0; i < INITIAL_COUNT; i++) InitLinkList(&list->initialIndex[i]);
    for (int i = 0; i < GROUP_COUNT; i++) InitLinkList(&list->groupIndex[i]);
}

void loadDefaultContacts(ContactList* list) {
    Contact defaults[] = {
        {"张伟", "13800138001", "010-88880001", "zhangwei@qq.com", "无", "无", 1, 'Z', "表哥，在北京工作"},
        {"李娜", "13900139002", "021-66660002", "lina@163.com", "无", "无", 1, 'L', "表姐"},
        {"王芳", "13700137003", "", "wangfang@gmail.com", "无", "无", 1, 'W', "妈妈"},
        {"刘洋", "13600136004", "0755-22220004", "liuyang@tech.com", "科技有限公司", "软件工程师", 2, 'L', "研发部同事"},
        {"陈静", "13500135005", "0755-22220005", "chenjing@tech.com", "科技有限公司", "产品经理", 2, 'C', "产品部"},
        {"杨帆", "13400134006", "", "yangfan@tech.com", "科技有限公司", "UI设计师", 2, 'Y', "设计组"},
        {"赵磊", "13300133007", "0755-22220007", "zhaolei@tech.com", "科技有限公司", "测试工程师", 2, 'Z', "质量部"},
        {"周杰", "13200132008", "", "zhoujie@edu.cn", "无", "无", 3, 'Z', "大学室友"},
        {"吴敏", "13100131009", "", "wumin@edu.cn", "无", "无", 3, 'W', "高中同学"},
        {"徐强", "13000130010", "", "xuqiang@edu.cn", "无", "无", 3, 'X', "班长"},
        {"孙总", "18900189011", "0755-88880011", "sunzong@tech.com", "科技有限公司", "技术总监", 4, 'S', "部门领导"},
        {"马经理", "18800188012", "", "majingli@tech.com", "科技有限公司", "项目经理", 4, 'M', "直属领导"},
        {"胡雨", "18700187013", "", "huge@qq.com", "自由职业", "摄影师", 5, 'H', "摄影爱好者"},
        {"邓小计", "18600186014", "", "dengchao@163.com", "健身房", "健身教练", 5, 'D', "健身认识的朋友"},
        {"范拉卡", "18500185015", "", "fanbb@gmail.com", "设计工作室", "设计师", 5, 'F', "设计圈好友"}
    };
    int num = sizeof(defaults) / sizeof(defaults[0]);
    for (int i = 0; i < num; i++) addContact(list, &defaults[i]);
    printf("已加载 %d 条默认联系人数据\n", num);
}

int inputContact(Contact* c) {
    char buf[BUF_LEN];
    memset(c, 0, sizeof(Contact));
    printf("请输入姓名："); scanf("%49s", c->name); clearEnter();
    printf("请输入手机号："); scanf("%19s", c->phone); clearEnter();
    printf("请输入固话："); scanf("%19s", c->tel); clearEnter();
    printf("请输入邮箱："); scanf("%49s", c->email); clearEnter();
    printf("请输入公司："); scanf("%49s", c->company); clearEnter();
    printf("请输入职位："); scanf("%29s", c->position); clearEnter();
    printf("选择群组(1亲属 2同事 3同学 4领导 5朋友)："); scanf("%d", &c->groupId); clearEnter();
    printf("请输入备注：");
    fgets(buf, BUF_LEN, stdin);
    buf[strcspn(buf, "\n")] = '\0';
    strncpy(c->remark, buf, sizeof(c->remark)-1);
    c->initial = getPinyinInitial(c->name);
    return 0;
}

int addContact(ContactList* list, Contact* c) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        Contact* temp = (Contact*)realloc(list->contacts, list->capacity * sizeof(Contact));
        if (temp == NULL) { printf("内存扩容失败！\n"); return -1; }
        list->contacts = temp;
    }
    list->contacts[list->count] = *c;
    int idx = list->count;
    list->count++;
    int iniIdx = getInitialIndex(c->initial);
    addToLinkList(&list->initialIndex[iniIdx], &list->contacts[idx]);
    if (c->groupId >= 1 && c->groupId <= 5)
        addToLinkList(&list->groupIndex[c->groupId], &list->contacts[idx]);
    return 0;
}

void showAllContacts(ContactList* list) {
    printf("\n===== 所有联系人（按声母分组）=====\n");
    printf("总联系人数量：%d 人\n\n", list->count);
    char head[] = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    for (int i = 0; i < INITIAL_COUNT; i++) {
        LinkList p = list->initialIndex[i]->next;
        if (p == NULL) continue;
        printf("【%c 分组】\n", head[i]);
        int num = 0;
        while (p != NULL) {
            printf(" %d. 姓名:%-12s 手机号:%-15s 群组:%s\n",
                ++num, p->data.name, p->data.phone, getGroupName(p->data.groupId));
            p = p->next;
        }
        printf("\n");
    }
}

void showByGroup(ContactList* list, int gid) {
    if (gid < 1 || gid > 5) { printf("群组编号错误！\n"); return; }
    printf("\n===== %s 群组联系人 =====\n", getGroupName(gid));
    LinkList p = list->groupIndex[gid]->next;
    int cnt = 0;
    while (p != NULL) {
        cnt++;
        printf(" %d. %s | %s\n", cnt, p->data.name, p->data.phone);
        p = p->next;
    }
    if (cnt == 0) printf("该群组暂无联系人\n");
}

void searchByName(ContactList* list, char* key) {
    printf("\n===== 姓名查询结果 =====\n");
    int find = 0;
    for (int i = 0; i < list->count; i++) {
        if (strstr(list->contacts[i].name, key) != NULL) {
            find++;
            Contact c = list->contacts[i];
            printf("姓名:%s 手机:%s 群组:%s 备注:%s\n",
                c.name, c.phone, getGroupName(c.groupId), c.remark);
        }
    }
    if (find == 0) printf("未匹配到联系人\n");
}

void searchByPhone(ContactList* list, char* key) {
    printf("\n===== 手机号查询结果 =====\n");
    int find = 0;
    for (int i = 0; i < list->count; i++) {
        if (strcmp(list->contacts[i].phone, key) == 0) {
            find = 1;
            Contact c = list->contacts[i];
            printf("姓名:%s 手机:%s 固话:%s 邮箱:%s 公司:%s 群组:%s\n",
                c.name, c.phone, c.tel, c.email, c.company, getGroupName(c.groupId));
            printf("职位:%s 备注:%s\n", c.position, c.remark);
            break;
        }
    }
    if (find == 0) printf("无此手机号联系人\n");
}

int deleteContact(ContactList* list, char* phone) {
    int pos = -1;
    for (int i = 0; i < list->count; i++)
        if (strcmp(list->contacts[i].phone, phone) == 0) { pos = i; break; }
    if (pos == -1) { printf("未找到该联系人，删除失败\n"); return -1; }
    for (int i = pos; i < list->count - 1; i++)
        list->contacts[i] = list->contacts[i + 1];
    list->count--;
    rebuildIndex(list);
    printf("联系人删除成功\n");
    return 0;
}

int modifyContact(ContactList* list, char* phone) {
    int pos = -1;
    for (int i = 0; i < list->count; i++)
        if (strcmp(list->contacts[i].phone, phone) == 0) { pos = i; break; }
    if (pos == -1) { printf("未找到联系人\n"); return -1; }
    Contact temp;
    printf("请重新录入新信息：\n");
    inputContact(&temp);
    list->contacts[pos] = temp;
    rebuildIndex(list);
    printf("信息修改完成\n");
    return 0;
}

void sortContacts(ContactList* list, int type) {
    for (int i = 0; i < list->count - 1; i++)
        for (int j = 0; j < list->count - 1 - i; j++) {
            int swap = 0;
            if (type == 1) {
                if (strcmp(list->contacts[j].name, list->contacts[j+1].name) > 0) swap = 1;
            } else {
                if (strcmp(list->contacts[j].phone, list->contacts[j+1].phone) > 0) swap = 1;
            }
            if (swap) {
                Contact t = list->contacts[j];
                list->contacts[j] = list->contacts[j+1];
                list->contacts[j+1] = t;
            }
        }
    rebuildIndex(list);
    printf("排序完成！\n");
}

void statContacts(ContactList* list) {
    printf("\n===== 通讯录统计信息 =====\n");
    printf("总联系人：%d 位\n", list->count);
    int groupCnt[6] = {0};
    for (int i = 0; i < list->count; i++) groupCnt[list->contacts[i].groupId]++;
    printf("亲属：%d 人\n", groupCnt[1]);
    printf("同事：%d 人\n", groupCnt[2]);
    printf("同学：%d 人\n", groupCnt[3]);
    printf("领导：%d 人\n", groupCnt[4]);
    printf("朋友：%d 人\n", groupCnt[5]);
}

int saveToFile(ContactList* list, const char* file) {
    FILE* fp = fopen(file, "wb");
    if (fp == NULL) { printf("文件打开失败，无法保存\n"); return -1; }
    fwrite(&list->count, sizeof(int), 1, fp);
    fwrite(list->contacts, sizeof(Contact), list->count, fp);
    fclose(fp);
    printf("数据保存成功！\n");
    return 0;
}

int loadFromFile(ContactList* list, const char* file) {
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        printf("联系人文件不存在，加载默认数据...\n");
        loadDefaultContacts(list);
        return -1;
    }
    int num;
    fread(&num, sizeof(int), 1, fp);
    list->count = 0;
    for (int i = 0; i < INITIAL_COUNT; i++) freeLinkList(&list->initialIndex[i]);
    for (int i = 0; i < GROUP_COUNT; i++) freeLinkList(&list->groupIndex[i]);
    if (num > list->capacity) {
        list->capacity = num;
        list->contacts = (Contact*)realloc(list->contacts, list->capacity * sizeof(Contact));
    }
    fread(list->contacts, sizeof(Contact), num, fp);
    list->count = num;
    fclose(fp);
    rebuildIndex(list);
    printf("联系人数据读取完成！共 %d 条\n", num);
    return 0;
}

void destroyContactList(ContactList* list) {
    free(list->contacts);
    for (int i = 0; i < INITIAL_COUNT; i++) freeLinkList(&list->initialIndex[i]);
    for (int i = 0; i < GROUP_COUNT; i++) freeLinkList(&list->groupIndex[i]);
}

// ==================== 个人名片 ====================
void initMyCard(MyCard* card) {
    strcpy(card->name, "小明");
    strcpy(card->phone, "13888888888");
    strcpy(card->tel, "0755-88888888");
    strcpy(card->email, "xiaoming@email.com");
    strcpy(card->company, "科技有限公司");
    strcpy(card->position, "高级软件工程师");
    strcpy(card->address, "深圳市南山区科技园");
    strcpy(card->wechat, "xiaoming_wx");
    strcpy(card->qq, "12345678");
    strcpy(card->bio, "热爱编程，专注于C/C++开发，喜欢开源技术和分享学习心得。");
}

void editMyCard(MyCard* card) {
    char buf[BUF_LEN];
    printf("\n===== 编辑个人名片 =====\n");
    printf("请输入姓名："); scanf("%49s", card->name); clearEnter();
    printf("请输入手机号："); scanf("%19s", card->phone); clearEnter();
    printf("请输入固话："); scanf("%19s", card->tel); clearEnter();
    printf("请输入邮箱："); scanf("%49s", card->email); clearEnter();
    printf("请输入公司："); scanf("%49s", card->company); clearEnter();
    printf("请输入职位："); scanf("%29s", card->position); clearEnter();
    printf("请输入地址：");
    fgets(buf, BUF_LEN, stdin); buf[strcspn(buf, "\n")] = '\0';
    strncpy(card->address, buf, sizeof(card->address)-1);
    printf("请输入微信号："); scanf("%29s", card->wechat); clearEnter();
    printf("请输入QQ号："); scanf("%19s", card->qq); clearEnter();
    printf("请输入个人简介：");
    fgets(buf, BUF_LEN, stdin); buf[strcspn(buf, "\n")] = '\0';
    strncpy(card->bio, buf, sizeof(card->bio)-1);
    printf("名片信息已更新！\n");
}

void showMyCard(MyCard* card) {
    printf("\n");
    printf("+----------------------------------+\n");
    printf("|          个 人 名 片             |\n");
    printf("+----------------------------------+\n");
    printf("|  姓  名：%-22s|\n", card->name);
    printf("|  职  位：%-22s|\n", card->position);
    printf("|  公  司：%-22s|\n", card->company);
    printf("+----------------------------------+\n");
    printf("|  手  机：%-22s|\n", card->phone);
    printf("|  固  话：%-22s|\n", card->tel);
    printf("|  邮  箱：%-22s|\n", card->email);
    printf("|  微  信：%-22s|\n", card->wechat);
    printf("|  Q  Q  ：%-22s|\n", card->qq);
    printf("+----------------------------------+\n");
    printf("|  地  址：%-22s|\n", card->address);
    printf("+----------------------------------+\n");
    printf("|  简  介：\n");
    printf("|  %-30s\n", card->bio);
    printf("+----------------------------------+\n");
}

int saveMyCard(MyCard* card, const char* file) {
    FILE* fp = fopen(file, "wb");
    if (fp == NULL) { printf("名片文件打开失败\n"); return -1; }
    fwrite(card, sizeof(MyCard), 1, fp);
    fclose(fp);
    printf("名片保存成功！\n");
    return 0;
}

int loadMyCard(MyCard* card, const char* file) {
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) { printf("名片文件不存在，使用预设名片\n"); return -1; }
    fread(card, sizeof(MyCard), 1, fp);
    fclose(fp);
    printf("名片读取完成！\n");
    return 0;
}

// ==================== 多级菜单 ====================
void showMainMenu(void) {
    printf("\n========================================\n");
    printf("        通讯录与名片管理系统 v2.0\n");
    printf("========================================\n");
    printf("      1. 联系人管理\n");
    printf("      2. 个人名片\n");
    printf("      0. 退出系统\n");
    printf("========================================\n");
    printf("请输入功能编号：");
}

void showContactMenu(void) {
    printf("\n========================================\n");
    printf("        联系人管理 - 二级菜单\n");
    printf("========================================\n");
    printf("      1. 添加联系人\n");
    printf("      2. 显示所有联系人\n");
    printf("      3. 按群组显示联系人\n");
    printf("      4. 查询联系人\n");
    printf("      5. 删除联系人\n");
    printf("      6. 修改联系人\n");
    printf("      7. 排序联系人\n");
    printf("      8. 统计信息\n");
    printf("      9. 保存数据到文件\n");
    printf("     10. 从文件读取数据\n");
    printf("      0. 返回主菜单\n");
    printf("========================================\n");
    printf("请输入功能编号：");
}

void showSearchMenu(void) {
    printf("\n========================================\n");
    printf("        查询联系人 - 三级菜单\n");
    printf("========================================\n");
    printf("      1. 按姓名查询\n");
    printf("      2. 按手机号查询\n");
    printf("      0. 返回上级菜单\n");
    printf("========================================\n");
    printf("请输入查询方式：");
}

void showCardMenu(void) {
    printf("\n========================================\n");
    printf("        个人名片 - 二级菜单\n");
    printf("========================================\n");
    printf("      1. 查看我的名片\n");
    printf("      2. 编辑我的名片\n");
    printf("      3. 保存名片到文件\n");
    printf("      4. 从文件读取名片\n");
    printf("      0. 返回主菜单\n");
    printf("========================================\n");
    printf("请输入功能编号：");
}

void searchMenu(ContactList* list) {
    int sub;
    while (1) {
        showSearchMenu();
        scanf("%d", &sub); clearEnter();
        if (sub == 0) break;
        char key[50];
        switch (sub) {
            case 1:
                printf("输入查询姓名关键词："); scanf("%49s", key); clearEnter();
                searchByName(list, key); pauseReturn(); break;
            case 2:
                printf("输入查询手机号："); scanf("%19s", key); clearEnter();
                searchByPhone(list, key); pauseReturn(); break;
            default:
                printf("输入错误，请重新选择！\n"); pauseReturn();
        }
    }
}

void contactMenu(ContactList* list) {
    int op;
    while (1) {
        showContactMenu();
        scanf("%d", &op); clearEnter();
        if (op == 0) break;
        switch (op) {
            case 1: { Contact c; inputContact(&c); addContact(list, &c);
                      printf("联系人添加成功！\n"); pauseReturn(); break; }
            case 2: showAllContacts(list); pauseReturn(); break;
            case 3: { int g; printf("输入群组编号(1-5)："); scanf("%d", &g); clearEnter();
                      showByGroup(list, g); pauseReturn(); break; }
            case 4: searchMenu(list); break;
            case 5: { char ph[20]; printf("输入要删除的手机号："); scanf("%19s", ph); clearEnter();
                      deleteContact(list, ph); pauseReturn(); break; }
            case 6: { char ph[20]; printf("输入要修改的手机号："); scanf("%19s", ph); clearEnter();
                      modifyContact(list, ph); pauseReturn(); break; }
            case 7: { int t; printf("1按姓名排序 2按手机号排序："); scanf("%d", &t); clearEnter();
                      sortContacts(list, t); pauseReturn(); break; }
            case 8: statContacts(list); pauseReturn(); break;
            case 9: saveToFile(list, "contact.dat"); pauseReturn(); break;
            case 10: loadFromFile(list, "contact.dat"); pauseReturn(); break;
            default: printf("输入错误，请选择0-10！\n"); pauseReturn();
        }
    }
}

void cardMenu(MyCard* card) {
    int op;
    while (1) {
        showCardMenu();
        scanf("%d", &op); clearEnter();
        if (op == 0) break;
        switch (op) {
            case 1: showMyCard(card); pauseReturn(); break;
            case 2: editMyCard(card); pauseReturn(); break;
            case 3: saveMyCard(card, "mycard.dat"); pauseReturn(); break;
            case 4: loadMyCard(card, "mycard.dat"); pauseReturn(); break;
            default: printf("输入错误，请重新选择！\n"); pauseReturn();
        }
    }
}

// ==================== 主函数 ====================
int main() {
    ContactList list;
    MyCard myCard;
    InitContactList(&list);
    initMyCard(&myCard);
    loadFromFile(&list, "contact.dat");
    loadMyCard(&myCard, "mycard.dat");

    int op;
    while (1) {
        showMainMenu();
        scanf("%d", &op); clearEnter();
        if (op == 0) {
            printf("是否保存数据？1保存 0不保存：");
            int s; scanf("%d", &s); clearEnter();
            if (s == 1) {
                saveToFile(&list, "contact.dat");
                saveMyCard(&myCard, "mycard.dat");
            }
            destroyContactList(&list);
            printf("程序退出，再见！\n");
            break;
        }
        switch (op) {
            case 1: contactMenu(&list); break;
            case 2: cardMenu(&myCard); break;
            default: printf("输入错误，请选择0-2！\n"); pauseReturn();
        }
    }
    return 0;
}

#include "my2.h"

// 注册请求
void registerReq(int sockfd, MSG *msg) {
    memset(msg, 0, sizeof(MSG));
    msg->type = REGISTER;
    printf("用户名："); scanf("%s", msg->username);
    printf("密码："); scanf("%s", msg->passwd);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
}

// 登录请求
void loginReq(int sockfd, MSG *msg) {
    memset(msg, 0, sizeof(MSG));
    msg->type = LOGIN;
    printf("用户名："); scanf("%s", msg->username);
    printf("密码："); scanf("%s", msg->passwd);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    if (msg->type == SUCCESS) {
        printf("%s\n", msg->msg);
        showMainMenu(sockfd, msg);
    } else {
        printf("%s\n", msg->msg);
    }
}

// 发布书籍请求
void publishBookReq(int sockfd, MSG *msg) {
    char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);

    memset(msg, 0, sizeof(MSG));
    msg->type = PUBLISH_BOOK;
   
    printf("书名："); scanf("%s", msg->book_name);
    printf("作者："); scanf("%s", msg->author);
    printf("品相(1-5)："); scanf("%d", &msg->condition);
    printf("取书地点："); scanf("%s", msg->location);
    printf("封面文件名（含路径，如/home/linux/head01.png）："); scanf("%s", msg->filename);
	char pathname[100] = {0};

    sprintf(pathname,"/home/linux/%s",msg->filename);
    int fd = open(pathname, O_RDONLY);
    if (fd == -1) {
        printf("封面文件打开失败：%s\n", strerror(errno));
        return;
    }
    msg->size = read(fd, msg->filedata, MAX_DATA_LEN-1);
    close(fd);
    if (msg->size <= 0) {
        printf("封面读取失败\n");
        return;
    }
    
    strcpy(msg->username, temp_un);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
}

// 搜索书籍请求（打印详细信息）
void searchBookReq(int sockfd, MSG *msg) {
 char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);
    memset(msg, 0, sizeof(MSG));
    msg->type = SEARCH_BOOK;
   
    
    printf("搜索书名关键词："); scanf("%s", msg->book_name);
	strcpy(msg->username, temp_un);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
    
    // 打印搜索到的书籍详细信息
    if (msg->count > 0) {
        printf("===============================\n");
        printf("ID  书名           作者         品相  地点       状态\n");
        printf("-----------------------------------\n");
		int i;
        for (i = 0; i < msg->count; i++) {
            printf("%-3d %-15s %-12s %-5d %-10s %s\n",
                   msg->books[i].id,
                   msg->books[i].book_name,
                   msg->books[i].author,
                   msg->books[i].condition,
                   msg->books[i].location,
                   msg->books[i].status == 0 ? "可借阅" : "已借出");
        }
        printf("===============================\n");
    }
    
   
}

// 借阅书籍请求
void borrowBookReq(int sockfd, MSG *msg) {
char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);
    memset(msg, 0, sizeof(MSG));
    msg->type = BORROW_BOOK;
    
    
    printf("借阅书籍ID："); scanf("%d", &msg->book_id);
strcpy(msg->username, temp_un);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
    
    
}

// 归还书籍请求
void returnBookReq(int sockfd, MSG *msg) {
  char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);
    memset(msg, 0, sizeof(MSG));
    msg->type = RETURN_BOOK;
  strcpy(msg->username, temp_un);
    
    printf("归还书籍ID："); scanf("%d", &msg->book_id);
 
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
    
   
}

// 我的书籍请求（打印详细信息）
void myBooksReq(int sockfd, MSG *msg) {
  char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);
    memset(msg, 0, sizeof(MSG));
    msg->type = MY_BOOKS;
  
       
    strcpy(msg->username, temp_un);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
    
    // 打印我的书籍详细信息
    if (msg->count > 0) {
        printf("===============================\n");
        printf("ID  书名           作者         品相  地点       状态\n");
        printf("-----------------------------------\n");
		int i;
        for (i = 0; i < msg->count; i++) {
            printf("%-3d %-15s %-12s %-5d %-10s %s\n",
                   msg->books[i].id,
                   msg->books[i].book_name,
                   msg->books[i].author,
                   msg->books[i].condition,
                   msg->books[i].location,
                   msg->books[i].status == 0 ? "可借阅" : "已借出");
        }
        printf("===============================\n");
    }
 
}

// 借阅历史请求（打印详细信息）
void borrowHistoryReq(int sockfd, MSG *msg) {
 char temp_un[MAX_NAME_LEN];
    strcpy(temp_un, msg->username);
    memset(msg, 0, sizeof(MSG));
    msg->type = BORROW_HISTORY;
   
    strcpy(msg->username, temp_un);
    send(sockfd, msg, sizeof(MSG), 0);
    recv(sockfd, msg, sizeof(MSG), 0);
    printf("%s\n", msg->msg);
    
    // 打印借阅历史详细信息
    if (msg->count > 0) {
        printf("=============================================\n");
        printf("书籍ID  书名           借阅时间           归还状态\n");
        printf("---------------------------------------------\n");
		int i;
        for (i = 0; i < msg->count; i++) {
            printf("%-7d %-15s %-20s %s\n",
                   msg->borrows[i].book_id,
                   msg->borrows[i].book_name,
                   msg->borrows[i].borrow_time,
                   strlen(msg->borrows[i].return_time) > 0 ? 
                   msg->borrows[i].return_time : "未归还");
        }
        printf("=============================================\n");
    }
    
}

// 主菜单
void showMainMenu(int sockfd, MSG *msg) {
    int choice;
    while (1) {
        printf("\n===== 校园二手书平台 =====\n");
        printf("1.发布书籍  2.搜索书籍  3.借阅书籍\n");
        printf("4.归还书籍  5.我的书籍  6.借阅历史\n");
        printf("7.退出系统\n");
        printf("=========================\n");
        printf("选择："); scanf("%d", &choice);
        switch (choice) {
            case 1: publishBookReq(sockfd, msg); break;
            case 2: searchBookReq(sockfd, msg); break;
            case 3: borrowBookReq(sockfd, msg); break;
            case 4: returnBookReq(sockfd, msg); break;
            case 5: myBooksReq(sockfd, msg); break;
            case 6: borrowHistoryReq(sockfd, msg); break;
            case 7: msg->type = EXIT; send(sockfd, msg, sizeof(MSG), 0);
                    printf("再见！\n"); close(sockfd); exit(0);
            default: printf("无效选择\n");
        }
    }
}

// 客户端主函数
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法：%s <服务器IP> <端口>\n", argv[0]);
        exit(-1);
    }
    
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(atoi(argv[2])), inet_addr(argv[1])};
    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("连接失败"); exit(-1);
    }
    printf("已连接服务器\n");
    
    MSG msg;
    int choice;
    while (1) {
        printf("\n===== 登录界面 =====\n");
        printf("1.注册  2.登录  3.退出\n");
        printf("===================\n");
        printf("选择："); scanf("%d", &choice);
        switch (choice) {
            case 1: registerReq(sockfd, &msg); break;
            case 2: loginReq(sockfd, &msg); break;
            case 3: close(sockfd); printf("再见！\n"); exit(0);
            default: printf("无效选择\n");
        }
    }
    
    return 0;
}


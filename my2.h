#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include <pthread.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>

// 请求类型
#define REGISTER      1
#define LOGIN         2
#define PUBLISH_BOOK  3
#define SEARCH_BOOK   4
#define BORROW_BOOK   5
#define RETURN_BOOK   6
#define MY_BOOKS      7
#define BORROW_HISTORY 8
#define EXIT          9

// 响应状态
#define SUCCESS       0
#define FAIL          -1
#define USER_EXIST    -2
#define USER_PWD_ERR  -3
#define BOOK_NOT_EXIST -4

// 常量定义
#define MAX_NAME_LEN  30
#define MAX_BOOK_INFO 50
#define MAX_DATA_LEN  1024  // 增大缓冲区以容纳详细信息
#define MAX_RECORDS   20    // 最大记录数
#define USER_DIR_BASE "/home/linux/"

// 书籍信息结构体（用于详细信息传输）
typedef struct {
    int id;
    char book_name[MAX_BOOK_INFO];
    char author[MAX_BOOK_INFO];
    int condition;
    char location[MAX_BOOK_INFO];
    char lender[MAX_NAME_LEN];
    int status;
    char cover_name[MAX_BOOK_INFO];
} BookInfo;

// 借阅记录结构体
typedef struct {
    int book_id;
    char book_name[MAX_BOOK_INFO];
    char borrower[MAX_NAME_LEN];
    char borrow_time[MAX_DATA_LEN/10];
    char return_time[MAX_DATA_LEN/10];
} BorrowRecord;

// 通信协议结构体
typedef struct {
    int type;
    char username[MAX_NAME_LEN];
    char passwd[MAX_NAME_LEN];
    
    // 书籍信息
    int book_id;
    char book_name[MAX_BOOK_INFO];
    char author[MAX_BOOK_INFO];
    int condition;
    char location[MAX_BOOK_INFO];
    int status;
    
    // 封面上传
    char filedata[MAX_DATA_LEN];
    int size;
    char filename[MAX_BOOK_INFO];
    
    // 详细信息传输
    int count;  // 记录数量
    BookInfo books[MAX_RECORDS];       // 书籍列表
    BorrowRecord borrows[MAX_RECORDS]; // 借阅记录列表
    
    char msg[MAX_DATA_LEN];
} MSG;

// 函数声明
void initDB(sqlite3 *db);
int doRegister(MSG *msg);
int doLogin(MSG *msg);
int doPublishBook(MSG *msg, sqlite3 *db);
int doSearchBook(MSG *msg, sqlite3 *db);
int doBorrowBook(MSG *msg, sqlite3 *db);
int doReturnBook(MSG *msg, sqlite3 *db);
int doMyBooks(MSG *msg, sqlite3 *db);
int doBorrowHistory(MSG *msg, sqlite3 *db);

void registerReq(int sockfd, MSG *msg);
void loginReq(int sockfd, MSG *msg);
void publishBookReq(int sockfd, MSG *msg);
void searchBookReq(int sockfd, MSG *msg);
void borrowBookReq(int sockfd, MSG *msg);
void returnBookReq(int sockfd, MSG *msg);
void myBooksReq(int sockfd, MSG *msg);
void borrowHistoryReq(int sockfd, MSG *msg);
void showMainMenu(int sockfd, MSG *msg);


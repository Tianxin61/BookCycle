#include "my2.h"

sqlite3 *db = NULL;

// 数据库操作封装
int db_exec(const char *sql) {
    char *errmsg = NULL;
    int ret = sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "DB exec err: %s\n", errmsg);
        sqlite3_free(errmsg);
        return FAIL;
    }
    return SUCCESS;
}

// 数据库查询并获取详细书籍信息
int db_get_books(const char *sql, BookInfo *books) {
    char *errmsg = NULL;
    char **res = NULL;
    int row, col;
    int ret = sqlite3_get_table(db, sql, &res, &row, &col, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "DB get err: %s\n", errmsg);
        sqlite3_free(errmsg);
        return FAIL;
    }
    

    // 提取查询结果到BookInfo数组
    int i;
    for (i = 0; i < row && i < MAX_RECORDS; i++) {
        books[i].id = atoi(res[(i+1)*col + 0]);
        strcpy(books[i].book_name, res[(i+1)*col + 1]);
        strcpy(books[i].author, res[(i+1)*col + 2]);
        books[i].condition = atoi(res[(i+1)*col + 3]);
        strcpy(books[i].location, res[(i+1)*col + 4]);
        strcpy(books[i].lender, res[(i+1)*col + 5]);
        books[i].status = atoi(res[(i+1)*col + 6]);
        strcpy(books[i].cover_name, res[(i+1)*col + 7]);
    }
    
    sqlite3_free_table(res);
    return row > MAX_RECORDS ? MAX_RECORDS : row;

}

// 数据库查询并获取借阅记录
int db_get_borrows(const char *sql, BorrowRecord *borrows) {
    char *errmsg = NULL;
    char **res = NULL;
    int row, col;
    int ret = sqlite3_get_table(db, sql, &res, &row, &col, &errmsg);
    if (ret != SQLITE_OK) {
        fprintf(stderr, "DB get err: %s\n", errmsg);
        sqlite3_free(errmsg);
        return FAIL;
    }
    

    // 提取查询结果到BorrowRecord数组
    int i;
    for (i = 0; i < row && i < MAX_RECORDS; i++) {
        borrows[i].book_id = atoi(res[(i+1)*col + 0]);
        strcpy(borrows[i].book_name, res[(i+1)*col + 1]);
        strcpy(borrows[i].borrower, res[(i+1)*col + 2]);
        strcpy(borrows[i].borrow_time, res[(i+1)*col + 3]);
        strcpy(borrows[i].return_time, res[(i+1)*col + 4]);
    }
    
    sqlite3_free_table(res);
    return row > MAX_RECORDS ? MAX_RECORDS : row;

}

// 创建用户目录
int createUserDir(const char *username) {
    char cmd[128];
    sprintf(cmd, "mkdir -p %s%s && chmod 777 %s%s", 
            USER_DIR_BASE, username, USER_DIR_BASE, username);
    if (system(cmd) == -1) {
        perror("create dir fail");
        return FAIL;
    }
    return SUCCESS;
}

// 初始化数据库
void initDB(sqlite3 *db) {
    db_exec("CREATE TABLE IF NOT EXISTS users ("
            "username TEXT PRIMARY KEY, passwd TEXT NOT NULL);");
    

    db_exec("CREATE TABLE IF NOT EXISTS books ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "book_name TEXT NOT NULL, author TEXT NOT NULL,"
            "condition INTEGER NOT NULL, location TEXT NOT NULL,"
            "lender TEXT NOT NULL, status INTEGER DEFAULT 0,"
            "cover_name TEXT NOT NULL,"
            "FOREIGN KEY(lender) REFERENCES users(username));");
    
    db_exec("CREATE TABLE IF NOT EXISTS borrows ("
            "book_id INTEGER NOT NULL,"
            "book_name TEXT NOT NULL,"  // 必须添加此字段
            "borrower TEXT NOT NULL,"
            "borrow_time TEXT NOT NULL,"
            "return_time TEXT DEFAULT '',"
            "FOREIGN KEY(book_id) REFERENCES books(id));");

}

// 注册功能
int doRegister(MSG *msg) {
    char sql[512];
    sprintf(sql, "SELECT * FROM users WHERE username='%s';", msg->username);
    char **res;
    int row, col;
    sqlite3_get_table(db, sql, &res, &row, &col, NULL);
    if (row > 0) {
        sqlite3_free_table(res);
        strcpy(msg->msg, "用户名已存在");
        return USER_EXIST;
    }
    sqlite3_free_table(res);
    

    sprintf(sql, "INSERT INTO users VALUES('%s','%s');", msg->username, msg->passwd);
    if (db_exec(sql) == FAIL) {
        strcpy(msg->msg, "注册失败");
        return FAIL;
    }
    
    if (createUserDir(msg->username) == FAIL) {
        strcpy(msg->msg, "目录创建失败");
        return FAIL;
    }
    
    strcpy(msg->msg, "注册成功！");
    return SUCCESS;

}

// 登录功能
int doLogin(MSG *msg) {
    char sql[512];
    sprintf(sql, "SELECT * FROM users WHERE username='%s' AND passwd='%s';",
            msg->username, msg->passwd);
    char **res;
    int row, col;
    sqlite3_get_table(db, sql, &res, &row, &col, NULL);
    if (row == 0) {
        sqlite3_free_table(res);
        strcpy(msg->msg, "用户名或密码错误");
        return USER_PWD_ERR;
    }
    sqlite3_free_table(res);
    

    char dirpath[100];
    sprintf(dirpath, "%s%s", USER_DIR_BASE, msg->username);
    DIR *dp = opendir(dirpath);
    if (dp == NULL) {
        strcpy(msg->msg, "用户目录不存在");
        return FAIL;
    }
    closedir(dp);
    
    strcpy(msg->msg, "登录成功！");
    return SUCCESS;

}

// 发布书籍
int doPublishBook(MSG *msg, sqlite3 *db) {
    char sql[512], cover_path[200];
    

    // 1. 构建用户专属目录的封面保存路径
    
    sprintf(cover_path, "%s%s/%s", 
            USER_DIR_BASE,   // 根目录（如/home/ubuntu/）
            msg->username,
    	 msg->filename  // 用户名（动态获取当前用户）
           );  // 仅文件名（不含路径）


​    

    // 3. 将客户端发送的图片数据写入用户目录
    int fd = open(cover_path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if (fd == -1) {
        sprintf(msg->msg, "封面保存失败：%s（路径：%s）", 
                strerror(errno), cover_path);
        return FAIL;
    }
    // 写入客户端发送的图片二进制数据
    write(fd, msg->filedata, msg->size);
    close(fd);
    
    // 4. 将书籍信息存入数据库（包含封面文件名）
    sprintf(sql, "INSERT INTO books(book_name,author,condition,location,lender,cover_name) "
            "VALUES('%s','%s',%d,'%s','%s','%s');",
            msg->book_name, msg->author, msg->condition,
            msg->location, msg->username, msg->filename);  // 存储文件名，便于后续读取
    
    if (db_exec(sql) == FAIL) {
        unlink(cover_path);  // 数据库写入失败时删除已保存的封面
        strcpy(msg->msg, "发布失败（数据库错误）");
        return FAIL;
    }
    
    msg->book_id = sqlite3_last_insert_rowid(db);
    sprintf(msg->msg, "发布成功！书籍ID：%d，封面已保存至：%s", 
            msg->book_id, cover_path);
    return SUCCESS;

}


// 搜索书籍（返回详细信息）
int doSearchBook(MSG *msg, sqlite3 *db) {
    char sql[512];
    sprintf(sql, "SELECT * FROM books WHERE book_name LIKE '%%%s%%';", msg->book_name);
    

    // 查询并获取书籍详细信息
    msg->count = db_get_books(sql, msg->books);
    if (msg->count < 0) {
        strcpy(msg->msg, "搜索失败");
        return FAIL;
    }
    
    sprintf(msg->msg, "搜索到%d本相关书籍", msg->count);
    return SUCCESS;

}

// 借阅书籍
int doBorrowBook(MSG *msg, sqlite3 *db) {
    char sql[512], time_buf[20], book_name[50];
    time_t t = time(NULL);
    strftime(time_buf, 20, "%Y-%m-%d %H:%M", localtime(&t));
    

    // 获取书籍名称
    sqlite3_stmt *stmt;
    sprintf(sql, "SELECT book_name, status FROM books WHERE id=%d;", msg->book_id);
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        strcpy(msg->msg, "查询书籍失败");
        return FAIL;
    }
    
    if (sqlite3_step(stmt) != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        strcpy(msg->msg, "书籍不存在");
        return BOOK_NOT_EXIST;
    }
    
    strcpy(book_name, (const char*)sqlite3_column_text(stmt, 0));
    int status = sqlite3_column_int(stmt, 1);
    sqlite3_finalize(stmt);
    
    if (status == 1) {
        strcpy(msg->msg, "书籍已借出");
        return FAIL;
    }
    
    // 更新书籍状态
    sprintf(sql, "UPDATE books SET status=1 WHERE id=%d;", msg->book_id);
    db_exec(sql);
    
    // 记录借阅历史
    sprintf(sql, "INSERT INTO borrows(book_id, book_name, borrower, borrow_time) "
            "VALUES(%d, '%s', '%s', '%s');",
            msg->book_id, book_name, msg->username, time_buf);
    db_exec(sql);


​    

    strcpy(msg->msg, "借阅成功");
    return SUCCESS;

}

// 归还书籍
int doReturnBook(MSG *msg, sqlite3 *db) {
    char sql[512], time_buf[20];
    time_t t = time(NULL);
    strftime(time_buf, 20, "%Y-%m-%d %H:%M", localtime(&t));
    

    // 检查借阅记录
    sprintf(sql, "SELECT * FROM borrows WHERE book_id=%d AND borrower='%s' AND return_time='';",
            msg->book_id, msg->username);
    char **res;
    int row, col;
    sqlite3_get_table(db, sql, &res, &row, &col, NULL);
    if (row == 0) {
        sqlite3_free_table(res);
        strcpy(msg->msg, "无此借阅记录");
        return FAIL;
    }
    sqlite3_free_table(res);
    
    // 更新书籍状态
    sprintf(sql, "UPDATE books SET status=0 WHERE id=%d;", msg->book_id);
    db_exec(sql);
    
    // 更新归还时间
    sprintf(sql, "UPDATE borrows SET return_time='%s' WHERE book_id=%d AND borrower='%s';",
            time_buf, msg->book_id, msg->username);
    db_exec(sql);
    
    strcpy(msg->msg, "归还成功");
    return SUCCESS;

}

// 我的书籍（返回详细信息）
int doMyBooks(MSG *msg, sqlite3 *db) {
    char sql[512];
    sprintf(sql, "SELECT * FROM books WHERE lender='%s';", msg->username);
    

    // 查询并获取书籍详细信息
    msg->count = db_get_books(sql, msg->books);
    if (msg->count < 0) {
        strcpy(msg->msg, "查询失败");
        return FAIL;
    }
    
    sprintf(msg->msg, "您发布了%d本书籍", msg->count);
    return SUCCESS;

}

// 借阅历史（返回详细信息）
int doBorrowHistory(MSG *msg, sqlite3 *db) {
    char sql[512];
    sprintf(sql, "SELECT book_id, book_name, borrower, borrow_time, return_time "
            "FROM borrows WHERE borrower='%s';", msg->username);
    

    // 查询并获取借阅记录
    msg->count = db_get_borrows(sql, msg->borrows);
    if (msg->count < 0) {
        strcpy(msg->msg, "查询失败");
        return FAIL;
    }
    
    sprintf(msg->msg, "您有%d条借阅记录", msg->count);
    return SUCCESS;

}

// 客户端处理线程
void *client_handler(void *arg) {
    int sockfd = *(int*)arg;
    free(arg);
    MSG msg;
    

    while (1) {
        memset(&msg, 0, sizeof(MSG));
        int ret = recv(sockfd, &msg, sizeof(MSG), 0);
        if (ret <= 0) {
            printf("客户端断开\n");
            break;
        }
        
        switch (msg.type) {
            case REGISTER: msg.type = doRegister(&msg); break;
            case LOGIN: msg.type = doLogin(&msg); break;
            case PUBLISH_BOOK: msg.type = doPublishBook(&msg, db); break;
            case SEARCH_BOOK: msg.type = doSearchBook(&msg, db); break;
            case BORROW_BOOK: msg.type = doBorrowBook(&msg, db); break;
            case RETURN_BOOK: msg.type = doReturnBook(&msg, db); break;
            case MY_BOOKS: msg.type = doMyBooks(&msg, db); break;
            case BORROW_HISTORY: msg.type = doBorrowHistory(&msg, db); break;
            case EXIT: goto exit_handler;
            default: msg.type = FAIL; strcpy(msg.msg, "未知请求");
        }
        
        send(sockfd, &msg, sizeof(MSG), 0);
    }

exit_handler:
    close(sockfd);
    pthread_exit(NULL);
}

// 服务器主函数
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("用法：%s <IP> <端口>\n", argv[0]);
        exit(-1);
    }
    

    if (sqlite3_open("./book.db", &db) != SQLITE_OK) {
        fprintf(stderr, "DB open fail: %s\n", sqlite3_errmsg(db));
        exit(-1);
    }
    
    initDB(db);
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {AF_INET, htons(atoi(argv[2])), inet_addr(argv[1])};
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);
    printf("服务器启动：%s:%s\n", argv[1], argv[2]);
    
    pthread_t tid;
    while (1) {
        int *client_fd = malloc(sizeof(int));
        *client_fd = accept(server_fd, NULL, NULL);
        printf("新连接：%d\n", *client_fd);
        pthread_create(&tid, NULL, client_handler, client_fd);
        pthread_detach(tid);
    }
    
    sqlite3_close(db);
    close(server_fd);
    return 0;

}


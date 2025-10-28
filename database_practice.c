#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLEARSCREEN_SWITCH  1   //控制是否开启清屏功能的总开关
#define ENABLE_INITIAL_INFO 1   //控制是否启用初始数据
#define INITIAL_CAPACITY    10  //设置数据库初始容量
#define AMPLI_MULTI         1.5 //设置容量用尽时的扩容倍数
#define DEL_WAY             0   //删除学生记录的方式；
                                    //0：将删除项之后整体前移(整齐)
                                    //1：仅将最后一项覆盖被删除数据(节省资源)

void clearScreen() {//清除终端内容
if(CLEARSCREEN_SWITCH==0)
    return;
#ifdef _WIN32  // Windows平台
    system("cls");
#else  // Unix/Linux/Mac平台
    system("clear");
#endif
}

void waitForKey(){//让用户敲下回车键才能继续执行代码的子函数
    printf("---按下回车键继续---");
    // 清空输入缓冲区
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);
    getchar();
    printf("\n");
    clearScreen();
}

#ifdef _WIN32
#include <windows.h>
void setupConsoleEncoding(){//windows环境下更改终端编码为GBK（我的vscode有bug必须要这个）
    SetConsoleOutputCP(936);
    SetConsoleCP(936);
}
#else
void setupConsoleEncoding(){

}
#endif
typedef struct{//定义一个学生数据的结构体
    char name[50];
    int age;
    int grade;
    char major[50];
}stu;

typedef struct{//定义一个数据库的结构体
    stu *students;
    int count;
    int capacity;
}database;



void displayStuInfo(database *db,int index){//显示指定编号的学生信息子函数
    if(index!=-1){
        printf("姓名：%s 年龄：%d 年级：%d 专业：%s\n",
        db->students[index].name,
        db->students[index].age,
        db->students[index].grade,
        db->students[index].major);
    }else{
        printf("结果为空\n");
    }
}

void addStu(database *db,char *name,int age,int grade,char *major){//添加学生信息的子函数
    if(db->count==db->capacity){//爆内存检查
        db->students=(stu*)realloc(db->students,sizeof(stu)*(int)(db->capacity*AMPLI_MULTI));//内存扩容
        db->capacity=(int)(db->capacity*AMPLI_MULTI);
    }
    strcpy(db->students[db->count].name,name);
    db->students[db->count].age=age;
    db->students[db->count].grade=grade;
    strcpy(db->students[db->count].major,major);
    db->count++;
    printf("%d号添加成功！\n",db->count);
}

int displayAllStu(database *db){//显示所有的学生的信息的子函数
    if(db->count==0){
        printf("//没有数据！\n");
        return 0;
    }
    for(int i=0;i<=db->count-1;i++){
        printf("%d. ",i+1);
        displayStuInfo(db,i);
    }
    return 1;
}

int delStu(database *db,int index){//删除学生信息的子函数
    if(DEL_WAY==0){
        if(index>=0&&index<=db->count-1){
            for(int i=index;i<db->count-1;i++){
                db->students[i]=db->students[i+1];
            }
            db->count--;
            return 1;
        }else{
            return 0;
        }
    }else{
        if(index>=0&&index<=db->count-1){
            db->students[index]=db->students[db->count-1];
            db->count--;
            return 1;
        }else{
            return 0;
        }
    }


}

int queryStuByName(database *db,char *name){//通过姓名精确查询学生信息的子函数，因为姓名是不可重复的所以比其他功能简短
    for(int i=0;i<=db->count-1;i++){
          if(strcmp(db->students[i].name,name)==0)
          return i;
    }
    return -1;
}

int displayQueryStuByAge(database *db,int age){//显示与关键字相符的项并显示出来，返回是否成功查询的结果
    clearScreen();
    printf("---年龄查询---\n");
    int count=0;
    for(int i=0;i<=db->count-1;i++){
          if(db->students[i].age==age){
            count++;
            printf("%d. ",count);
            displayStuInfo(db,i);
          }
    } 
    if(count==0){
        printf("没有数据！\n");
        return 0;
    }else{
        return 1;
    }
}
int displayQueryStuByGrade(database *db,int grade){//显示与关键字相符的项并显示出来，返回是否成功查询的结果
    clearScreen();
    printf("---年级查询---\n");
    int count=0;
    for(int i=0;i<=db->count-1;i++){
          if(db->students[i].grade==grade){
            count++;
            printf("%d. ",count);
            displayStuInfo(db,i);
          }
    } 
    if(count==0){
        printf("没有数据！\n");
        return 0;
    }else{
        return 1;
    }
}
int displayQueryStuByMajor(database *db,char *major){//显示与关键字相符的项并显示出来，返回是否成功查询的结果
    clearScreen();
    printf("---专业查询---\n");
    int count=0;
    for(int i=0;i<=db->count-1;i++){
          if(strcmp(db->students[i].major,major)==0){
            count++;
            printf("%d. ",count);
            displayStuInfo(db,i);
          }
    } 
    if(count==0){
        printf("没有数据！\n");
        return 0;
    }else{
        return 1;
    }
}

int queryStuByAge(database *db,int age,int index){//根据关键字获取第index个符合条件的项
    int count;
    for(int i=0;i<=db->count-1;i++){
        if(db->students[i].age==age){
            count++;
            if(count==index){
            return i;
            }
        }
    }
    return -1;
}

int queryStuByGrade(database *db,int grade,int index){//根据关键字获取第index个符合条件的项
    int count;
    for(int i=0;i<=db->count-1;i++){
        if(db->students[i].grade==grade){
            count++;
            if(count==index){
            return i;
            }
        }
    }
    return -1;
}

int queryStuByMajor(database *db,char *major,int index){//根据关键字获取第index个符合条件的项
    int count;
    for(int i=0;i<=db->count-1;i++){
        if(strcmp(db->students[i].major,major)==0){
            count++;
            if(count==index){
            return i;
            }
        }
    }
    return -1;
}
void queryStu(database *db){//信息查询，不返回指定编号
printf("---信息查询---\n");
    printf("1.通过姓名查询\n");
    printf("2.通过年龄查询\n");
    printf("3.通过年级查询\n");
    printf("4.通过专业查询\n");
    int choice;
    int index;
    scanf("%d",&choice);
    clearScreen();
    switch(choice){
        case 1: char name[50];
                printf("请输入查询姓名：");
                scanf("%s",name);
                clearScreen();
                displayStuInfo(db,queryStuByName(db,name));
                break;
        case 2: int age;
                printf("请输入查询年龄：");
                scanf("%d",&age);
                clearScreen();
                displayQueryStuByAge(db,age);
                break;
        case 3: int grade;
                printf("请输入查询年级：");
                scanf("%d",&grade);
                clearScreen();
                displayQueryStuByGrade(db,grade);
                break;
        case 4: char major[50];
                printf("请输入查询专业：");
                scanf("%s",major);
                clearScreen();
                displayQueryStuByMajor(db,major);
                break;
    }
}

int queryStuReturn(database *db){//查询学生信息并返回学生信息编号
    printf("---信息查询---\n");
    printf("1.通过姓名查询\n");
    printf("2.通过年龄查询\n");
    printf("3.通过年级查询\n");
    printf("4.通过专业查询\n");
    printf("5.全局查询\n");
    int choice;
    int index;
    scanf("%d",&choice);
    clearScreen();
    switch(choice){
        case 1: char name[50];
                printf("请输入查询姓名：");
                scanf("%s",name);
                clearScreen();
                return queryStuByName(db,name);
        case 2: int age;
                printf("请输入查询年龄：");
                scanf("%d",&age);
                clearScreen();
                if(displayQueryStuByAge(db,age)==0)
                    return -1;
                printf("请输入指定编号：");
                scanf("%d",&index);
                clearScreen();
                return queryStuByAge(db,age,index);
        case 3: int grade;
                printf("请输入查询年级：");
                scanf("%d",&grade);
                clearScreen();
                if(displayQueryStuByGrade(db,grade)==0)
                    return -1;
                printf("请输入指定编号：");
                scanf("%d",&index);
                clearScreen();
                return queryStuByGrade(db,grade,index);
        case 4: char major[50];
                printf("请输入查询专业：");
                scanf("%s",major);
                clearScreen();
                if(displayQueryStuByMajor(db,major)==0)
                    return -1;
                printf("请输入指定编号：");
                scanf("%d",&index);
                clearScreen();
                return queryStuByMajor(db,major,index);
        case 5: if(displayAllStu(db)!=0){
                        printf("请输入指定编号：");
                        int index;
                        scanf("%d",&index);
                        clearScreen();
                        return index-1;
                    }
                return -1;
    }
}

void addStuUI(database *db){//添加学生信息时的操作界面
        char name[50];
    int age;
    int grade;
    char major[50];
    do{
        printf("---添加学生记录---\n");
        printf("请输入姓名：");
        scanf("%s",name);
        if(queryStuByName(db,name)!=-1){    
            printf("该姓名已存在!\n");
            waitForKey();
        }
    }while(queryStuByName(db,name)!=-1);
    printf("请输入年龄：");
    scanf("%d",&age);
    printf("请输入年级(阿拉伯数字)：");
    scanf("%d",&grade);
    printf("请输入专业：");
    scanf("%s",major);
    addStu(db,name,age,grade,major);
    waitForKey();
    return;
}

void initDB(database *db){//初始化数据库
    db->count = 0;
    db->capacity = INITIAL_CAPACITY;
    db->students = (stu*)malloc(db->capacity*sizeof(stu));
    memset(db->students,0,db->capacity*sizeof(stu));
    if(ENABLE_INITIAL_INFO==1){
        //预制/*菜*/人
    /*     while(1){//超级爆内存测试
        addStu(db,"张三",18,1,"自动化类");
        addStu(db,"李四",18,1,"低空");
        addStu(db,"小伍",18,1,"光电");
        addStu(db,"yhc",18,1,"自动化类");
        addStu(db,"hym",18,1,"应用物理")
        } */
        addStu(db,"张三",18,1,"自动化类");
        addStu(db,"李四",18,1,"低空");
        addStu(db,"小伍",18,1,"光电");
        addStu(db,"yhc",18,1,"自动化类");
        addStu(db,"hym",18,1,"应用物理");
        clearScreen();
    }
    
}

int main(){
    setupConsoleEncoding();//将终端编码设置为936 GBK编码
    database db;//定义一个数据库
    initDB(&db);//初始化数据库
    //db.students[5].age=1;
    int choice=0;
    //printf("%s",db.students[5].name);
    do{
        printf("----学生数据库系统菜单----\n");
        printf("//选择对应选项\n");
        printf("1.添加学生记录\n");
        printf("2.显示所有学生记录\n");
        printf("3.修改学生记录\n");
        printf("4.删除学生记录\n");
        printf("5.查找学生记录\n");
        printf("0.退出系统\n");
        //printf("%s",db.students[0].name);
        scanf("%d",&choice);
        clearScreen();
        switch(choice){
            case 1: addStuUI(&db);
                    break;
            case 2: printf("---显示所有学生记录---\n");
                    displayAllStu(&db);
                    waitForKey();
                    break;
            case 3: printf("---修改学生记录---\n");
                    printf("如何查找被修改项？\n");
                    if(delStu(&db,queryStuReturn(&db))!=1){
                        printf("修改失败！\n");
                        waitForKey();
                        break;
                    }
                    addStuUI(&db);
                    break;
            case 4: printf("---删除学生记录---\n");
                    printf("如何查找被删除项？\n");
                    if(delStu(&db,queryStuReturn(&db))==1){
                        printf("删除成功！\n");
                    }else{
                        printf("删除失败！\n");
                    }
                    waitForKey();
                    break;
            case 5: printf("---查找学生记录---\n");
                    queryStu(&db);
                    waitForKey();
                    break;
                    //printf("%d",queryStuByName(&db,name));
            case 0: return 0;
            default:printf("无效操作！\n");
                    waitForKey();
        }
    }while(1);
    return 0;
}
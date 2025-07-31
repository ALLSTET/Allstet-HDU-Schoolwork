// 头文件与常量定义
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define BLOCKSIZE 1024
#define SIZE 1024000
#define END 65535
#define FREE 0
#define ROOTBLOCKNUM 2
#define MAXOPENFILE 10

// 位示图相关定义
#define BLOCKNUM (SIZE / BLOCKSIZE)
#define BITMAP_SIZE ((BLOCKNUM + 7) / 8)
unsigned char bitmap[BITMAP_SIZE]; // 全局位示图

// FCB结构体
typedef struct FCB
{
  char filename[8];
  char exname[3];
  unsigned char attribute;
  unsigned short time;
  unsigned short date;
  unsigned short first;
  unsigned long length;
  char free;
} fcb;

// FAT结构体
typedef struct FAT
{
  unsigned short id;
} fat;

// 用户打开文件表
typedef struct USEROPEN
{
  char filename[8];
  char exname[3];
  unsigned char attribute;
  unsigned short time;
  unsigned short date;
  unsigned short first;
  unsigned long length;
  char free;
  int dirno;
  int diroff;
  char dir[MAXOPENFILE][80];
  int count;
  char fcbstate;
  char topenfile;
} useropen;

// 引导块
typedef struct BLOCK0
{
  char information[200];
  unsigned short root;
  unsigned char *startblock;
} block0;

// 全局变量
unsigned char *myvhard;
useropen openfilelist[MAXOPENFILE];
useropen *ptrcurdir;
char currentdir[80];
unsigned char *startp;

// 函数声明
void startsys();
void my_format();
void my_cd(char *dirname);
void my_mkdir(char *dirname);
void my_rmdir(char *dirname);
void my_ls(void);
int my_create(char *filename);
void my_rm(char *filename);
int my_open(char *filename);
void my_close(int fd);
int my_write(int fd);
int my_read(int fd, int len);
void my_exitsys();
int do_read(int fd, char *text, int len);
int do_write(int fd, char *text, int len);

// 位示图操作函数声明
int bitmap_alloc_block();
void bitmap_free_block(int blk);
int bitmap_is_used(int blk);

// main函数
int main(void)
{
#ifdef _WIN32
  SetConsoleOutputCP(65001); // 设置控制台输出为UTF-8
  SetConsoleCP(65001);       // 设置控制台输入为UTF-8
#endif
  myvhard = NULL;
  startp = NULL;
  memset(openfilelist, 0, sizeof(openfilelist));
  startsys();

  printf("欢迎使用简易文件系统！\n");
  printf("支持的命令有:my_format, my_mkdir, my_rmdir, my_ls, my_create, my_rm, my_open, my_close, my_write, my_read, my_cd, my_exitsys\n");
  printf("命令调用格式:my_(COMMAND)\n");

  char buf[128];
  while (1)
  {
    printf("<%s>", currentdir);
    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);
    buf[strcspn(buf, "\n")] = 0;
    char name[80] = {0};

    if (strcmp(buf, "my_exitsys") == 0)
    {
      my_exitsys();
      break;
    }
    else if (strcmp(buf, "my_format") == 0)
    {
      my_format();
    }
    else if (strncmp(buf, "my_mkdir ", 9) == 0)
    {
      sscanf(buf + 9, "%s", name);
      my_mkdir(name);
    }
    else if (strncmp(buf, "my_rmdir ", 9) == 0)
    {
      sscanf(buf + 9, "%s", name);
      my_rmdir(name);
    }
    else if (strcmp(buf, "my_ls") == 0)
    {
      my_ls();
    }
    else if (strncmp(buf, "my_create ", 10) == 0)
    {
      sscanf(buf + 10, "%s", name);
      my_create(name);
    }
    else if (strncmp(buf, "my_rm ", 6) == 0)
    {
      sscanf(buf + 6, "%s", name);
      my_rm(name);
    }
    else if (strncmp(buf, "my_open ", 8) == 0)
    {
      sscanf(buf + 8, "%s", name);
      my_open(name);
    }
    else if (strncmp(buf, "my_cd ", 6) == 0)
    {
      sscanf(buf + 6, "%s", name);
      my_cd(name);
    }
    else if (strncmp(buf, "my_write ", 9) == 0)
    {
      int fd;
      sscanf(buf + 9, "%d", &fd);
      my_write(fd);
    }
    else if (strncmp(buf, "my_read ", 8) == 0)
    {
      int fd, len;
      sscanf(buf + 8, "%d %d", &fd, &len);
      my_read(fd, len);
    }
    else if (strncmp(buf, "my_close ", 9) == 0)
    {
      int fd;
      sscanf(buf + 9, "%d", &fd);
      my_close(fd);
    }
    else
    {
      printf("Undefined Command!\tif need to exit,enter \"my_exitsys\"\n");
    }
  }
  return 0;
}

// ===================== 文件系统核心函数实现 =====================

// 启动文件系统
void startsys()
{
  FILE *fp;
  char magic[9] = {0};
  myvhard = (unsigned char *)malloc(SIZE);
  if (!myvhard)
  {
    printf("虚拟磁盘空间申请失败！\n");
    exit(1);
  }
  fp = fopen("myfsys", "rb+");
  if (fp)
  {
    fread(myvhard, 1, SIZE, fp);
    strncpy(magic, (char *)myvhard, 8);
    magic[8] = '\0';
    if (strcmp(magic, "10101010") != 0)
    {
      printf("myfsys文件系统魔数错误,重新格式化文件系统\n");
      my_format();
      fseek(fp, 0, SEEK_SET);
      fwrite(myvhard, 1, SIZE, fp);
    }
    fclose(fp);
  }
  else
  {
    printf("myfsys文件系统不存在,现在开始创建文件系统\n");
    fp = fopen("myfsys", "wb+");
    if (!fp)
    {
      printf("创建myfsys文件失败!\n");
      exit(1);
    }
    my_format();
    fwrite(myvhard, 1, SIZE, fp);
    fclose(fp);
  }
  memset(openfilelist, 0, sizeof(openfilelist));
  memset(currentdir, 0, sizeof(currentdir));
  useropen *root = &openfilelist[0];
  memset(root, 0, sizeof(useropen));
  strcpy(root->filename, ".");
  root->attribute = 0;
  root->first = 5;
  root->length = 2 * sizeof(fcb);
  root->free = 1;
  root->dirno = 5;
  root->diroff = 0;
  root->count = 0;
  root->fcbstate = 0;
  root->topenfile = 1;
  ptrcurdir = root;
  strcpy(currentdir, "/");
}

// 格式化虚拟磁盘
void my_format()
{
  block0 *boot = (block0 *)myvhard;
  memset(boot, 0, BLOCKSIZE);
  strcpy(boot->information, "BLOCKSIZE:1024\nBLOCKNUM:1000\nMAXOPENFILE:10");
  memcpy(myvhard, "10101010", 8);
  boot->root = 5;
  boot->startblock = myvhard + BLOCKSIZE * 5;
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  fat *fat2 = (fat *)(myvhard + BLOCKSIZE * 3);
  int fat_item_count = SIZE / BLOCKSIZE;
  for (int i = 0; i < 6; i++)
  {
    fat1[i].id = END;
    fat2[i].id = END;
  }
  for (int i = 6; i < fat_item_count; i++)
  {
    fat1[i].id = FREE;
    fat2[i].id = FREE;
  }
  fcb *root_dir = (fcb *)(myvhard + 5 * BLOCKSIZE);
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
  {
    memset(&root_dir[i], 0, sizeof(fcb));
    root_dir[i].free = 0;
    root_dir[i].filename[0] = '\0';
  }
  strcpy(root_dir[0].filename, ".");
  root_dir[0].attribute = 0;
  root_dir[0].first = 5;
  root_dir[0].length = 2 * sizeof(fcb); // 根目录.和..的length应为2*sizeof(fcb)
  root_dir[0].free = 1;
  strcpy(root_dir[1].filename, "..");
  root_dir[1].attribute = 0;
  root_dir[1].first = 5;
  root_dir[1].length = 2 * sizeof(fcb);
  root_dir[1].free = 1;
  // 初始化位示图
  memset(bitmap, 0, sizeof(bitmap));
  // 保留前6块（引导块、FAT等），置为已用
  for (int i = 0; i < 6; i++)
  {
    int byte = i / 8, bit = i % 8;
    bitmap[byte] |= (1 << bit);
  }
  // 根目录块也置为已用
  int rootblk = 5;
  int byte = rootblk / 8, bit = rootblk % 8;
  bitmap[byte] |= (1 << bit);
}

// 更改当前目录
void my_cd(char *dirname)
{

  if (strcmp(dirname, "/") == 0)
  {
    ptrcurdir = &openfilelist[0];
    strcpy(currentdir, "/");
    return;
  }

  int cur_fd = 0;
  fcb *dirfcb = (fcb *)(myvhard + 5 * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  int found = 0;
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, dirname) == 0 && dirfcb[i].attribute == 0)
    {
      found = 1;
      // 填写打开文件表
      useropen *u = &openfilelist[1];
      memset(u, 0, sizeof(useropen));
      strncpy(u->filename, dirfcb[i].filename, sizeof(u->filename) - 1);
      u->attribute = 0;
      u->first = dirfcb[i].first;
      u->length = dirfcb[i].length;
      u->free = 1;
      u->dirno = 5;
      u->diroff = i;
      u->count = 0;
      u->fcbstate = 0;
      u->topenfile = 1;
      ptrcurdir = u;
      snprintf(currentdir, sizeof(currentdir), "/%s", dirname);
      break;
    }
  }
  if (!found)
  {
    printf("目录 %s 不存在！\n", dirname);
  }
}

// 创建子目录
void my_mkdir(char *dirname)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  // 重名检查
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, dirname) == 0)
    {
      printf("目录名 %s 已存在！\n", dirname);
      return;
    }
  }
  // 用bitmap分配新块
  int new_block = bitmap_alloc_block();
  if (new_block == -1)
  {
    printf("磁盘空间不足！\n");
    return;
  }
  // FAT表同步
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  fat1[new_block].id = END;
  // 找空目录项
  int slot_found = -1;
  for (int i = 0; i < num_entries; i++)
  {
    if (!dirfcb[i].free && dirfcb[i].filename[0] == '\0')
    {
      slot_found = i;
      break;
    }
  }
  if (slot_found == -1)
    slot_found = num_entries;
  // 写入目录项
  fcb new_dir;
  memset(&new_dir, 0, sizeof(fcb));
  strncpy(new_dir.filename, dirname, sizeof(new_dir.filename) - 1);
  new_dir.attribute = 0;
  new_dir.first = new_block;
  new_dir.length = 2 * sizeof(fcb); // 新目录.和..的length应为2*sizeof(fcb)
  new_dir.free = 1;
  memcpy(&dirfcb[slot_found], &new_dir, sizeof(fcb));
  // 初始化新目录块
  fcb *newblk = (fcb *)(myvhard + new_block * BLOCKSIZE);
  memset(newblk, 0, BLOCKSIZE);
  strcpy(newblk[0].filename, ".");
  newblk[0].attribute = 0;
  newblk[0].first = new_block;
  newblk[0].length = 2 * sizeof(fcb);
  newblk[0].free = 1;
  strcpy(newblk[1].filename, "..");
  newblk[1].attribute = 0;
  newblk[1].first = openfilelist[cur_fd].first;
  newblk[1].length = 2 * sizeof(fcb);
  newblk[1].free = 1;
  // 更新父目录length
  openfilelist[cur_fd].length = 0;
  dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
    if (dirfcb[i].free == 1 && dirfcb[i].filename[0] != '\0')
      openfilelist[cur_fd].length += sizeof(fcb);
  printf("目录 %s 创建成功！\n", dirname);
}

// 删除子目录
void my_rmdir(char *dirname)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  int idx = -1;
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, dirname) == 0 && dirfcb[i].attribute == 0)
    {
      idx = i;
      break;
    }
  }
  if (idx == -1)
  {
    printf("目录 %s 不存在！\n", dirname);
    return;
  }
  // 检查目录是否为空
  int blk = dirfcb[idx].first;
  fcb *sublist = (fcb *)(myvhard + blk * BLOCKSIZE);
  int empty = 1;
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
  {
    if (sublist[i].free && strcmp(sublist[i].filename, ".") != 0 && strcmp(sublist[i].filename, "..") != 0)
    {
      empty = 0;
      break;
    }
  }
  if (!empty)
  {
    printf("目录 %s 非空，不能删除！\n", dirname);
    return;
  }
  // 回收FAT
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  blk = dirfcb[idx].first;
  while (blk != END)
  {
    int next = fat1[blk].id;
    fat1[blk].id = FREE;
    bitmap_free_block(blk); // 回收bitmap
    if (next == END)
      break;
    blk = next;
  }
  memset(&dirfcb[idx], 0, sizeof(fcb));
  dirfcb[idx].free = 0;
  // 更新父目录length
  openfilelist[cur_fd].length = 0;
  dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
    if (dirfcb[i].free == 1 && dirfcb[i].filename[0] != '\0')
      openfilelist[cur_fd].length += sizeof(fcb);
  printf("目录 %s 删除成功！\n", dirname);
}

// 显示目录内容
void my_ls(void)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  printf("文件名\t类型\t大小\n");
  printf("----------------------\n");
  int dot_count = 0, dotdot_count = 0;
  for (int i = 0; i < num_entries; i++)
  {
    // 只显示有效目录项（free==1 且 filename[0]!='\0'）
    if (dirfcb[i].free == 1 && dirfcb[i].filename[0] != '\0')
    {
      // 跳过根目录区的多余.和..，并且只显示一个.和..，且只显示大小为2*sizeof(fcb)的.和..（即正常的）
      if (strcmp(dirfcb[i].filename, ".") == 0)
      {
        if (dot_count > 0 || dirfcb[i].length != 2 * sizeof(fcb))
          continue;
        dot_count++;
      }
      else if (strcmp(dirfcb[i].filename, "..") == 0)
      {
        if (dotdot_count > 0 || dirfcb[i].length != 2 * sizeof(fcb))
          continue;
        dotdot_count++;
      }
      // 只显示普通目录项和正常. ..，过滤掉异常length的. ..（如1080）
      printf("%s%s%s\t%s\t%lu\n",
             dirfcb[i].filename,
             dirfcb[i].attribute == 1 && dirfcb[i].exname[0] ? "." : "",
             dirfcb[i].attribute == 1 && dirfcb[i].exname[0] ? dirfcb[i].exname : "",
             dirfcb[i].attribute == 0 ? "DIR" : "FILE",
             dirfcb[i].length);
    }
  }
}

// 创建文件
int my_create(char *filename)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  // 重名检查
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, filename) == 0)
    {
      printf("文件名已存在！\n");
      return -1;
    }
  }
  // 查找空闲打开文件表项
  int free_fd = -1;
  for (int i = 0; i < MAXOPENFILE; i++)
  {
    if (!openfilelist[i].topenfile)
    {
      free_fd = i;
      break;
    }
  }
  if (free_fd == -1)
  {
    printf("打开文件表已满，无法创建新文件！\n");
    return -1;
  }
  // 用bitmap分配新块
  int new_block = bitmap_alloc_block();
  if (new_block == -1)
  {
    printf("磁盘空间不足！\n");
    return -1;
  }
  // FAT表同步
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  fat1[new_block].id = END;
  // 找空目录项
  int slot_found = -1;
  for (int i = 0; i < num_entries; i++)
  {
    if (!dirfcb[i].free && dirfcb[i].filename[0] == '\0')
    {
      slot_found = i;
      break;
    }
  }
  if (slot_found == -1)
    slot_found = num_entries;
  // 写入目录项
  fcb new_file;
  memset(&new_file, 0, sizeof(fcb));
  // 拆分主文件名和扩展名
  char *dot = strchr(filename, '.');
  if (dot)
  {
    size_t name_len = dot - filename;
    if (name_len > 8)
      name_len = 8;
    strncpy(new_file.filename, filename, name_len);
    new_file.filename[name_len] = '\0';
    strncpy(new_file.exname, dot + 1, 3);
    new_file.exname[3] = '\0';
  }
  else
  {
    strncpy(new_file.filename, filename, 8);
    new_file.filename[8] = '\0';
    new_file.exname[0] = '\0';
  }
  new_file.attribute = 1;
  new_file.first = new_block;
  new_file.length = 0;
  new_file.free = 1;
  memcpy(&dirfcb[slot_found], &new_file, sizeof(fcb));
  // 填写打开文件表项
  useropen *u = &openfilelist[free_fd];
  memset(u, 0, sizeof(useropen));
  strncpy(u->filename, filename, sizeof(u->filename) - 1);
  u->attribute = 1;
  u->first = new_block;
  u->length = 0;
  u->free = 1;
  u->dirno = openfilelist[cur_fd].first;
  u->diroff = slot_found;
  u->count = 0;
  u->fcbstate = 0;
  u->topenfile = 1;
  // 更新父目录length
  openfilelist[cur_fd].length = 0;
  dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
    if (dirfcb[i].free == 1 && dirfcb[i].filename[0] != '\0')
      openfilelist[cur_fd].length += sizeof(fcb);
  printf("文件 %s 创建成功，fd=%d\n", filename, free_fd);
  return free_fd;
}

// 删除文件
void my_rm(char *filename)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  int idx = -1;
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, filename) == 0 && dirfcb[i].attribute == 1)
    {
      idx = i;
      break;
    }
  }
  if (idx == -1)
  {
    printf("文件 %s 不存在！\n", filename);
    return;
  }
  // 检查是否已打开，关闭之
  for (int i = 0; i < MAXOPENFILE; i++)
  {
    if (openfilelist[i].topenfile && strcmp(openfilelist[i].filename, filename) == 0)
    {
      my_close(i);
      break;
    }
  }
  // 回收FAT
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  int blk = dirfcb[idx].first;
  while (blk != END)
  {
    int next = fat1[blk].id;
    fat1[blk].id = FREE;
    bitmap_free_block(blk); // 回收bitmap
    if (next == END)
      break;
    blk = next;
  }
  memset(&dirfcb[idx], 0, sizeof(fcb));
  dirfcb[idx].free = 0;
  // 更新父目录length
  openfilelist[cur_fd].length = 0;
  dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  for (int i = 0; i < BLOCKSIZE / sizeof(fcb); i++)
    if (dirfcb[i].free == 1 && dirfcb[i].filename[0] != '\0')
      openfilelist[cur_fd].length += sizeof(fcb);
  printf("文件 %s 删除成功！\n", filename);
}

// 打开文件
int my_open(char *filename)
{
  int cur_fd = ptrcurdir - openfilelist;
  fcb *dirfcb = (fcb *)(myvhard + openfilelist[cur_fd].first * BLOCKSIZE);
  int num_entries = BLOCKSIZE / sizeof(fcb);
  // 检查是否已打开
  for (int i = 0; i < MAXOPENFILE; i++)
  {
    if (openfilelist[i].topenfile && strcmp(openfilelist[i].filename, filename) == 0)
    {
      printf("文件已打开！\n");
      return -1;
    }
  }
  int idx = -1;
  for (int i = 0; i < num_entries; i++)
  {
    if (dirfcb[i].free && strcmp(dirfcb[i].filename, filename) == 0 && dirfcb[i].attribute == 1)
    {
      idx = i;
      break;
    }
  }
  if (idx == -1)
  {
    printf("文件 %s 不存在！\n", filename);
    return -1;
  }
  // 查找空闲打开文件表项
  int free_fd = -1;
  for (int i = 0; i < MAXOPENFILE; i++)
  {
    if (!openfilelist[i].topenfile)
    {
      free_fd = i;
      break;
    }
  }
  if (free_fd == -1)
  {
    printf("打开文件表已满！\n");
    return -1;
  }
  // 填写打开文件表项
  useropen *u = &openfilelist[free_fd];
  memset(u, 0, sizeof(useropen));
  strncpy(u->filename, dirfcb[idx].filename, sizeof(u->filename) - 1);
  u->attribute = 1;
  u->first = dirfcb[idx].first;
  u->length = dirfcb[idx].length;
  u->free = 1;
  u->dirno = openfilelist[cur_fd].first;
  u->diroff = idx;
  u->count = 0;
  u->fcbstate = 0;
  u->topenfile = 1;
  printf("文件 %s 已打开，fd=%d\n", filename, free_fd);
  return free_fd;
}

// 关闭文件
void my_close(int fd)
{
  if (fd < 0 || fd >= MAXOPENFILE)
  {
    printf("无效的文件描述符！\n");
    return;
  }
  useropen *u = &openfilelist[fd];
  if (!u->topenfile)
  {
    printf("文件未打开！\n");
    return;
  }
  // 若FCB被修改，写回目录项
  if (u->fcbstate)
  {
    fcb *dirfcb = (fcb *)(myvhard + u->dirno * BLOCKSIZE);
    memcpy(&dirfcb[u->diroff], u, sizeof(fcb));
  }
  // 更新length到目录项
  if (u->fcbstate)
  {
    fcb *dirfcb = (fcb *)(myvhard + u->dirno * BLOCKSIZE);
    dirfcb[u->diroff].length = u->length;
  }
  memset(u, 0, sizeof(useropen));
  printf("文件描述符 %d 已关闭\n", fd);
}

// 写文件
int my_write(int fd)
{
  if (fd < 0 || fd >= MAXOPENFILE)
  {
    printf("文件描述符无效！\n");
    return -1;
  }
  useropen *u = &openfilelist[fd];
  if (!u->topenfile)
  {
    printf("文件未打开！\n");
    return -1;
  }
  printf("请选择写入方式（1-截断写，2-覆盖写，3-追加写）：");
  int wstyle = 0;
  scanf("%d", &wstyle);
  getchar(); // 吃掉回车
  if (wstyle == 1)
  { // 截断写
    // 释放除首块外的所有块
    fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
    int blk = u->first;
    if (blk != END)
    {
      int next = fat1[blk].id;
      while (next != END)
      {
        int tmp = fat1[next].id;
        fat1[next].id = FREE;
        next = tmp;
      }
      fat1[blk].id = END;
    }
    u->length = 0;
    u->count = 0;
  }
  else if (wstyle == 2)
  { // 覆盖写
    u->count = 0;
  }
  else if (wstyle == 3)
  { // 追加写
    u->count = u->length;
  }
  else
  {
    printf("无效写入方式！\n");
    return -1;
  }
  printf("请输入要写入的内容（以#结束）：\n");
  char text[BLOCKSIZE * 4] = {0};
  int len = 0;
  char ch;
  while ((ch = getchar()) != '#')
  {
    if (ch == EOF)
      break;
    if (len < BLOCKSIZE * 4 - 1)
      text[len++] = ch;
  }
  text[len] = '\0';
  int written = do_write(fd, text, len);
  if (written >= 0)
    printf("写入了 %d 字节。\n", written);
  else
    printf("写入失败！\n");
  return written;
}

// 读文件
int my_read(int fd, int len)
{
  if (fd < 0 || fd >= MAXOPENFILE)
  {
    printf("文件描述符无效！\n");
    return -1;
  }
  useropen *u = &openfilelist[fd];
  if (!u->topenfile)
  {
    printf("文件未打开！\n");
    return -1;
  }
  char *text = (char *)malloc(len + 1);
  int readed = do_read(fd, text, len);
  if (readed > 0)
  {
    text[readed] = '\0';
    printf("文件内容：\n%s\n", text);
  }
  free(text);
  return readed;
}

// 实际写文件
int do_write(int fd, char *text, int len)
{
  if (fd < 0 || fd >= MAXOPENFILE || !text || len <= 0)
    return -1;
  useropen *u = &openfilelist[fd];
  if (!u->topenfile)
    return -1;
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  int file_pos = u->count;
  int blk_offset = file_pos % BLOCKSIZE;
  int skip = file_pos / BLOCKSIZE;
  int current_block = u->first;
  int remaining = skip;
  while (remaining-- > 0 && current_block != END)
  {
    if (current_block < 0 || current_block >= SIZE / BLOCKSIZE)
      return -1;
    current_block = fat1[current_block].id;
  }
  int blk = current_block;
  int offset = 0;
  int left = len;
  int written = 0;
  while (left > 0)
  {
    if (blk == END || blk < 0 || blk >= SIZE / BLOCKSIZE)
    {
      int new_blk = bitmap_alloc_block();
      if (new_blk == -1)
        break;
      fat1[new_blk].id = END;
      if (u->first == END)
      {
        u->first = new_blk;
        blk = new_blk;
      }
      else
      {
        int last_blk = u->first;
        while (fat1[last_blk].id != END)
        {
          last_blk = fat1[last_blk].id;
        }
        fat1[last_blk].id = new_blk;
        blk = new_blk;
      }
      blk_offset = 0;
    }
    unsigned char *blk_addr = myvhard + blk * BLOCKSIZE;
    int space_in_blk = BLOCKSIZE - blk_offset;
    int to_write = (left < space_in_blk) ? left : space_in_blk;
    memcpy(blk_addr + blk_offset, text + offset, to_write);
    offset += to_write;
    left -= to_write;
    written += to_write;
    blk_offset = 0;
    blk = fat1[blk].id;
  }
  u->count += written;
  if (u->count > u->length)
    u->length = u->count;
  u->fcbstate = 1;
  return written;
}

// 实际读文件
int do_read(int fd, char *text, int len)
{
  if (fd < 0 || fd >= MAXOPENFILE || !text || len <= 0)
    return -1;
  useropen *u = &openfilelist[fd];
  if (!u->topenfile)
    return -1;
  if (u->count >= u->length)
    return 0;
  fat *fat1 = (fat *)(myvhard + BLOCKSIZE * 1);
  int file_pos = u->count;
  int blk_offset = file_pos % BLOCKSIZE;
  int skip = file_pos / BLOCKSIZE;
  int current_block = u->first;
  int remaining = skip;
  while (remaining-- > 0 && current_block != END)
  {
    if (current_block < 0 || current_block >= SIZE / BLOCKSIZE)
      return -1;
    current_block = fat1[current_block].id;
  }
  int blk = current_block;
  int offset = 0;
  int left = (len > u->length - u->count) ? (u->length - u->count) : len;
  int readed = 0;
  while (blk != END && left > 0)
  {
    if (blk < 0 || blk >= SIZE / BLOCKSIZE)
      break;
    unsigned char *blk_addr = myvhard + blk * BLOCKSIZE;
    int available = BLOCKSIZE - blk_offset;
    int to_read = (left < available) ? left : available;
    memcpy(text + offset, blk_addr + blk_offset, to_read);
    offset += to_read;
    left -= to_read;
    readed += to_read;
    blk_offset = 0;
    blk = fat1[blk].id;
  }
  u->count += readed;
  return readed;
}

// 位示图操作函数
int bitmap_alloc_block()
{
  for (int i = 0; i < BLOCKNUM; i++)
  {
    int byte = i / 8, bit = i % 8;
    if (!(bitmap[byte] & (1 << bit)))
    {
      bitmap[byte] |= (1 << bit);
      return i;
    }
  }
  return -1;
}
void bitmap_free_block(int blk)
{
  int byte = blk / 8, bit = blk % 8;
  bitmap[byte] &= ~(1 << bit);
}
int bitmap_is_used(int blk)
{
  int byte = blk / 8, bit = blk % 8;
  return (bitmap[byte] & (1 << bit)) != 0;
}

// 退出文件系统
void my_exitsys()
{
  FILE *fp = fopen("myfsys", "wb+");
  if (!fp)
  {
    printf("保存文件系统失败！\n");
    return;
  }
  // 同步所有打开文件的FCB到磁盘
  for (int i = 0; i < MAXOPENFILE; i++)
  {
    if (openfilelist[i].topenfile && openfilelist[i].fcbstate)
    {
      fcb *dirfcb = (fcb *)(myvhard + openfilelist[i].dirno * BLOCKSIZE);
      memcpy(&dirfcb[openfilelist[i].diroff], &openfilelist[i], sizeof(fcb));
    }
  }
  fwrite(myvhard, 1, SIZE, fp);
  fclose(fp);
  free(myvhard);
  myvhard = NULL;
  memset(openfilelist, 0, sizeof(openfilelist));
  printf("文件系统已保存并退出。\n");
  exit(0);
}
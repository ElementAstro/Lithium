# 从零入门 C 语言： Day5 - 标准输入

## 引入

在 C 语言中，输入是从用户或文件等外部源获取数据并存储到程序变量中的过程。C 语言提供了多种方式来获取输入数据，包括标准输入函数、文件输入函数以及低级别的系统输入函数。

## 标准输入函数

### `scanf`

`scanf`是 C 语言中最常用的标准输入函数，它允许从标准输入（通常是键盘）中读取格式化的数据，并将这些数据存储到变量中。

```c
int scanf(const char *format, ...);
```

- format：指定要读取的输入数据类型的格式字符串（例如"%d"表示整数，"%f"表示浮点数）。
- 返回值：返回成功读取的变量数量。如果读取失败，返回值为 EOF。

举个例子：

```c
#include <stdio.h>

int main() {
    int num;
    float f;

    printf("Enter an integer and a float: ");
    scanf("%d %f", &num, &f);

    printf("You entered: %d and %.2f\n", num, f);
    return 0;
}
```

需要注意的是：

- scanf 需要传入变量的地址（使用&符号），因为它需要修改这些变量的值。
- scanf 会忽略输入数据中的空格、换行符和制表符，但可以用空格等分隔符来读取多项数据。
- 如果输入的格式与指定的格式不匹配，可能导致读取失败或数据错误。

### `fscanf`

fscanf 与 scanf 类似，但它是从文件流中读取格式化数据。

```c
int fscanf(FILE *stream, const char *format, ...);
```

- stream：文件流指针，指定要读取数据的文件。
- 其他参数与 scanf 相同。

来个基础示例

```c
#include <stdio.h>

int main() {
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    int num;
    fscanf(file, "%d", &num);
    printf("Number from file: %d\n", num);

    fclose(file);
    return 0;
}
```

切记，使用 fscanf 时，确保文件已成功打开，并在使用完毕后 **正确关闭文件**。

### `sscanf`

从名字可以看出，sscanf 从字符串中读取格式化数据，而不是从标准输入或文件。

```c
int sscanf(const char *str, const char *format, ...);
```

- str：要读取数据的字符串。
- 其他参数与 scanf 相同。

```c
#include <stdio.h>

int main() {
    char input[] = "42 3.14";
    int num;
    float f;

    sscanf(input, "%d %f", &num, &f);
    printf("Parsed values: %d and %.2f\n", num, f);

    return 0;
}
```

## 字符输入函数

### `getchar`

getchar 从标准输入中读取一个字符。它是一个简单的字符输入函数，通常用于逐字符读取输入。

```c
int getchar(void);
```

- 返回值：返回读取的字符（作为 int 类型）。如果遇到输入结束（EOF），返回 EOF。

```c
#include <stdio.h>

int main() {
    char c;

    printf("Enter a character: ");
    c = getchar();

    printf("You entered: %c\n", c);
    return 0;
}
```

需要格外注意的是，getchar 不会跳过空格和换行符，它会逐字符读取每一个输入字符。由于返回值为 int，你需要将其转换为 char 类型来使用。

### `fgetc`

fgetc 与 getchar 类似，但它用于从文件中读取一个字符。

```c
int fgetc(FILE *stream);
```

- stream：要从中读取字符的文件流指针。
- 返回值：返回读取的字符（作为 int 类型）。如果遇到文件结束或错误，返回 EOF。

```c
#include <stdio.h>

int main() {
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    char c;
    while ((c = fgetc(file)) != EOF) {
        putchar(c); // 打印读取的字符
    }

    fclose(file);
    return 0;
}
```

注意点：

- fgetc 用于从文件流中逐字符读取数据，适用于处理文件内容的逐行或逐字符处理。
- 读取时会自动移动文件指针，逐字符读取下一个字符。

### `getc`

getc 与 fgetc 功能相同，但可能实现方式略有不同。getc 通常用于从文件流中读取字符。

```c
int getc(FILE *stream);
```

- stream：文件流指针。
- 返回值：返回读取的字符（作为 int 类型），或在遇到 EOF 时返回 EOF。

```c
#include <stdio.h>

int main() {
    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    char c;
    while ((c = getc(file)) != EOF) {
        putchar(c); // 打印读取的字符
    }

    fclose(file);
    return 0;
}
```

### `ungetc`

ungetc 将字符“放回”到输入流中，使其成为下一个要读取的字符。这在某些解析场景中非常有用。

```c
int ungetc(int c, FILE *stream);
```

- c：要放回的字符。
- stream：文件流指针。
- 返回值：返回放回的字符，若失败返回 EOF。

```c
#include <stdio.h>

int main() {
    int c;
    FILE *file = fopen("input.txt", "r");

    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    c = fgetc(file);
    if (c != EOF) {
        ungetc(c, file); // 将字符放回流中
        c = fgetc(file); // 再次读取同一个字符
        printf("Character read again: %c\n", c);
    }

    fclose(file);
    return 0;
}
```

ungetc 最多只能将一个字符放回流中，放回后可以再次读取同一个字符。
在使用 ungetc 之前，**确保放回的字符与流的状态一致**。

## 行输入函数

### `gets`（不推荐）

gets 从标准输入读取一行字符串，直到遇到换行符或文件结束符。由于存在缓冲区溢出的风险，**gets 已被 C11 标准弃用**。

```c
char *gets(char *str);
```

- str：指向接收输入字符串的缓冲区指针。
- 返回值：返回输入字符串指针（str），如遇到 EOF 则返回 NULL。

Outdated!!!

```c
#include <stdio.h>

int main() {
    char str[100];

    printf("Enter a string: ");
    gets(str);

    printf("You entered: %s\n", str);
    return 0;
}
```

由于 gets 不检查输入是否超过缓冲区大小，可能导致缓冲区溢出并引发安全问题，强烈建议不要使用 gets，应使用更安全的 fgets。

### `fgets`

fgets 是读取一行输入的推荐方法，它可以避免缓冲区溢出问题。

```c
char *fgets(char *str, int n, FILE *stream);
```

- str：指向接收输入字符串的缓冲区指针。
- n：要读取的最大字符数（包括终止符）。
- stream：文件流指针，stdin 表示标准输入。
- 返回值：返回 str，如果遇到 EOF 或发生错误，返回 NULL。

```c
#include <stdio.h>

int main() {
    char str[100];

    printf("Enter a string: ");
    if (fgets(str, 100, stdin) != NULL) {
        printf("You entered: %s", str);
    } else {
        printf("Error reading input.\n");
    }

    return 0;
}
```

注意,fgets 会保留输入的换行符\n，如果你不想保留换行符，可以手动去掉它：

```c
str[strcspn(str, "\n")] = '\0';
```

fgets 是 **安全** 的，适合用于读取输入行或处理来自文件的文本行。

## 低级别输入函数

C 语言还提供了一些低级别的输入函数，这些函数直接与操作系统交互，通常用于更底层的输入/输出操作。

### `read`

read 是 UNIX 系统调用，用于从文件描述符读取原始数据。它允许对低级别的文件操作进行更多控制，通常在系统编程中使用。

```c
ssize_t read(int fd, void *buf, size_t count);
```

- fd：文件描述符，表示从哪个文件或输入源读取数据。
- buf：指向接收读取数据的缓冲区指针。
- count：要读取的最大字节数。
- 返回值：返回读取的字节数，如果遇到 EOF 返回 0，如遇错误返回-1。

```c
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

int main() {
    char buffer[100];
    int fd = open("input.txt", O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    ssize_t bytesRead = read(fd, buffer, sizeof(buffer) - 1);

    if (bytesRead == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    buffer[bytesRead] = '\0'; // 确保字符串以空字符结束
    printf("Read from file: %s\n", buffer);

    close(fd);
    return 0;
}
```

贴近操作系统是有代价的：不会自动处理文本行结束符或进行缓冲区管理，因此需要手动管理输入数据。

### `getch` 和 `getche`

这两个函数用于从标准输入读取单个字符，并且不需要按下回车键即可输入。它们通常用于处理键盘输入，特别是在控制台应用程序中。

- getch：读取单个字符，不显示在屏幕上。
- getche：读取单个字符，并显示在屏幕上。

这些函数不是标准 C 库的一部分，通常在 Windows 环境中由`conio.h`提供。

```c
int getch(void);
int getche(void);
```

```c
#include <stdio.h>
#include <conio.h>

int main() {
    char c;

    printf("Press a key: ");
    c = getch();  // 使用 getch() 获取字符，但不显示
    printf("\nYou pressed: %c\n", c);

    printf("Press another key: ");
    c = getche();  // 使用 getche() 获取字符，并显示
    printf("\nYou pressed: %c\n", c);

    return 0;
}
```

**Windows Only!**

## 文件输入函数

C 语言还提供了一些用于从文件中读取数据的函数。

### `fread`

fread 用于从文件中读取原始数据块，它通常用于二进制文件的读取。

```c
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
```

- ptr：指向接收读取数据的缓冲区指针。
- size：每个数据块的大小（字节数）。
- nmemb：要读取的块的数量。
- stream：文件流指针。
- 返回值：返回成功读取的块数。

```c
#include <stdio.h>

int main() {
    FILE *file = fopen("data.bin", "rb");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    int data[5];
    size_t bytesRead = fread(data, sizeof(int), 5, file);

    for (size_t i = 0; i < bytesRead; i++) {
        printf("data[%zu] = %d\n", i, data[i]);
    }

    fclose(file);
    return 0;
}
```

- fread 直接读取原始数据块，而不是格式化的数据。适用于读取二进制文件或自定义文件格式。
- 确保缓冲区足够大以容纳读取的数据，并检查返回值以确定是否成功读取。

### `getline`

getline 用于从文件或标准输入中读取整行数据，包括换行符。它动态分配缓冲区以适应读取的数据行大小。

```c
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
```

- lineptr：指向缓冲区的指针指针。如果指针指向 NULL，getline 会自动分配缓冲区。
- n：指向缓冲区大小的指针。
- stream：文件流指针。
- 返回值：返回读取的字符数（包括换行符），如果遇到 EOF 返回-1。

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    FILE *file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1;
    }

    while ((read = getline(&line, &len, file)) != -1) {
        printf("Retrieved line of length %zu: %s", read, line);
    }

    free(line);
    fclose(file);
    return 0;
}
```

**getline 是 POSIX 标准的扩展函数**

## 安全输入（以`scanf_s`为例）

scanf_s 是 scanf 函数的安全版本，它通过增加对输入长度的控制，来避免输入数据超过目标缓冲区的大小，从而降低缓冲区溢出的风险。

```c
int scanf_s(const char *format, ...);
```

- format：格式字符串，与 scanf 的格式字符串类似，用于指定输入的数据类型。
- ...：可变参数，指定要存储输入数据的变量地址。对于某些类型的输入（如字符串或字符数组），需要额外指定目标缓冲区的大小。

```c
#include <stdio.h>

int main() {
    char buffer[10];
    int num;

    printf("Enter a number and a string: ");
    scanf_s("%d %9s", &num, buffer, sizeof(buffer));

    printf("You entered: %d and %s\n", num, buffer);
    return 0;
}
```

在这个示例中，%d 用于读取一个整数，%9s 用于读取最多 9 个字符的字符串。字符串后面的 sizeof(buffer) 指定了缓冲区的大小，以确保不会读入超过缓冲区容量的字符串。
scanf_s 在读取字符串时要求提供额外的参数，指定目标缓冲区的大小。这个参数是 **必需** 的。

## 总结

C 语言提供了多种输入方式来满足不同的需求，从简单的标准输入函数到复杂的文件输入函数，每种方式都有其适用场景和使用注意点：

- 标准输入函数：scanf、fscanf、sscanf 用于读取格式化的数据，适合从标准输入或文件中获取结构化数据。
- 字符输入函数：getchar、fgetc、getc 用于逐字符读取输入，适合处理逐字符输入或文件内容。
- 行输入函数：fgets 和 getline 用于读取整行数据，fgets 安全性高，getline 适合处理动态长度行。
- 低级别输入函数：read、getch、getche 用于系统级编程或控制台输入，适合需要直接与硬件或操作系统交互的场景。
- 文件输入函数：fread 用于读取二进制数据块，适合处理二进制文件或自定义格式的文件。

掌握这些输入函数，可以让你在 C 语言编程中灵活地处理各种输入数据，满足不同的编程需要 AwA

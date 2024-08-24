# 从零入门 C 语言： Day4 - 你是个什么东西

## 引入

学习 C 语言时，理解类型系统和类型转换是非常重要的。它们帮助你掌握如何存储和操作不同类型的数据。在接下来的讲解中，我会以简单生动的方式带你一探 C 语言类型系统的奥秘，并通过示例加深理解。

## 类型系统

C 语言是一门强类型语言，这意味着每一个变量在使用之前都必须有一个确定的类型。数据类型决定了变量可以存储什么样的数据，以及在内存中占据多少空间。

### 基本数据类型

#### 整型（Integer Types）

- int：标准整型，通常占用 4 字节内存。
- short：短整型，通常占用 2 字节。
- long：长整型，通常占用 4 或 8 字节。
- long long：更长的整型，通常占用 8 字节。

```c
int age = 25;
short year = 2022;
long population = 8000000L;
long long distance = 12345678901234LL;
```

#### 字符型（Character Type）

char：用于存储单个字符，占用 1 字节内存。

```c
char letter = 'A';
```

#### 浮点型（Floating Point Types）

- float：单精度浮点型，通常占用 4 字节内存。
- double：双精度浮点型，通常占用 8 字节内存。
- long double：扩展精度浮点型，通常占用 12 或 16 字节。

```c
float pi = 3.14f;
double e = 2.718281828459045;
```

#### 枚举类型（Enumerated Types）

枚举类型是一种用户定义的类型，用于定义一组命名的整型常量。主要是为了更好的描述数据，并简化代码。

```c
enum Day { SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
enum Day today = WEDNESDAY;
```

#### void 类型

void 类型表示“无类型”，通常用于函数返回类型表示该函数不返回任何值。**注意：部分教程说函数返回值 void 可以省略并不是好习惯！**

```c
void sayHello() {
    printf("Hello, World!\n");
}
```

#### 指针类型

指针是 C 语言中的重要类型，用于存储内存地址，也是 c 语言入门学习的噩梦之一。指针的类型决定了它指向的变量类型。

```c
int x = 10;
int *ptr = &x; // ptr 是一个指向整数的指针，存储 x 的地址
```

### 类型转换

在 C 语言中，有时候需要将一种类型的数据转换为另一种类型的数据。类型转换可以分为`隐式类型转换`和`显式类型转换`。

#### 隐式类型转换

当你在表达式中混合使用不同类型的变量时，编译器会自动将它们转换为相同的类型，以避免数据损失。这种转换称为隐式类型转换。说白了就是编译器帮你自动添加了一些代码，完成了这个任务，不需要你来操心！

```c
int a = 5;
double b = 3.2;
double result = a + b; // a 被隐式转换为 double 类型
```

但是需要注意的是，并不是所有同类类型都可以隐式转换！编译器通常会将“窄”类型（如 int）转换为“宽”类型（如 double），以确保精度。

#### 显式类型转换（强制类型转换）

有时候，常规的隐式转换已经不能满足需求，你需要手动将一种类型的数据转换为另一种类型，这被称为显式类型转换。具体的语法形式为：

```c
new_type variable = (new_type)expression;
```

```c
double pi = 3.14159;
int truncated_pi = (int)pi; // 将 double 转换为 int，结果为3
```

这时候你可能发现了，pi 的小数部分被抹去了，因此强制转换可能会导致数据丢失，所以使用强制转换时要小心，确保了解数据的潜在变化。

### 来点例子

- 整数到浮点数的转换

```c
#include <stdio.h>

int main() {
    int apples = 10;
    double price = 1.5;
    double total_cost = apples * price; // 隐式类型转换，apples 被转换为 double
    printf("Total cost: $%.2f\n", total_cost);
    return 0;
}
```

在这个例子中，`apples` 是一个整型，`price` 是一个浮点型。在计算`total_cost`时，`apples` 被隐式转换为 **double** 类型以进行浮点数乘法运算。

- 强制类型转换

```c
#include <stdio.h>

int main() {
    double average = 85.6;
    int rounded_average = (int)average; // 强制转换为 int，结果为 85
    printf("Rounded average: %d\n", rounded_average);
    return 0;
}
```

`average` 是一个`double`类型。在将其转换为`int`时，强制转换 **截断了小数部分** ，使得`rounded_average`变为 85。

- 指针类型转换

```c
#include <stdio.h>

int main() {
    int a = 42;
    void *ptr = &a; // 将 int 指针转换为 void 指针
    int *int_ptr = (int *)ptr; // 强制转换回 int 指针
    printf("Value of a: %d\n", *int_ptr);
    return 0;
}
```

这里，`ptr` 原本是一个`void`指针，可以指向任何类型的变量。我们将它强制转换回 int 指针，以正确地访问变量 a 的值。

### 陷阱和注意点

- 溢出问题：在类型转换时，特别是从大类型转换为小类型时，要注意溢出问题。例如，将一个 long 值强制转换为 short 时，如果 long 值超出了 short 的范围，可能会得到一个错误的结果。

```c
long big_number = 1234567890L;
short small_number = (short)big_number; // 可能导致溢出，结果不可预测
```

- 截断问题：当从浮点类型转换为整数类型时，小数部分会被截断。要确保这种截断是你想要的结果。

```c
double pi = 3.14159;
int truncated_pi = (int)pi; // 结果为 3，截断了小数部分
```

- 指针类型的转换：指针的类型转换需要特别小心。如果你将一个指向某种类型的指针转换为另一种类型的指针，可能会导致未定义行为。

```c
int a = 10;
double *ptr = (double *)&a; // 可能导致指针错误访问
```

## 数组

刚刚我们讲解的都是单个变量，那么如果我要存储一组类型相同的变量，难道只能够定义 n 个变量吗？显然不用，这个时候就要请出数组了！

数组是 C 语言中一个非常重要的概念，它让我们可以处理一组相同类型的数据。数组在内存中是连续存储的，可以高效地管理和访问大量数据。

### 一维数组

#### 定义和初始化

一维数组是最基本的数组类型，存储一组相同类型的数据。

```c
int arr[5]; // 定义一个包含5个整型元素的数组，中括号内的数字即为数组大小
```

你可以在定义数组时进行初始化：

```c
int arr[5] = {1, 2, 3, 4, 5}; // 初始化一个包含5个元素的数组
```

如果你不指定数组的大小，编译器会根据初始化列表自动推导大小：

```c
int arr[] = {1, 2, 3}; // 自动推导数组大小为3
```

需要注意的是，如果你部分初始化数组，那么未初始化的元素会被设置为 0。

```c
int arr[5] = {1, 2}; // 剩余元素自动初始化为0
```

#### 访问元素

这里很明确，我们可以通过数组的索引来访问和修改元素。这里有一个大坑， **索引从 0 开始** ，初学者往往会习惯性认为索引从 1 开始而导致错误。

```c
int x = arr[2]; // 获取数组的第三个元素，值为3
arr[0] = 10;    // 将第一个元素的值改为10
```

### 多维数组

多维数组用于存储表格或矩阵形式的数据。其中，二维数组是最常见的多维数组。

#### 定义&初始化

二维数组可以看作是“数组的数组”，即每个元素本身又是一个数组。来个简单的例子：

```c
int matrix[3][4]; // 定义一个3行4列的二维数组
```

同理，二维数组的初始化可以通过嵌套的花括号来完成：

```c
int matrix[3][4] = {
    {1, 2, 3, 4},
    {5, 6, 7, 8},
    {9, 10, 11, 12}
};
```

当然，如果你不明确行列也是可以的，编译器会自动处理行和列的关联，就像下面这样：

```c
int matrix[3][4] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
```

#### 访问

由访问一维数组需要一个索引可以推断，访问二维数组的元素需要两个索引，分别表示行和列。

```c
int value = matrix[1][2]; // 获取第二行第三列的值，值为7
matrix[2][3] = 15;        // 将第三行第四列的值设为15
```

多维数组的元素在内存中是连续存储的，**行优先（row-major）** 顺序。即第一行的所有元素存储在内存中，然后是第二行，依此类推。

### 指针数组和数组指针

#### 指针数组

指针数组是一个数组，其中的每个元素都是指针。它通常用于存储字符串数组或其他需要动态分配内存的数据。

```c
char *strArr[3]; // 定义一个字符指针数组
strArr[0] = "Hello";
strArr[1] = "World";
strArr[2] = "C";
```

你可以通过数组索引访问这些字符串：

```c
printf("%s\n", strArr[1]); // 输出 "World"
```

#### 数组指针

数组指针是指向数组的指针。它允许我们通过指针操作数组。

```c
int arr[5] = {1, 2, 3, 4, 5};
int (*p)[5] = &arr; // p 是一个指向包含5个整数的数组的指针
```

你可以通过指针访问数组元素：

```c
int x = (*p)[2]; // 获取第三个元素，值为3
```

#### 指针运算

由于数组名实际上是指向数组首元素的指针，因此可以进行指针运算：

```c
int arr[5] = {1, 2, 3, 4, 5};
int *p = arr;

printf("%d\n", *(p + 2)); // 输出3，即arr[2]
```

p + 2 表示指针 p 向后移动两个元素，然后通过\*解引用访问该元素的值。

### 二级数组（指针的数组）

二级数组通常用于表示指针的数组，特别是在处理二维数组或指针数组时。

#### 定义 and 用法

一个简单的二级数组是一个指向指针的指针（char **或 int **）。这种结构可以用于 **动态分配** 二维数组或处理字符指针数组。

```c
char *lines[] = {"line1", "line2", "line3"};
char **p = lines; // p 是一个指向字符指针的指针
```

你可以使用二级指针访问或修改指针数组的内容：

```c
printf("%s\n", p[1]); // 输出 "line2"
p[2] = "new line3";   // 修改第三个指针所指向的字符串
```

#### 动态分配

二级指针非常适合动态分配二维数组，下面是一个典型例子：

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int rows = 3; // 矩阵的行数
    int cols = 4; // 矩阵的列数

    // 动态分配一个指向指针的数组，用于存储每一行的指针
    int **matrix = (int **)malloc(rows * sizeof(int *));

    // 为每一行动态分配列数的内存
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int *)malloc(cols * sizeof(int));
    }

    // 初始化矩阵
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = i * cols + j; // 将矩阵元素赋值为它的线性索引
        }
    }

    // 打印矩阵
    printf("矩阵内容：\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf("%d ", matrix[i][j]); // 打印每个元素
        }
        printf("\n"); // 换行
    }

    // 释放内存
    for (int i = 0; i < rows; i++) {
        free(matrix[i]); // 释放每一行的内存
    }
    free(matrix); // 释放存储行指针的数组

    return 0; // 返回成功状态
}
```

```txt
0 1 2 3
4 5 6 7
8 9 10 11
```

### 柔性数组（Flexible Array Members）

柔性数组成员是一种 C99 标准引入的高级特性，允许在结构体中定义一个可变长度的数组。这种数组没有固定的大小，需要通过动态内存分配来使用。

#### 定义&使用

```c
#include <stdio.h>
#include <stdlib.h>

struct FlexibleArray {
    int length;
    int array[]; // 柔性数组成员
};

int main() {
    int n = 5;

    // 动态分配内存，包含结构体和数组的总大小
    struct FlexibleArray *fa = malloc(sizeof(struct FlexibleArray) + n * sizeof(int));

    fa->length = n;
    for (int i = 0; i < n; i++) {
        fa->array[i] = i * 2; // 初始化数组
    }

    // 打印数组
    for (int i = 0; i < fa->length; i++) {
        printf("%d ", fa->array[i]);
    }
    printf("\n");

    free(fa); // 释放内存

    return 0;
}
```

```txt
0 2 4 6 8
```

需要格外注意的是，柔性数组成员 array[]在定义时不指定大小，内存分配时根据需要来决定数组的实际大小！

### 静态数组和动态数组

其实，上面的很多例子中已经出现了这两个概念，那么下面就是具体的讲解。

#### 静态数组

静态数组是在 **编译时** 分配内存的，数组的大小在程序编译时就已经确定，并且在程序的生命周期内保持不变。静态数组通常分配在栈内存中，大小是 **固定的**。

```c
int static_array[10]; // 定义一个大小为10的静态数组
```

优点：

- 访问速度快，因为它们位于栈内存中。
- 不需要手动管理内存，编译器会自动处理内存的分配和释放。

缺点：

- 大小固定，一旦定义，无法在程序运行时更改。
- 如果数组很大，可能导致栈溢出，特别是在嵌套调用较深的情况下。

#### 动态数组

动态数组是在程序运行时根据需要动态分配内存的。它的大小可以在运行时确定，并且可以在程序的不同阶段分配和释放内存。动态数组通常分配在堆内存中。

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int n = 10;
    int *dynamic_array = (int *)malloc(n * sizeof(int)); // 动态分配一个大小为n的数组

    if (dynamic_array == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    // 使用数组
    for (int i = 0; i < n; i++) {
        dynamic_array[i] = i * 2;
    }

    // 打印数组
    for (int i = 0; i < n; i++) {
        printf("%d ", dynamic_array[i]);
    }
    printf("\n");

    // 释放内存
    free(dynamic_array);

    return 0;
}
```

优点：

- 数组的大小可以在运行时动态调整。
- 适合处理需要在运行时确定大小的大型数据集。

缺点：

- 需要手动管理内存，必须确保在使用完后释放内存（使用 free 函数），否则会导致内存泄漏。
- 动态内存分配相比静态内存分配速度稍慢，因为它涉及到系统调用。

### 函数中的数组参数

当数组作为参数传递给函数时，它实际上是将数组的指针传递给函数。因此，函数内对数组的任何修改会影响到原数组。

```c
void modifyArray(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * 2;
    }
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    modifyArray(arr, 5);

    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]); // 输出2 4 6 8 10
    }
    return 0;
}
```

### 复杂一点的特性

#### 动态调整数组大小：realloc

动态数组的大小可以在运行时通过 `realloc`函数调整。这允许你在程序运行期间根据需要扩展或缩小数组的大小。

```c
#include <stdio.h>
#include <stdlib.h>

int main() {
    int *arr = (int *)malloc(5 * sizeof(int));

    for (int i = 0; i < 5; i++) {
        arr[i] = i;
    }

    // 调整数组大小
    arr = (int *)realloc(arr, 10 * sizeof(int));

    for (int i = 5; i < 10; i++) {
        arr[i] = i * 2;
    }

    // 打印数组
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");

    free(arr);
    return 0;
}
```

需要注意，realloc 可能会移动数组到新的内存位置，因此返回的指针可能不同于原来的指针。如果 realloc 失败，返回 NULL，旧的内存保持不变。

### 数组中的注意点

- 数组越界

数组越界是 C 语言中常见且危险的错误，访问数组边界之外的内存可能导致未定义行为或程序崩溃。通常情况下你会获得名为 Segmentation Fault 的错误，也就是臭名昭著的段错误!

```c
int arr[5] = {1, 2, 3, 4, 5};
int value = arr[5]; // 错误：越界访问，而且是由最容易犯得记错索引导致的
```

- 指针和数组的关系

数组名在大多数表达式中会被转换为指向其第一个元素的指针。例如：

```c
int arr[5] = {1, 2, 3, 4, 5};
int *p = arr; // p 指向 arr[0]
```

但是，数组名并不是指针，它是一个常量指针，不能被修改。

## 字符串

很多时候我们需要让变量能够存储一个句子，那么就需要用到字符串了。C 语言中的字符串体系是一个非常重要的概念，因为它涉及到如何存储、处理和操作文本数据。在 C 语言中，字符串并不像在其他高级语言中那样是一个独立的数据类型，而是一个字符数组。

### 表示字符串

在 C 语言中，字符串是由字符数组表示的，且以 **空字符（\0）** 结尾。空字符标志着字符串的结束， **因此它的长度比实际字符数多一位**。

### 定义与初始化

```c
char str1[] = "Hello, World!";
```

这里我们并没有添加\0，但是仍然是正确的，因为编译器会自动添加\0，因此 str1 的实际大小是 14 个字符（包括\0）。这种初始化方式是最常用的，也是最安全的，因为它 **自动处理了字符串的结束标志**。

当然了，如果你不嫌麻烦，也可以逐字符初始化一个字符串，虽然这有点呆：

```c
char str2[] = {'H', 'e', 'l', 'l', 'o', '\0'};
```

不过这种方式也更灵活，但方便与灵活不可兼得，这样容易出错，尤其是在手动忘记添加\0 时，当场爆炸。

你还可以定义一个指向字符串的指针：

```c
char *str3 = "Hello, World!";
```

这里 str3 是一个指向字符串常量的指针。需要注意的是，这种方式定义的字符串通常存储在只读内存区，因此不能修改它的内容，可以加上 const 修饰符表明这是一个常量。

### 常见操作

C 语言标准库提供了一组函数来处理字符串。这些函数大多在`<string.h>`头文件中定义。需要注意的是，c 语言提供的很多操作函数其实是不安全的，在

#### `strlen`：获取字符串长度

`strlen 用于获取字符串的长度 **（不包括末尾的\0）**。

```c
#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "Hello, World!";
    int length = strlen(str);
    printf("Length of the string: %d\n", length);
    return 0;
}
```

```txt
Length of the string: 13
```

#### `strcpy`：复制字符串

strcpy 函数将一个字符串复制到另一个字符串中。

```c
#include <stdio.h>
#include <string.h>

int main() {
    char src[] = "Hello";
    char dest[20]; // 确保目标数组足够大，不然会溢出
    strcpy(dest, src);
    printf("Copied string: %s\n", dest);
    return 0;
}
```

```txt
Copied string: Hello
```

#### `strcat`：连接字符串

strcat 函数将两个字符串连接在一起。

```c
#include <stdio.h>
#include <string.h>

int main() {
    char str1[20] = "Hello";
    char str2[] = ", World!";
    strcat(str1, str2); // 确保目标数组有足够的空间存储连接后的字符串，包括空字符\0。
    printf("Concatenated string: %s\n", str1);
    return 0;
}
```

```txt
Concatenated string: Hello, World!
```

#### `strcmp`：比较字符串

strcmp 函数用于比较两个字符串的字典顺序。

```c
#include <stdio.h>
#include <string.h>

int main() {
    char str1[] = "Apple";
    char str2[] = "Banana";
    int result = strcmp(str1, str2);

    if (result < 0) {
        printf("str1 is less than str2\n");
    } else if (result > 0) {
        printf("str1 is greater than str2\n");
    } else {
        printf("str1 is equal to str2\n");
    }
    return 0;
}
```

```txt
str1 is less than str2
```

strcmp 返回一个整数值，如果第一个字符串在字典顺序上小于、等于或大于第二个字符串，则分别返回负值、0 或正值。

#### `strchr` 和 `strstr`：查找字符或子串

strchr 用于查找字符串中某个字符的第一次出现。
strstr 用于查找一个字符串中第一次出现的子串。

```c
#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "Hello, World!";
    char *pos = strchr(str, 'W');

    if (pos != NULL) {
        printf("Character found at position: %ld\n", pos - str);
    } else {
        printf("Character not found.\n");
    }

    char *substr = strstr(str, "World");

    if (substr != NULL) {
        printf("Substring found: %s\n", substr);
    } else {
        printf("Substring not found.\n");
    }

    return 0;
}
```

```txt
Character found at position: 7
Substring found: World!
```

### 字符串操作中的注意点

很多前面都有，但还是反复强调！

#### 字符数组大小

在定义字符数组时，一定要确保数组大小足够容纳字符串和空字符\0。例如：

```c
char str[5] = "Hello"; // 错误：数组长度不足
```

这个例子会导致缓冲区溢出，因为"Hello"需要 6 个字符空间（包括\0）。

#### 字符串常量和可变性

字符串常量（例如"Hello"）通常存储在只读内存中，因此不能通过指针修改它们。如果你尝试修改一个字符串常量，会导致运行时错误：

```c
char *str = "Hello";
str[0] = 'h'; // 错误：可能导致程序崩溃
```

#### 缓冲区溢出

字符串操作时的缓冲区溢出是 C 语言中非常常见且危险的错误。操作字符串时务必确保目标缓冲区的大小足够。例如：

```c
char str1[10] = "Hello";
char str2[] = "World!";
strcat(str1, str2); // 错误：str1 缓冲区溢出
```

这种错误会导致未定义行为，甚至程序崩溃。

### 来几个经典的例子

#### 反转字符串

```c
#include <stdio.h>
#include <string.h>

void reverse(char str[]) {
    int n = strlen(str);
    for (int i = 0; i < n / 2; i++) {
        char temp = str[i];
        str[i] = str[n - i - 1];
        str[n - i - 1] = temp;
    }
}

int main() {
    char str[] = "Hello, World!";
    reverse(str);
    printf("Reversed string: %s\n", str);
    return 0;
}
```

```txt
Reversed string: !dlroW ,olleH
```

#### 检查回文字符串

```c
#include <stdio.h>
#include <string.h>

int isPalindrome(char str[]) {
    int n = strlen(str);
    for (int i = 0; i < n / 2; i++) {
        if (str[i] != str[n - i - 1]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    char str[] = "madam";
    if (isPalindrome(str)) {
        printf("The string is a palindrome.\n");
    } else {
        printf("The string is not a palindrome.\n");
    }
    return 0;
}
```

```txt
The string is a palindrome.
```

## 总结

今天的内容很多，需要好好消化，希望你能够理解并应用到你的实际编程项目中。

**人生是一条不断探索的旅程，充满了选择与变化，体验着喜悦与挑战，最终形成我们独特的故事。**

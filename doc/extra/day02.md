# 从零入门 C 语言： Day2 - 不一样的选择却殊途同归

分支语句在 C 语言中就像是程序的导航系统，告诉代码在不同的条件下该去哪里。这些语句帮助你的程序在不同的情况下选择不同的路线，就像你开车时选择不同的道路以避开交通堵塞。通过这些分支，程序能根据不同的输入或状态选择最合适的代码块，从而提升其灵活性和适应性。C 语言中的分支语句有点像你在饭店点餐时的选项：你可以选择“if”你喜欢这道菜，或者“if-else”你更喜欢另一道菜。还有“else-if”结构，就像在菜单上翻找更多的选择，最后是“switch”语句，就像你在自助餐厅里随机挑选自己想吃的美食。本文将以一种轻松愉快的方式，详细介绍这些分支语句的语法、用法以及注意事项，确保你在编程的旅途中不会迷路。
~~奇怪的比喻增加了~~

## `if` 语句

if 语句是 C 语言中最基础的条件分支控制结构，主要用于判断某个条件是否为真，如果为真则执行特定的代码块。if 语句的条件表达式通常是一个逻辑表达式，其值为 true 或 false。

### 语法

```c
if (条件表达式) {
// 条件为真时执行的代码块
}
```

- 条件表达式：通常为一个布尔表达式。如果条件成立（即结果为 true），则执行大括号内的代码块；否则跳过该代码块。
- 代码块：大括号 {} 内的代码仅在条件为真时执行。

### 举个简单的例子

```c
#include <stdio.h>

int main() {
    int x = 5;

    if (x > 0) {
        printf("x is positive.\n");
    }

    return 0;

}
```

`if (x > 0)` 判断变量 x 是否大于 0。因为 x 的值为 5，条件成立，所以会输出 `x is positive.`。

## `if-else` 语句

if-else 语句是 if 语句的扩展，允许程序在条件不成立时执行另一段代码。即当条件为真时执行 if 分支的代码；当条件为假(false)时，执行 else 分支的代码。

### 语法格式

```c
if (条件表达式) {
// 条件为真时执行的代码块
} else {
// 条件为假时执行的代码块
}
```

### 再来个例子

```c
#include <stdio.h>

int main() {
    int x = -3;

    if (x > 0) {
        printf("x is positive.\n");
    } else {
        printf("x is not positive.\n");
    }

    return 0;

}
```

在这个例子中，变量 x 的值为-3，因此 `if (x > 0)` 条件不成立，程序会执行 else 分支，输出 `x is not positive.`。

## `else if` 结构

else if 结构用于处理多个条件判断。它可以在一个 if-else 语句的基础上，增加多个条件判断，直到找到一个为真的条件。如果某个条件成立，则执行该条件对应的代码块，之后的分支将不会执行。

### 语法格式

```c
if (条件表达式 1) {
// 条件 1 为真时执行的代码块
} else if (条件表达式 2) {
// 条件 2 为真时执行的代码块
} else {
// 当所有条件都不成立时执行的代码块
}
```

### 还需要一个例子

```c
#include <stdio.h>

int main() {
    int x = 0;
    if (x > 0) {
        printf("x is positive.\n");
    } else if (x < 0) {
        printf("x is negative.\n");
    } else {
        printf("x is zero.\n");
    }
    return 0;
}
```

程序首先判断 x > 0 是否成立，如果成立则执行第一个分支。如果不成立，继续判断 x < 0，如果也不成立，则执行 else 分支。在这个例子中，x 的值为 0，因此会输出 `x is zero.`。

## `switch` 语句

switch 语句是一种多分支的选择结构，常用于处理离散值的判断。它通过匹配一个表达式的值，执行与该值对应的代码块。switch 语句常用于替代多个 if-else if 的条件判断，使得代码更简洁易读。

### 语法格式

```c
switch (表达式) {
case 常量 1:
// 当表达式的值等于常量 1 时执行
break;
case 常量 2:
// 当表达式的值等于常量 2 时执行
break;
// 可以有多个 case 分支
default:
// 当所有 case 都不匹配时执行
}
```

- 表达式：通常是一个整型表达式或字符表达式。
- case：用于匹配表达式的值。当表达式的值等于某个 case 分支的常量时，执行该分支的代码。
- break：用于跳出 switch 语句。如果不加 break，程序会继续执行下一个 case 的代码（即使条件不匹配），这称为"fall-through"现象。
- default：当所有 case 分支都不匹配时，执行 default 分支的代码。

### 又来一个例子

```c
#include <stdio.h>

int main() {
    int day = 3;
    switch (day) {
        case 1:
            printf("Monday\n");
            break;
        case 2:
            printf("Tuesday\n");
            break;
        case 3:
            printf("Wednesday\n");
            break;
        case 4:
            printf("Thursday\n");
            break;
        case 5:
            printf("Friday\n");
            break;
        default:
            printf("Invalid day\n");
    }
    return 0;
}
```

在这个例子中，day 的值为 3，因此程序会执行 case 3 的代码，输出“Wednesday”。如果没有 break 语句，程序会继续执行下一个 case 的代码，直到遇到 break 或结束 switch 语句。

## 嵌套的分支语句

分支语句可以互相嵌套，以处理更加复杂的条件判断。常见的嵌套方式是在 if 语句内部嵌套另一个 if 或 switch 语句。

### if里套if

```c
#include <stdio.h>

int main() {
    int x = 10;

    if (x > 0) {
        if (x % 2 == 0) {
            printf("x is positive and even.\n");
        } else {
            printf("x is positive and odd.\n");
        }
    } else {
        printf("x is not positive.\n");
    }

    return 0;

}
```

外层 if 语句判断 x 是否为正数，内层 if 语句进一步判断它的奇偶性。在这个例子中，x 的值为 10，因此会输出 `x is positive and even.`。

### switch里套switch

```c
#include <stdio.h>

int main() {
    int category = 1;
    int type = 2;

    switch (category) {
        case 1:
            switch (type) {
                case 1:
                    printf("Category 1, Type 1\n");
                    break;
                case 2:
                    printf("Category 1, Type 2\n");
                    break;
                default:
                    printf("Unknown type in category 1\n");
            }
            break;
        case 2:
            printf("Category 2\n");
            break;
        default:
            printf("Unknown category\n");
    }
    return 0;
}
```

这里我们嵌套了两个 switch 语句，分别处理类别和类别下的类型。category 为 1，type 为 2，因此会输出 `Category 1, Type 2`。

## 注意

### if 语句中的空语句

在使用 if 语句时，建议总是使用大括号{}包围代码块，即使代码块中只有一条语句。这可以防止某些情况下由于缩进或其他原因导致的逻辑错误。

典中典：未使用大括号的潜在问题

```c
#include <stdio.h>

int main() {
    int x = 5;

    if (x > 0)
        printf("x is positive.\n");
        printf("This is outside the if.\n");  // 实际上总是会执行

    return 0;

}
```

```txt
x is positive.
This is outside the if.
```

由于没有使用大括号，第二个 printf 语句并不属于 if 语句的条件判断部分。它将始终执行，可能导致不符合预期的程序行为。

改进示例

```c
#include <stdio.h>

int main() {
int x = 5;

    if (x > 0) {
        printf("x is positive.\n");
        printf("This is inside the if.\n");  // 只有条件成立时才执行
    }

    return 0;

}
```

x is positive.
This is inside the if.

### switch 语句中的“fall-through”现象

switch 语句中，如果没有 break 语句，会发生“fall-through”现象，导致程序继续执行下一个 case 的代码块，即使表达式不匹配下一个 case。

```c
#include <stdio.h>

int main() {
    int x = 2;

    switch (x) {
        case 1:
            printf("One\n");
        case 2:
            printf("Two\n");  // 执行此 case 后会继续执行 case 3 的代码
        case 3:
            printf("Three\n");
            break;
        default:
            printf("Unknown\n");
    }

    return 0;

}
```

```txt
Two
Three
```

改进示例

```c
#include <stdio.h>

int main() {
    int x = 2;

    switch (x) {
        case 1:
            printf("One\n");
            break;
        case 2:
            printf("Two\n");
            break;
        case 3:
            printf("Three\n");
            break;
        default:
            printf("Unknown\n");
    }

    return 0;

}
```

```txt
Two
```

## 来一点高级技巧

### 使用三元运算符简化条件判断

C 语言提供了三元运算符?:，用于简化简单的条件判断。三元运算符的语法如下：

```c
条件 ? 表达式 1 : 表达式 2;
```

当条件为真时，返回表达式 1 的值；否则返回表达式 2 的值。它通常用于替代简单的 if-else 结构。

```c
#include <stdio.h>

int main() {
    int x = 5;
    const char result = (x > 0) ? "positive" : "non-positive";

    printf("x is %s.\n", result);
    return 0;

}
```

### 枚举与 switch 语句的结合

当处理一组相关的常量时，可以使用 enum 枚举类型结合 switch 语句，这样代码更加清晰，并减少了使用魔术数字的风险。
示例：使用枚举和 switch

```c
#include <stdio.h>

enum Days { MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY };

int main() {
    enum Days today = WEDNESDAY;

    switch (today) {
        case MONDAY:
            printf("Today is Monday.\n");
            break;
        case TUESDAY:
            printf("Today is Tuesday.\n");
            break;
        case WEDNESDAY:
            printf("Today is Wednesday.\n");
            break;
        case THURSDAY:
            printf("Today is Thursday.\n");
            break;
        case FRIDAY:
            printf("Today is Friday.\n");
            break;
        default:
            printf("Unknown day.\n");
    }

    return 0;

}
```

### 总结

通过合理使用分支语句，程序可以根据输入和条件灵活调整执行路径，完成更为复杂的任务。在编写代码时，牢记代码的可读性和可维护性，尽量简化逻辑和控制流。

**生命中最重要的不是我们选择了什么，而是我们如何对待我们所选择的**

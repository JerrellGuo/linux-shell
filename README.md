# linux-shell
一个用c语言写的简单的linux shell模拟器

编译方式：linux或者windows cygwin下cd到代码目录，执行make
功能介绍：
1 支持ls、 cal、cd、time等系统命令
2 支持";" "&&" "||" 等序列化命令
3 支持backgr execution: “&”
4 支持管道命令： command1 | command2
5 支持subshell: "(commands)"
6 支持输入输出重定向：">","<",">>"
7 支持shell scripts
8 支持HOME，PATH,CDPATH内部变量设置

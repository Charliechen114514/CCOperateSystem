#!/usr/bin/perl
use strict;
use warnings;

# 检查是否提供了文件名参数
if (@ARGV != 1) {
    die "Usage: perl modify_list.pl <filename>\n";
}

# 获取文件名参数
my $file = $ARGV[0];

# 打开文件进行读取
open my $fh, '+<', $file or die "Cannot open file $file: $!\n";

# 读取文件内容
my @lines = <$fh>;

# 关闭文件
close $fh;

# 插入#define set_intr_status() (void) 到文件开头
unshift @lines, "#define set_intr_status(param) 0;(void)param\n";

# 替换操作
foreach my $line (@lines) {
    # 删除#include "include/kernel/interrupt.h"
    $line =~ s/#include "include\/kernel\/interrupt.h"\s*\n//g;

    # 替换(INTR_OFF)为(0)
    $line =~ s/\(INTR_OFF\)/\(0\)/g;

    # 替换Interrupt_Status为int
    $line =~ s/Interrupt_Status/int/g;
}

# 重新写入文件
open my $out_fh, '>', $file or die "Cannot open file $file for writing: $!\n";
print $out_fh @lines;
close $out_fh;

print "File $file has been modified successfully.\n";

if [[ ! -d "../lib" || ! -d "../middlewares" ]]; then 
    echo "dependent dir don\`t exist!" 
    cwd=$(pwd) 
    cwd=${cwd##*/} 
    cwd=${cwd%/} 
    if [[ $cwd != "command" ]]; then 
        echo -e "you\`d better in command dir\n" 
    fi 
    exit 
fi 

BIN="prog_no_arg" 
CFLAGS="-Wall -c -fno-builtin -W -Wstrict-prototypes \
       -Wmissing-prototypes -Wsystem-headers" 

OBJS="../middlewares/string.o ../middlewares/syscall.o \
       ../middlewares/stdio.o ../middlewares/assert.o" 
DD_IN=$BIN 
DD_OUT="/home/work/my_workspace/bochs/hd60M.img" 

gcc $CFLAGS -I $LIB -o $BIN".o" $BIN".c" 
ld -e main $BIN".o" $OBJS -o $BIN 
SEC_CNT=$(ls -l $BIN | awk '{printf("%d", ($5+511)/512)}') 

if [[ -f $BIN ]]; then 
    dd if=./$DD_IN of=$DD_OUT bs=512 \
       count=$SEC_CNT seek=300 conv=notrunc 
fi 

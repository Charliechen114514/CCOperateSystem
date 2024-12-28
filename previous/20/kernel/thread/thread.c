#include "thread.h"   //函数声明 各种结构体
#include "stdint.h"   //前缀
#include "string.h"   //memset
#include "global.h"   //不清楚
#include "memory.h"   //分配页需要
#include "debug.h"
#include "interrupt.h"
#include "print.h"
#include "../userprog/process.h"

struct task_struct* main_thread;                        //主线程main_thread的pcb
struct list thread_ready_list;			  //就绪队列
struct list thread_all_list;				  //总线程队列

extern void switch_to(struct task_struct* cur,struct task_struct* next);

// 获取 pcb 指针
// 这部分我可以来稍微解释一下
// 我们线程所在的esp 肯定是在 我们get得到的那一页内存 pcb页上下浮动 但是我们的pcb的最起始位置是整数的 除去后面的12位
// 那么我们对前面的取 & 则可以得到 我们的地址所在地
struct task_struct* running_thread(void)
{
    uint32_t esp;
    asm ("mov %%esp,%0" : "=g"(esp));
    return (struct task_struct*)(esp & 0xfffff000);
}


void kernel_thread(thread_func* function,void* func_arg)
{
    intr_enable();					    //开中断 防止后面的时间中断被屏蔽无法切换线程
    function(func_arg);
}

void thread_create(struct task_struct* pthread,thread_func function,void* func_arg)
{
    pthread->self_kstack -= sizeof(struct intr_stack);  //减去中断栈的空间
    pthread->self_kstack -= sizeof(struct thread_stack);
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack; 
    kthread_stack->eip = kernel_thread;                 //地址为kernel_thread 由kernel_thread 执行function
    kthread_stack->function = function;
    kthread_stack->func_arg = func_arg;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->ebx = kthread_stack->esi = 0; //初始化一下
    return;
}

void init_thread(struct task_struct* pthread,char* name,int prio)
{
    memset(pthread,0,sizeof(*pthread)); //pcb位置清0
    strcpy(pthread->name,name);
    
    if(pthread == main_thread)
    	pthread->status = TASK_RUNNING;                              //我们的主线程肯定是在运行的
    else
    	pthread->status = TASK_READY;					//放到就绪队列里面
    	
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE); //刚开始的位置是最低位置 栈顶位置+一页得最顶部
                                                                     //后面还要对这个值进行修改
                                                                 
    pthread->priority = prio;                                        
    pthread->ticks = prio;                                           //和特权级 相同的时间片
    pthread->elapsed_ticks = 0;
    pthread->pgdir = NULL;                                           //线程没有单独的地址
    pthread->stack_magic = KERNEL_MAGIC;                               //设置的魔数 检测是否越界限
}

struct task_struct* thread_start(char* name,int prio,thread_func function,void* func_arg)
{
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread,name,prio);
    thread_create(thread,function,func_arg);
    ASSERT(!elem_find(&thread_ready_list,&thread->general_tag));     //之前不应该在就绪队列里面
    list_append(&thread_ready_list,&thread->general_tag);
    
    ASSERT(!elem_find(&thread_all_list,&thread->all_list_tag));
    list_append(&thread_all_list,&thread->all_list_tag);
     
    return thread;
}


//之前在loader.S的时候已经 mov esp,0xc0009f00
//现在的esp已经就在预留的pcb位置上了
void make_main_thread(void)
{		
    main_thread = running_thread();					//得到main_thread 的pcb指针
    init_thread(main_thread,"main",31);				
    
    ASSERT(!elem_find(&thread_all_list,&main_thread->all_list_tag));
    list_append(&thread_all_list,&main_thread->all_list_tag);
    
}

void schedule(void)
{
    ASSERT(intr_get_status() == INTR_OFF);
    
    struct task_struct* cur = running_thread();			//得到当前pcb的地址
    if(cur->status == TASK_RUNNING)
    {
    	ASSERT(!elem_find(&thread_ready_list,&cur->general_tag));     //目前在运行的肯定ready_list是不在的
    	list_append(&thread_ready_list,&cur->general_tag);           //加入尾部
    	
    	cur->status = TASK_READY;
    	cur->ticks = cur->priority;
    }
    else
    {}
    
    ASSERT(!list_empty(&thread_ready_list));
    struct task_struct* thread_tag = list_pop(&thread_ready_list);
    //书上面的有点难理解 代码我写了一个我能理解的
    struct task_struct* next = (struct task_struct*)((uint32_t)thread_tag & 0xfffff000);
    next->status = TASK_RUNNING;
    process_activate(next);
    switch_to(cur,next);                                              //esp头顶的是 返回地址 +12是next +8是cur
}

void thread_init(void)
{
    put_str("thread_init start!\n");
    list_init(&thread_ready_list);
    list_init(&thread_all_list);
    make_main_thread();
    put_str("thread_init done!\n");
}

void thread_block(enum task_status stat)
{
    //设置block状态的参数必须是下面三个以下的
    ASSERT(((stat == TASK_BLOCKED) || (stat == TASK_WAITING) || stat == TASK_HANGING));
    
    enum intr_status old_status = intr_disable();			 //关中断
    struct task_struct* cur_thread = running_thread();		 
    cur_thread->status = stat;					 //把状态重新设置
    
    //调度器切换其他进程了 而且由于status不是running 不会再被放到就绪队列中
    schedule();	
    				
    //被切换回来之后再进行的指令了
    intr_set_status(old_status);
}

//由锁拥有者来执行的 善良者把原来自我阻塞的线程重新放到队列中
void thread_unblock(struct task_struct* pthread)
{
    enum intr_status old_status = intr_disable();
    ASSERT(((pthread->status == TASK_BLOCKED) || (pthread->status == TASK_WAITING) || (pthread->status == TASK_HANGING)));
    if(pthread->status != TASK_READY)
    {
    	//被阻塞线程 不应该存在于就绪队列中）
    	ASSERT(!elem_find(&thread_ready_list,&pthread->general_tag));
    	if(elem_find(&thread_ready_list,&pthread->general_tag))
    	    PANIC("thread_unblock: blocked thread in ready_list\n"); //debug.h中定义过
    	
    	//让阻塞了很久的任务放在就绪队列最前面
    	list_push(&thread_ready_list,&pthread->general_tag);
    	
    	//状态改为就绪态
    	pthread->status = TASK_READY;
    }
    intr_set_status(old_status);
}


